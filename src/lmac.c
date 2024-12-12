/*
 * lmac.c
 *
 *  Created on: 27 nov. 2024
 *      Author: Ludo
 */

#include "lmac.h"

#ifndef LMAC_DRIVER_DISABLE_FLAGS_FILE
#include "lmac_driver_flags.h"
#endif
#include "lmac_hw.h"
#include "types.h"

#ifndef LMAC_DRIVER_DISABLE

/*** LMAC local macros ***/

#define LMAC_ADDRESS_MARKER     0x80

/*** LMAC local structures ***/

/*******************************************************************/
typedef enum {
    LMAC_FRAME_FIELD_INDEX_DESTINATION_ADDRESS = 0,
    LMAC_FRAME_FIELD_INDEX_SOURCE_ADDRESS = (LMAC_FRAME_FIELD_INDEX_DESTINATION_ADDRESS + LMAC_ADDRESS_SIZE_BYTES),
    LMAC_FRAME_FIELD_INDEX_DATA = (LMAC_FRAME_FIELD_INDEX_SOURCE_ADDRESS + LMAC_ADDRESS_SIZE_BYTES)
} LMAC_frame_field_index_t;

/*******************************************************************/
typedef struct {
    LMAC_rx_irq_cb_t rx_irq_callback;
    uint8_t self_address;
    uint8_t destination_address;
    uint8_t rx_byte_count;
#ifdef LMAC_DRIVER_MODE_SLAVE
    uint32_t first_rx_byte_timestamp_seconds;
#endif
} LMAC_context_t;

/*** LMAC local global variables ***/

static LMAC_context_t lmac_ctx;

/*** LMAC local functions ***/

/*******************************************************************/
#define _LMAC_check_address(address, error_code) { \
    /* Check address */ \
    if (address >= LMAC_ADDRESS_LAST) { \
        status = error_code; \
        goto errors; \
    } \
}

/*******************************************************************/
static void _LMAC_rx_irq_callback(uint8_t data) {
#ifdef LMAC_DRIVER_MODE_SLAVE
    // Local variables.
    uint32_t uptime_seconds = LMAC_HW_get_uptime_seconds();
    // Manage timeout.
    if (uptime_seconds > (lmac_ctx.first_rx_byte_timestamp_seconds + LMAC_DRIVER_RX_TIMEOUT_SECONDS)) {
        // Reset receiver.
        lmac_ctx.rx_byte_count = 0;
        // Check address.
        if (data != (lmac_ctx.self_address | LMAC_ADDRESS_MARKER)) {
            LMAC_HW_enable_rx();
            return;
        }
    }
#endif
    // Check field index.
    switch (lmac_ctx.rx_byte_count) {
    case LMAC_FRAME_FIELD_INDEX_DESTINATION_ADDRESS:
        // Compare to self address.
        if (data == (lmac_ctx.self_address | LMAC_ADDRESS_MARKER)) {
            lmac_ctx.rx_byte_count++;
        }
#ifdef LMAC_DRIVER_MODE_SLAVE
        // Update first byte timestamp.
        lmac_ctx.first_rx_byte_timestamp_seconds = uptime_seconds;
#endif
        break;
    case LMAC_FRAME_FIELD_INDEX_SOURCE_ADDRESS:
#ifdef LMAC_DRIVER_MODE_MASTER
        // Check if the source address corresponds to the previous node.
        if (data == (lmac_ctx.destination_address | LMAC_ADDRESS_MARKER)) {
            lmac_ctx.rx_byte_count++;
        }
        else {
            lmac_ctx.rx_byte_count = 0;
        }
#else
        // Store source address for next reply.
        lmac_ctx.destination_address = (data & LMAC_ADDRESS_MASK);
        lmac_ctx.rx_byte_count++;
#endif
        break;
    default:
        // Transmit byte to upper layer.
        if (lmac_ctx.rx_irq_callback != NULL) {
            lmac_ctx.rx_irq_callback(data);
        }
        break;
    }
}

/*** LMAC functions ***/

/*******************************************************************/
LMAC_status_t LMAC_init(uint32_t baud_rate, LMAC_rx_irq_cb_t rx_irq_callback) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    uint8_t self_address = 0;
    // Check parameter.
    if (rx_irq_callback == NULL) {
        status = LMAC_ERROR_NULL_PARAMETER;
        goto errors;
    }
    // Init context.
    lmac_ctx.rx_irq_callback = rx_irq_callback;
    lmac_ctx.self_address = LMAC_ADDRESS_LAST;
    lmac_ctx.destination_address = LMAC_ADDRESS_LAST;
    lmac_ctx.rx_byte_count = 0;
#ifdef LMAC_DRIVER_MODE_SLAVE
    lmac_ctx.first_rx_byte_timestamp_seconds = 0;
#endif
    // Init hardware interface.
    status = LMAC_HW_init(baud_rate, &_LMAC_rx_irq_callback, &self_address);
    if (status != LMAC_SUCCESS) goto errors;
    // Check address.
    _LMAC_check_address(self_address, LMAC_ERROR_SELF_ADDRESS);
    // Update local context.
    lmac_ctx.self_address = self_address;
errors:
    return status;
}

/*******************************************************************/
LMAC_status_t LMAC_de_init(void) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    // Release hardware interface.
    status = LMAC_HW_de_init();
    if (status != LMAC_SUCCESS) goto errors;
errors:
    return status;
}

/*******************************************************************/
LMAC_status_t LMAC_enable_rx(void) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    // Release hardware interface.
    status = LMAC_HW_enable_rx();
    if (status != LMAC_SUCCESS) goto errors;
errors:
    return status;
}

/*******************************************************************/
LMAC_status_t LMAC_disable_rx(void) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    // Release hardware interface.
    status = LMAC_HW_disable_rx();
    if (status != LMAC_SUCCESS) goto errors;
errors:
    return status;
}

/*******************************************************************/
LMAC_status_t LMAC_write(uint8_t* data, uint32_t data_size_bytes) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    uint8_t lmac_header[LMAC_FRAME_FIELD_INDEX_DATA];
    // Check address.
    _LMAC_check_address(lmac_ctx.destination_address, LMAC_ERROR_DESTINATION_ADDRESS);
    // Build address header.
    lmac_header[LMAC_FRAME_FIELD_INDEX_DESTINATION_ADDRESS] = (lmac_ctx.destination_address | LMAC_ADDRESS_MARKER);
    lmac_header[LMAC_FRAME_FIELD_INDEX_SOURCE_ADDRESS] = lmac_ctx.self_address;
    // Send header.
    status = LMAC_HW_write(lmac_header, LMAC_FRAME_FIELD_INDEX_DATA);
    if (status != LMAC_SUCCESS) goto errors;
    // Send command.
    status = LMAC_HW_write(data, data_size_bytes);
    if (status != LMAC_SUCCESS) goto errors;
errors:
    lmac_ctx.rx_byte_count = 0;
    return status;
}

#ifdef LMAC_DRIVER_MODE_MASTER
/*******************************************************************/
LMAC_status_t LMAC_set_destination_address(uint8_t destination_address) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    // Check address.
    _LMAC_check_address(destination_address, LMAC_ERROR_DESTINATION_ADDRESS);
    // Update local context.
    lmac_ctx.destination_address = destination_address;
errors:
    return status;
}
#endif

/*** LMAC compilation flags check ***/

#if (!(defined LMAC_DRIVER_MODE_MASTER) && !(defined LMAC_DRIVER_MODE_SLAVE))
#error "lmac-driver: None mode selected"
#endif

#endif /* LMAC_DRIVER_DISABLE */
