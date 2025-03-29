/*
 * lmac.h
 *
 *  Created on: 27 nov. 2024
 *      Author: Ludo
 */

#ifndef __LMAC_H__
#define __LMAC_H__

#ifndef LMAC_DRIVER_DISABLE_FLAGS_FILE
#include "lmac_driver_flags.h"
#endif
#include "error.h"
#include "types.h"

/*** LMAC macros ***/

#define LMAC_ADDRESS_LAST           0x7F
#define LMAC_ADDRESS_MASK           0x7F
#define LMAC_ADDRESS_SIZE_BYTES     1

/*** LMAC structures ***/

/*!******************************************************************
 * \enum LMAC_status_t
 * \brief LMAC driver error codes.
 *******************************************************************/
typedef enum {
    // Driver errors.
    LMAC_SUCCESS = 0,
    LMAC_ERROR_NULL_PARAMETER,
    LMAC_ERROR_SELF_ADDRESS,
    LMAC_ERROR_DESTINATION_ADDRESS,
    LMAC_ERROR_TX_DATA_SIZE,
    LMAC_ERROR_RX_DESTINATION_ADDRESS,
    LMAC_ERROR_RX_SOURCE_ADDRESS,
    LMAC_ERROR_RX_DATA_SIZE,
    // Low level drivers errors.
    LMAC_ERROR_BASE_HW_INTERFACE = ERROR_BASE_STEP,
    LMAC_ERROR_BASE_NVM = (LMAC_ERROR_BASE_HW_INTERFACE + LMAC_DRIVER_HW_INTERFACE_ERROR_BASE_LAST),
    // Last base value.
    LMAC_ERROR_BASE_LAST = (LMAC_ERROR_BASE_NVM + LMAC_DRIVER_NVM_ERROR_BASE_LAST),
} LMAC_status_t;

#ifndef LMAC_DRIVER_DISABLE

/*!******************************************************************
 * \fn LMAC_rx_irq_cb_t
 * \brief Byte reception interrupt callback.
 *******************************************************************/
typedef void (*LMAC_rx_irq_cb_t)(uint8_t data);

/*!******************************************************************
 * \fn LMAC_error_cb_t
 * \brief Transfer error callback.
 *******************************************************************/
typedef void (*LMAC_error_cb_t)(LMAC_status_t status);

/*** LMAC functions ***/

/*!******************************************************************
 * \fn LMAC_status_t LMAC_init(uint32_t baud_rate, LMAC_rx_irq_cb_t rx_irq_callback)
 * \brief Init LMAC interface.
 * \param[in]   baud_rate: Bus baud rate.
 * \param[in]   rx_irq_callback: Function to call on frame reception interrupt.
 * \param[in]   error_callback: Function to call on error event.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
LMAC_status_t LMAC_init(uint32_t baud_rate, LMAC_rx_irq_cb_t rx_irq_callback, LMAC_error_cb_t error_callback);

/*!******************************************************************
 * \fn LMAC_status_t LMAC_de_init(void)
 * \brief Release LMAC interface.
 * \param[in]   none
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
LMAC_status_t LMAC_de_init(void);

/*!******************************************************************
 * \fn LMAC_status_t LMAC_enable_rx(void)
 * \brief Enable bus reception.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
LMAC_status_t LMAC_enable_rx(void);

/*!******************************************************************
 * \fn LMAC_status_t LMAC_disable_rx(void)
 * \brief Disable bus reception.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
LMAC_status_t LMAC_disable_rx(void);

/*!******************************************************************
 * \fn LBUS_status_t LMAC_write(uint8_t* data, uint32_t data_size_bytes)
 * \brief Send data over LMAC bus.
 * \param[in]   data: Byte array to send.
 * \param[in]   data_size_bytes: Number of bytes to send.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
LMAC_status_t LMAC_write(uint8_t* data, uint32_t data_size_bytes);

#ifdef LMAC_DRIVER_MODE_MASTER
/*!******************************************************************
 * \fn LMAC_status_t LMAC_set_destination_address(uint8_t destination_address)
 * \brief Set destination address.
 * \param[in]   destination_address: Address to use when sending a packet.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
LMAC_status_t LMAC_set_destination_address(uint8_t destination_address);
#endif

/*******************************************************************/
#define LMAC_exit_error(base) { ERROR_check_exit(lmac_status, LMAC_SUCCESS, base) }

/*******************************************************************/
#define LMAC_stack_error(base) { ERROR_check_stack(lmac_status, LMAC_SUCCESS, base) }

/*******************************************************************/
#define LMAC_stack_exit_error(base, code) { ERROR_check_stack_exit(lmac_status, LMAC_SUCCESS, base, code) }

#endif /* LMAC_DRIVER_DISABLE */

#endif /* __LMAC_H__ */
