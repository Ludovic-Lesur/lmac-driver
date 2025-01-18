/*
 * lmac_hw.c
 *
 *  Created on: 27 nov. 2024
 *      Author: Ludo
 */

#include "lmac_hw.h"

#ifndef LMAC_DRIVER_DISABLE_FLAGS_FILE
#include "lmac_driver_flags.h"
#endif
#include "lmac.h"
#include "types.h"

#ifndef LMAC_DRIVER_DISABLE

/*** LMAC HW functions ***/

/*******************************************************************/
LMAC_status_t __attribute__((weak)) LMAC_HW_init(uint32_t baud_rate, LMAC_rx_irq_cb_t rx_irq_callback, uint8_t* self_address) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    /* To be implemented */
    UNUSED(baud_rate);
    UNUSED(rx_irq_callback);
    UNUSED(self_address);
    return status;
}

/*******************************************************************/
LMAC_status_t __attribute__((weak)) LMAC_HW_de_init(void) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    /* To be implemented */
    return status;
}

/*******************************************************************/
LMAC_status_t __attribute__((weak)) LMAC_HW_enable_rx(void) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    /* To be implemented */
    return status;
}

/*******************************************************************/
LMAC_status_t __attribute__((weak)) LMAC_HW_disable_rx(void) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    /* To be implemented */
    return status;
}

/*******************************************************************/
LMAC_status_t __attribute__((weak)) LMAC_HW_write(uint8_t* data, uint32_t data_size_bytes) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    /* To be implemented */
    UNUSED(data);
    UNUSED(data_size_bytes);
    return status;
}

#endif /* LMAC_DRIVER_DISABLE */
