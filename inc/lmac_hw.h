/*
 * lmac_hw.h
 *
 *  Created on: 27 nov. 2024
 *      Author: Ludo
 */

#ifndef __LMAC_HW_H__
#define __LMAC_HW_H__

#ifndef LMAC_DRIVER_DISABLE_FLAGS_FILE
#include "lmac_driver_flags.h"
#endif
#include "lmac.h"
#include "types.h"

#ifndef LMAC_DRIVER_DISABLE

/*** LMAC HW functions ***/

/*!******************************************************************
 * \fn LMAC_status_t LMAC_HW_init(LMAC_rx_irq_cb_t rx_irq_callback, uint8_t* self_address
 * \brief Init LMAC hardware interface.
 * \param[in]   baud_rate: Bus baud rate.
 * \param[in]   rx_irq_callback: Function to call on byte reception interrupt.
 * \param[out]  self_address: Node address must be returned in this pointer.
 * \retval      Function execution status.
 *******************************************************************/
LMAC_status_t LMAC_HW_init(uint32_t baud_rate, LMAC_rx_irq_cb_t rx_irq_callback, uint8_t* self_address);

/*!******************************************************************
 * \fn LMAC_status_t LMAC_HW_de_init(void)
 * \brief Release LMAC hardware interface.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
LMAC_status_t LMAC_HW_de_init(void);

/*!******************************************************************
 * \fn LMAC_status_t LMAC_HW_enable_rx(void)
 * \brief Enable bus reception.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
LMAC_status_t LMAC_HW_enable_rx(void);

/*!******************************************************************
 * \fn LMAC_status_t LMAC_HW_disable_rx(void)
 * \brief Disable bus reception.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
LMAC_status_t LMAC_HW_disable_rx(void);

/*!******************************************************************
 * \fn LMAC_status_t LMAC_HW_write(uint8_t* data, uint32_t data_size_bytes)
 * \brief Send data over LMAC physical interface.
 * \param[in]   data: Byte array to send.
 * \param[in]   data_size_bytes: Number of bytes to send.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
LMAC_status_t LMAC_HW_write(uint8_t* data, uint32_t data_size_bytes);

#ifdef LMAC_DRIVER_MODE_SLAVE
/*!******************************************************************
 * \fn uint32_t LMAC_HW_get_uptime_seconds(void)
 * \brief Get system uptime.
 * \param[in]   none
 * \param[out]  none
 * \retval      System uptime in seconds.
 *******************************************************************/
uint32_t LMAC_HW_get_uptime_seconds(void);
#endif

#endif /* LMAC_DRIVER_DISABLE */

#endif /* __LMAC_HW_H__ */
