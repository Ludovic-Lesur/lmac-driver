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

#define LMAC_ADDRESS_MARKER         0x80

#define LMAC_END_MARKER             0x17
#define LMAC_END_MAKER_SIZE_BYTES   1

#define LMAC_DATA_SIZE_BYTES(size)  (size - (LMAC_ADDRESS_SIZE_BYTES << 1) - LMAC_END_MAKER_SIZE_BYTES)

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
    LMAC_error_cb_t error_callback;
    uint8_t self_address;
    uint8_t destination_address;
    uint8_t rx_buffer[LMAC_DRIVER_BUFFER_SIZE];
    uint32_t rx_buffer_size;
} LMAC_context_t;

/*** LMAC local global variables ***/

static LMAC_context_t lmac_ctx = {
    .rx_irq_callback = NULL,
    .error_callback = NULL,
    .self_address = LMAC_ADDRESS_LAST,
    .destination_address = LMAC_ADDRESS_LAST,
    .rx_buffer = { [0 ... (LMAC_DRIVER_BUFFER_SIZE - 1)] = 0x00 },
    .rx_buffer_size = 0
};

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
#define _LMAC_error_callback(error_code) { \
    if (lmac_ctx.error_callback != NULL) { \
        lmac_ctx.error_callback(error_code); \
    } \
}

/*******************************************************************/
static void _LMAC_decode_frame(void) {
    // Local variables.
    uint32_t idx = 0;
    // Check destination address.
    if (lmac_ctx.rx_buffer[LMAC_FRAME_FIELD_INDEX_DESTINATION_ADDRESS] != (lmac_ctx.self_address | LMAC_ADDRESS_MARKER)) {
        _LMAC_error_callback(LMAC_ERROR_RX_DESTINATION_ADDRESS);
        goto errors;
    }
#ifdef LMAC_DRIVER_MODE_MASTER
    // Check if the source address corresponds to the previous node.
    if (lmac_ctx.rx_buffer[LMAC_FRAME_FIELD_INDEX_SOURCE_ADDRESS] != lmac_ctx.destination_address) {
        _LMAC_error_callback(LMAC_ERROR_RX_SOURCE_ADDRESS);
        goto errors;
    }
#else
    // Store source address for next reply.
    lmac_ctx.destination_address = lmac_ctx.rx_buffer[LMAC_FRAME_FIELD_INDEX_SOURCE_ADDRESS];
#endif
    // Frame is valid.
    if (lmac_ctx.rx_irq_callback != NULL) {
        // Data bytes loop.
        for (idx = 0; idx < LMAC_DATA_SIZE_BYTES(lmac_ctx.rx_buffer_size); idx++) {
            lmac_ctx.rx_irq_callback(lmac_ctx.rx_buffer[LMAC_FRAME_FIELD_INDEX_DATA + idx]);
        }
    }
errors:
    return;
}

/*******************************************************************/
static void _LMAC_rx_irq_callback(uint8_t data) {
    // Store byte into buffer.
    lmac_ctx.rx_buffer[lmac_ctx.rx_buffer_size] = data;
    // Increment index.
    lmac_ctx.rx_buffer_size++;
    // Check end marker.
    if (data == LMAC_END_MARKER) {
        // Decode frame.
        _LMAC_decode_frame();
        // Reset size.
        lmac_ctx.rx_buffer_size = 0;
    }
    else {
        // Check size.
        if (lmac_ctx.rx_buffer_size >= LMAC_DRIVER_BUFFER_SIZE) {
            // Reset index.
            lmac_ctx.rx_buffer_size = 0;
            // Call error callback.
            _LMAC_error_callback(LMAC_ERROR_RX_DATA_SIZE);
        }
    }
}

/*** LMAC functions ***/

/*******************************************************************/
LMAC_status_t LMAC_init(uint32_t baud_rate, LMAC_rx_irq_cb_t rx_irq_callback, LMAC_error_cb_t error_callback) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    uint8_t self_address = 0;
    uint32_t idx = 0;
    // Check parameter.
    if (rx_irq_callback == NULL) {
        status = LMAC_ERROR_NULL_PARAMETER;
        goto errors;
    }
    // Init context.
    lmac_ctx.rx_irq_callback = rx_irq_callback;
    lmac_ctx.error_callback = error_callback;
    lmac_ctx.self_address = LMAC_ADDRESS_LAST;
    lmac_ctx.destination_address = LMAC_ADDRESS_LAST;
    for (idx = 0; idx < LMAC_DRIVER_BUFFER_SIZE; idx++) {
        lmac_ctx.rx_buffer[idx] = 0x00;
    }
    lmac_ctx.rx_buffer_size = 0;
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
    uint8_t lmac_frame[LMAC_DRIVER_BUFFER_SIZE];
    uint32_t lmac_frame_size = 0;
    uint32_t idx = 0;
    // Check address.
    _LMAC_check_address(lmac_ctx.destination_address, LMAC_ERROR_DESTINATION_ADDRESS);
    // Check size.
    if (data_size_bytes > LMAC_DATA_SIZE_BYTES(LMAC_DRIVER_BUFFER_SIZE)) {
        status = LMAC_ERROR_TX_DATA_SIZE;
        goto errors;
    }
    // Build address header.
    lmac_frame[lmac_frame_size++] = (lmac_ctx.destination_address | LMAC_ADDRESS_MARKER);
    lmac_frame[lmac_frame_size++] = lmac_ctx.self_address;
    // Build data.
    for (idx = 0; idx < data_size_bytes; idx++) {
        lmac_frame[lmac_frame_size++] = data[idx];
    }
    lmac_frame[lmac_frame_size++] = LMAC_END_MARKER;
    // Send frame.
    status = LMAC_HW_write(lmac_frame, lmac_frame_size);
    if (status != LMAC_SUCCESS) goto errors;
errors:
    lmac_ctx.rx_buffer_size = 0;
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
