/**
 * @file ramviewer.c
 * @brief RAMViewer MCU Library Implementation
 *
 * Communication protocol implementation for RAM read/write operations.
 * Uses state machine for receiving frames and interrupt-driven transmission.
 */

#include "ramviewer.h"
#include <string.h>

/*===========================================================================*/
/* Version                                                                   */
/*===========================================================================*/

#define RV_VERSION_STRING   "1.0.0"

/*===========================================================================*/
/* Private Types                                                             */
/*===========================================================================*/

/** @brief Receiver state machine states */
typedef enum {
    RX_STATE_IDLE,          /**< Waiting for SOF */
    RX_STATE_LEN_L,         /**< Waiting for length low byte */
    RX_STATE_LEN_H,         /**< Waiting for length high byte */
    RX_STATE_CMD,           /**< Waiting for command */
    RX_STATE_SEQ,           /**< Waiting for sequence number */
    RX_STATE_PAYLOAD,       /**< Receiving payload */
    RX_STATE_CRC_L,         /**< Waiting for CRC low byte */
    RX_STATE_CRC_H          /**< Waiting for CRC high byte */
} rx_state_t;

/** @brief Transmitter state */
typedef enum {
    TX_STATE_IDLE,          /**< Idle, no transmission */
    TX_STATE_SENDING        /**< Transmission in progress */
} tx_state_t;

/*===========================================================================*/
/* Private Variables                                                         */
/*===========================================================================*/

/** @brief Receive buffer */
static uint8_t rv_rx_buffer[RV_FRAME_OVERHEAD + RV_MAX_PAYLOAD_SIZE];

/** @brief Transmit buffer */
static uint8_t rv_tx_buffer[RV_FRAME_OVERHEAD + RV_MAX_PAYLOAD_SIZE];

/** @brief Receiver state */
static volatile rx_state_t rv_rx_state = RX_STATE_IDLE;

/** @brief Transmitter state */
static volatile tx_state_t rv_tx_state = TX_STATE_IDLE;

/** @brief Expected payload length */
static uint16_t rv_rx_payload_len;

/** @brief Current payload index */
static uint16_t rv_rx_payload_idx;

/** @brief Total TX frame length */
static uint16_t rv_tx_len;

/** @brief Current TX byte index */
static volatile uint16_t rv_tx_idx;

/** @brief Send byte callback */
static rv_send_byte_fn rv_send_byte = NULL;

/*===========================================================================*/
/* Private Function Prototypes                                               */
/*===========================================================================*/

static uint16_t rv_crc16_ccitt(const uint8_t *data, uint16_t len);
static void rv_process_frame(void);
static void rv_handle_read(const uint8_t *payload, uint16_t len, uint8_t seq);
static void rv_handle_write(const uint8_t *payload, uint16_t len, uint8_t seq);
static void rv_send_response(uint8_t cmd, uint8_t seq, uint16_t payload_len);
static void rv_send_error(uint8_t error_code, uint8_t seq);
static void rv_send_pong(uint8_t seq);

/*===========================================================================*/
/* CRC16 Implementation                                                      */
/*===========================================================================*/

/**
 * @brief Calculate CRC16-CCITT
 * @param data Input data
 * @param len Data length
 * @return CRC16 value
 *
 * Polynomial: 0x1021, Initial value: 0xFFFF
 */
static uint16_t rv_crc16_ccitt(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    uint8_t i;

    while (len--) {
        crc ^= (uint16_t)(*data++) << 8;
        for (i = 0; i < 8; i++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/*===========================================================================*/
/* Public Functions                                                          */
/*===========================================================================*/

void rv_init(rv_send_byte_fn send_fn)
{
    rv_send_byte = send_fn;
    rv_rx_state = RX_STATE_IDLE;
    rv_tx_state = TX_STATE_IDLE;
    rv_tx_idx = 0;
    rv_tx_len = 0;
}

void rv_rx_byte(uint8_t byte)
{
    switch (rv_rx_state) {
        case RX_STATE_IDLE:
            if (byte == RV_SOF) {
                rv_rx_buffer[0] = byte;
                rv_rx_state = RX_STATE_LEN_L;
            }
            break;

        case RX_STATE_LEN_L:
            rv_rx_buffer[1] = byte;
            rv_rx_payload_len = byte;
            rv_rx_state = RX_STATE_LEN_H;
            break;

        case RX_STATE_LEN_H:
            rv_rx_buffer[2] = byte;
            rv_rx_payload_len |= ((uint16_t)byte << 8);
            if (rv_rx_payload_len > RV_MAX_PAYLOAD_SIZE) {
                /* Frame too large, reset */
                rv_rx_state = RX_STATE_IDLE;
            } else {
                rv_rx_state = RX_STATE_CMD;
            }
            break;

        case RX_STATE_CMD:
            rv_rx_buffer[3] = byte;
            rv_rx_state = RX_STATE_SEQ;
            break;

        case RX_STATE_SEQ:
            rv_rx_buffer[4] = byte;
            rv_rx_payload_idx = 0;
            if (rv_rx_payload_len > 0) {
                rv_rx_state = RX_STATE_PAYLOAD;
            } else {
                rv_rx_state = RX_STATE_CRC_L;
            }
            break;

        case RX_STATE_PAYLOAD:
            rv_rx_buffer[5 + rv_rx_payload_idx] = byte;
            rv_rx_payload_idx++;
            if (rv_rx_payload_idx >= rv_rx_payload_len) {
                rv_rx_state = RX_STATE_CRC_L;
            }
            break;

        case RX_STATE_CRC_L:
            rv_rx_buffer[5 + rv_rx_payload_len] = byte;
            rv_rx_state = RX_STATE_CRC_H;
            break;

        case RX_STATE_CRC_H:
            rv_rx_buffer[6 + rv_rx_payload_len] = byte;
            /* Frame complete, process it */
            rv_process_frame();
            rv_rx_state = RX_STATE_IDLE;
            break;

        default:
            rv_rx_state = RX_STATE_IDLE;
            break;
    }
}

void rv_tx_complete(void)
{
    if (rv_tx_state == TX_STATE_SENDING && rv_tx_idx < rv_tx_len) {
        if (rv_send_byte != NULL) {
            rv_send_byte(rv_tx_buffer[rv_tx_idx]);
            rv_tx_idx++;
        }
    } else {
        rv_tx_state = TX_STATE_IDLE;
    }
}

bool rv_is_tx_busy(void)
{
    return (rv_tx_state == TX_STATE_SENDING);
}

const char* rv_get_version(void)
{
    return RV_VERSION_STRING;
}

/*===========================================================================*/
/* Private Functions                                                         */
/*===========================================================================*/

/**
 * @brief Process a complete received frame
 */
static void rv_process_frame(void)
{
    uint16_t frame_len;
    uint16_t recv_crc;
    uint16_t calc_crc;
    uint8_t cmd;
    uint8_t seq;
    uint8_t *payload;

    frame_len = 5 + rv_rx_payload_len;

    /* Extract and verify CRC */
    recv_crc = rv_rx_buffer[frame_len] |
               ((uint16_t)rv_rx_buffer[frame_len + 1] << 8);
    calc_crc = rv_crc16_ccitt(rv_rx_buffer, frame_len);

    if (recv_crc != calc_crc) {
        rv_send_error(RV_ERR_CRC, rv_rx_buffer[4]);
        return;
    }

    cmd = rv_rx_buffer[3];
    seq = rv_rx_buffer[4];
    payload = &rv_rx_buffer[5];

    switch (cmd) {
        case RV_CMD_READ_VAR:
            rv_handle_read(payload, rv_rx_payload_len, seq);
            break;

        case RV_CMD_WRITE_VAR:
            rv_handle_write(payload, rv_rx_payload_len, seq);
            break;

        case RV_CMD_PING:
            rv_send_pong(seq);
            break;

        default:
            rv_send_error(RV_ERR_CMD, seq);
            break;
    }
}

/**
 * @brief Handle read variable command
 * @param payload Command payload
 * @param len Payload length
 * @param seq Sequence number
 */
static void rv_handle_read(const uint8_t *payload, uint16_t len, uint8_t seq)
{
    uint8_t count;
    uint8_t i;
    uint16_t resp_len;
    uint8_t *resp;
    const uint8_t *var_entry;
    uint32_t addr;
    uint16_t size;
    uint8_t bit_off;
    uint8_t bit_size;
    uint8_t *mem_ptr;
    uint16_t j;

    if (len < 1) {
        rv_send_error(RV_ERR_SIZE, seq);
        return;
    }

    count = payload[0];
    if (count == 0 || count > RV_MAX_VARIABLES) {
        rv_send_error(RV_ERR_SIZE, seq);
        return;
    }

    /* Verify payload length: 1 + count * 8 */
    if (len < (uint16_t)(1 + count * 8)) {
        rv_send_error(RV_ERR_SIZE, seq);
        return;
    }

    /* Build response in TX buffer */
    resp = &rv_tx_buffer[5];
    resp_len = 1;  /* count byte */
    resp[0] = count;

    for (i = 0; i < count; i++) {
        var_entry = &payload[1 + i * 8];

        /* Parse variable info (little-endian) */
        addr = var_entry[0] |
               ((uint32_t)var_entry[1] << 8) |
               ((uint32_t)var_entry[2] << 16) |
               ((uint32_t)var_entry[3] << 24);
        size = var_entry[4] | ((uint16_t)var_entry[5] << 8);
        bit_off = var_entry[6];
        bit_size = var_entry[7];

        /* Get memory pointer */
        mem_ptr = (uint8_t *)addr;

        if (bit_off != RV_NO_BITFIELD) {
            /* Bitfield read: extract bits from byte */
            uint8_t byte_val = *mem_ptr;
            uint8_t mask = (uint8_t)((1U << bit_size) - 1U);
            uint8_t value = (byte_val >> bit_off) & mask;
            resp[resp_len] = value;
            resp_len++;
        } else {
            /* Normal read: copy bytes */
            for (j = 0; j < size; j++) {
                if (resp_len >= RV_MAX_PAYLOAD_SIZE) {
                    rv_send_error(RV_ERR_SIZE, seq);
                    return;
                }
                resp[resp_len] = mem_ptr[j];
                resp_len++;
            }
        }
    }

    rv_send_response(RV_CMD_READ_RESP, seq, resp_len);
}

/**
 * @brief Handle write variable command
 * @param payload Command payload
 * @param len Payload length
 * @param seq Sequence number
 */
static void rv_handle_write(const uint8_t *payload, uint16_t len, uint8_t seq)
{
    uint8_t count;
    uint8_t i;
    uint16_t offset;
    uint32_t addr;
    uint16_t size;
    uint8_t bit_off;
    uint8_t bit_size;
    uint8_t *mem_ptr;
    uint16_t j;

    if (len < 1) {
        rv_send_error(RV_ERR_SIZE, seq);
        return;
    }

    count = payload[0];
    if (count == 0 || count > RV_MAX_VARIABLES) {
        rv_send_error(RV_ERR_SIZE, seq);
        return;
    }

    offset = 1;

    for (i = 0; i < count; i++) {
        /* Check remaining length */
        if (offset + 8 > len) {
            rv_send_error(RV_ERR_SIZE, seq);
            return;
        }

        /* Parse variable info (little-endian) */
        addr = payload[offset] |
               ((uint32_t)payload[offset + 1] << 8) |
               ((uint32_t)payload[offset + 2] << 16) |
               ((uint32_t)payload[offset + 3] << 24);
        size = payload[offset + 4] | ((uint16_t)payload[offset + 5] << 8);
        bit_off = payload[offset + 6];
        bit_size = payload[offset + 7];
        offset += 8;

        /* Get memory pointer */
        mem_ptr = (uint8_t *)addr;

        if (bit_off != RV_NO_BITFIELD) {
            /* Bitfield write: read-modify-write */
            if (offset >= len) {
                rv_send_error(RV_ERR_SIZE, seq);
                return;
            }
            uint8_t byte_val = *mem_ptr;
            uint8_t mask = (uint8_t)(((1U << bit_size) - 1U) << bit_off);
            uint8_t new_val = payload[offset];
            *mem_ptr = (byte_val & ~mask) | ((new_val << bit_off) & mask);
            offset++;
        } else {
            /* Normal write: copy bytes */
            if (offset + size > len) {
                rv_send_error(RV_ERR_SIZE, seq);
                return;
            }
            for (j = 0; j < size; j++) {
                mem_ptr[j] = payload[offset];
                offset++;
            }
        }
    }

    /* Send success response */
    rv_tx_buffer[5] = RV_ERR_OK;
    rv_send_response(RV_CMD_WRITE_RESP, seq, 1);
}

/**
 * @brief Send response frame
 * @param cmd Response command
 * @param seq Sequence number
 * @param payload_len Payload length (payload already in rv_tx_buffer[5..])
 */
static void rv_send_response(uint8_t cmd, uint8_t seq, uint16_t payload_len)
{
    uint16_t crc;

    /* Build frame header */
    rv_tx_buffer[0] = RV_SOF;
    rv_tx_buffer[1] = (uint8_t)(payload_len & 0xFF);
    rv_tx_buffer[2] = (uint8_t)((payload_len >> 8) & 0xFF);
    rv_tx_buffer[3] = cmd;
    rv_tx_buffer[4] = seq;
    /* Payload already at rv_tx_buffer[5..] */

    /* Calculate and append CRC */
    crc = rv_crc16_ccitt(rv_tx_buffer, 5 + payload_len);
    rv_tx_buffer[5 + payload_len] = (uint8_t)(crc & 0xFF);
    rv_tx_buffer[6 + payload_len] = (uint8_t)((crc >> 8) & 0xFF);

    /* Start transmission */
    rv_tx_len = 7 + payload_len;
    rv_tx_idx = 1;  /* First byte sent below */
    rv_tx_state = TX_STATE_SENDING;

    /* Send first byte */
    if (rv_send_byte != NULL) {
        rv_send_byte(rv_tx_buffer[0]);
    }
}

/**
 * @brief Send error response
 * @param error_code Error code
 * @param seq Sequence number
 */
static void rv_send_error(uint8_t error_code, uint8_t seq)
{
    rv_tx_buffer[5] = error_code;
    rv_send_response(RV_CMD_ERROR, seq, 1);
}

/**
 * @brief Send pong response
 * @param seq Sequence number
 */
static void rv_send_pong(uint8_t seq)
{
    rv_send_response(RV_CMD_PONG, seq, 0);
}
