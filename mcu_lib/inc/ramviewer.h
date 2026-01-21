/**
 * @file ramviewer.h
 * @brief RAMViewer MCU Library - Serial communication library for RAM read/write
 *
 * This library provides MCU-side support for the RAMViewer tool.
 * It only depends on UART RX/TX interrupt functions.
 *
 * Usage:
 * 1. Call rv_init() with your UART send function
 * 2. Call rv_rx_byte() from UART RX interrupt
 * 3. Call rv_tx_complete() from UART TX complete interrupt
 *
 * Example:
 * @code
 * void uart_send_byte(uint8_t byte) {
 *     UART_DATA = byte;
 * }
 *
 * void UART_RX_IRQHandler(void) {
 *     uint8_t c = UART_DATA;
 *     rv_rx_byte(c);
 * }
 *
 * void UART_TX_IRQHandler(void) {
 *     rv_tx_complete();
 * }
 *
 * int main(void) {
 *     rv_init(uart_send_byte);
 *     // Enable UART interrupts
 *     while (1) { }
 * }
 * @endcode
 */

#ifndef RAMVIEWER_H
#define RAMVIEWER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/* Configuration                                                             */
/*===========================================================================*/

/**
 * @brief Maximum payload size in bytes
 * Adjust based on available RAM. Default: 256 bytes
 * Minimum recommended: 64 bytes
 */
#ifndef RV_MAX_PAYLOAD_SIZE
#define RV_MAX_PAYLOAD_SIZE     256
#endif

/**
 * @brief Maximum number of variables per command
 * Default: 30 variables
 */
#ifndef RV_MAX_VARIABLES
#define RV_MAX_VARIABLES        30
#endif

/*===========================================================================*/
/* Protocol Constants                                                        */
/*===========================================================================*/

/** @brief Start of frame marker */
#define RV_SOF                  0xAA

/** @brief Frame overhead: SOF(1) + LEN(2) + CMD(1) + SEQ(1) + CRC(2) = 7 */
#define RV_FRAME_OVERHEAD       7

/*---------------------------------------------------------------------------*/
/* Command Types                                                             */
/*---------------------------------------------------------------------------*/

#define RV_CMD_READ_VAR         0x01    /**< Read variable(s) request */
#define RV_CMD_WRITE_VAR        0x02    /**< Write variable(s) request */
#define RV_CMD_READ_RESP        0x81    /**< Read response */
#define RV_CMD_WRITE_RESP       0x82    /**< Write response */
#define RV_CMD_ERROR            0xFF    /**< Error response */
#define RV_CMD_PING             0x10    /**< Heartbeat ping */
#define RV_CMD_PONG             0x90    /**< Heartbeat pong */

/*---------------------------------------------------------------------------*/
/* Error Codes                                                               */
/*---------------------------------------------------------------------------*/

#define RV_ERR_OK               0x00    /**< Success */
#define RV_ERR_CRC              0x01    /**< CRC mismatch */
#define RV_ERR_ADDR             0x02    /**< Invalid address */
#define RV_ERR_SIZE             0x03    /**< Invalid size */
#define RV_ERR_CMD              0x04    /**< Unknown command */

/*---------------------------------------------------------------------------*/
/* Bitfield Markers                                                          */
/*---------------------------------------------------------------------------*/

/** @brief Marker indicating non-bitfield variable */
#define RV_NO_BITFIELD          0xFF

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/

/**
 * @brief Callback function type for sending a single byte
 * @param byte The byte to send via UART
 */
typedef void (*rv_send_byte_fn)(uint8_t byte);

/*===========================================================================*/
/* Public API                                                                */
/*===========================================================================*/

/**
 * @brief Initialize RAMViewer library
 * @param send_fn Callback function for sending a byte via UART
 *
 * Call this function once during system initialization.
 */
void rv_init(rv_send_byte_fn send_fn);

/**
 * @brief Process received byte from UART
 * @param byte The received byte
 *
 * Call this function from UART RX interrupt handler.
 * This function is interrupt-safe and executes quickly.
 */
void rv_rx_byte(uint8_t byte);

/**
 * @brief Handle UART TX complete event
 *
 * Call this function from UART TX complete interrupt handler.
 * This triggers sending the next byte if available.
 */
void rv_tx_complete(void);

/**
 * @brief Check if transmitter is busy
 * @return true if transmission is in progress
 */
bool rv_is_tx_busy(void);

/**
 * @brief Get library version
 * @return Version string
 */
const char* rv_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* RAMVIEWER_H */
