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

#define LMAC_ADDRESS_MARKER                         0x80
#define LMAC_DATA_MASK                              0x7F

#define LMAC_DESTINATION_ADDRESS_INDEX              0

#define LMAC_SOURCE_ADDRESS_INDEX                   (LMAC_DESTINATION_ADDRESS_INDEX + LMAC_ADDRESS_SIZE_BYTES)

#define LMAC_DATA_INDEX                             (LMAC_SOURCE_ADDRESS_INDEX + LMAC_ADDRESS_SIZE_BYTES)
#define LMAC_DATA_SIZE_BYTES(frame_size)            (frame_size - (LMAC_ADDRESS_SIZE_BYTES << 1) - LMAC_CHECKSUM_SIZE_BYTES - LMAC_END_MARKER_SIZE_BYTES)

#define LMAC_CHECKSUM_INDEX(frame_size)             (LMAC_DATA_INDEX + LMAC_DATA_SIZE_BYTES(frame_size))
#define LMAC_CHECKSUM_RANGE_SIZE_BYTES(frame_size)  (frame_size - LMAC_CHECKSUM_SIZE_BYTES - LMAC_END_MARKER_SIZE_BYTES)
#define LMAC_CHECKSUM_SIZE_BYTES                    2

#define LMAC_END_MARKER                             0x7F
#define LMAC_END_MARKER_SIZE_BYTES                  1

/*** LMAC local structures ***/

/*******************************************************************/
typedef struct {
    uint8_t init_flag;
    LMAC_rx_irq_cb_t rx_irq_callback;
    uint8_t self_address;
    volatile uint8_t destination_address;
    volatile uint8_t rx_buffer[LMAC_DRIVER_BUFFER_SIZE];
    volatile uint32_t rx_buffer_size;
} LMAC_context_t;

/*** LMAC local global variables ***/

static LMAC_context_t lmac_ctx = {
    .init_flag = 0,
    .rx_irq_callback = NULL,
    .self_address = LMAC_ADDRESS_LAST,
    .destination_address = LMAC_ADDRESS_LAST,
    .rx_buffer = { [0 ... (LMAC_DRIVER_BUFFER_SIZE - 1)] = 0x00 },
    .rx_buffer_size = 0
};

/*** LMAC local functions ***/

/*******************************************************************/
#define _LMAC_check_address(address, error_code) { \
    /* Check address */ \
    if (address > LMAC_ADDRESS_LAST) { \
        status = error_code; \
        goto errors; \
    } \
}

/*******************************************************************/
static void _LMAC_compute_checksum(uint8_t* data, uint32_t data_size_bytes, uint8_t* cksl, uint8_t* cksh) {
    // Local variables.
    uint32_t idx = 0;
    // Reset result.
    (*cksl) = 0;
    (*cksh) = 0;
    // Fletcher-16 algorithm.
    for (idx = 0; idx < data_size_bytes; idx++) {
        (*cksl) = (((*cksl) + data[idx]) & LMAC_DATA_MASK);
        (*cksh) = (((*cksh) + (*cksl)) & LMAC_DATA_MASK);
    }
    // Change end marker.
    if ((*cksl) == LMAC_END_MARKER) {
        (*cksl) = 0x00;
    }
    if ((*cksh) == LMAC_END_MARKER) {
        (*cksh) = 0x00;
    }
}

/*******************************************************************/
static void _LMAC_decode_frame(void) {
    // Local variables.
    uint8_t cksl = 0;
    uint8_t cksh = 0;
    uint32_t idx = 0;
    // Check destination address.
    if (lmac_ctx.rx_buffer[LMAC_DESTINATION_ADDRESS_INDEX] != (lmac_ctx.self_address | LMAC_ADDRESS_MARKER)) {
        LMAC_HW_stack_error(LMAC_ERROR_RX_DESTINATION_ADDRESS);
        goto errors;
    }
#ifdef LMAC_DRIVER_MODE_MASTER
    // Check if the source address corresponds to the previous node.
    if (lmac_ctx.rx_buffer[LMAC_SOURCE_ADDRESS_INDEX] != lmac_ctx.destination_address) {
        LMAC_HW_stack_error(LMAC_ERROR_RX_SOURCE_ADDRESS);
        goto errors;
    }
#else
    // Store source address for next reply.
    lmac_ctx.destination_address = lmac_ctx.rx_buffer[LMAC_SOURCE_ADDRESS_INDEX];
#endif
    // Compute checksum.
    _LMAC_compute_checksum((uint8_t*) lmac_ctx.rx_buffer, LMAC_CHECKSUM_RANGE_SIZE_BYTES(lmac_ctx.rx_buffer_size), &cksl, &cksh);
    // Verify checksum.
    if ((lmac_ctx.rx_buffer[LMAC_CHECKSUM_INDEX(lmac_ctx.rx_buffer_size)] != cksh) || (lmac_ctx.rx_buffer[LMAC_CHECKSUM_INDEX(lmac_ctx.rx_buffer_size) + 1] != cksl)) {
        LMAC_HW_stack_error(LMAC_ERROR_RX_CHECKSUM);
        goto errors;
    }
    // Frame is valid.
    if (lmac_ctx.rx_irq_callback != NULL) {
        // Data bytes loop.
        for (idx = 0; idx < LMAC_DATA_SIZE_BYTES(lmac_ctx.rx_buffer_size); idx++) {
            lmac_ctx.rx_irq_callback(lmac_ctx.rx_buffer[LMAC_DATA_INDEX + idx]);
        }
    }
errors:
    return;
}

/*******************************************************************/
static void _LMAC_rx_irq_callback(uint8_t data) {
    // Store byte into buffer.
    lmac_ctx.rx_buffer[lmac_ctx.rx_buffer_size] = data;
    lmac_ctx.rx_buffer_size++;
    // Check end marker.
    if (data == LMAC_END_MARKER) {
        // Decode frame.
        _LMAC_decode_frame();
        // Restart reception.
        lmac_ctx.rx_buffer_size = 0;
    }
    else {
        // Check size.
        if (lmac_ctx.rx_buffer_size >= LMAC_DRIVER_BUFFER_SIZE) {
            // Restart reception.
            LMAC_HW_stack_error(LMAC_ERROR_RX_DATA_SIZE);
            lmac_ctx.rx_buffer_size = 0;
        }
    }
}

/*** LMAC functions ***/

/*******************************************************************/
LMAC_status_t LMAC_init(uint32_t baud_rate, LMAC_rx_irq_cb_t rx_irq_callback) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    uint8_t self_address = 0;
    // Check state.
    if (lmac_ctx.init_flag != 0) {
        status = LMAC_ERROR_ALREADY_INITIALIZED;
        goto errors;
    }
    // Init context.
    lmac_ctx.rx_irq_callback = rx_irq_callback;
    lmac_ctx.self_address = LMAC_ADDRESS_LAST;
    lmac_ctx.destination_address = LMAC_ADDRESS_LAST;
    lmac_ctx.rx_buffer_size = 0;
    // Init hardware interface.
    status = LMAC_HW_init(baud_rate, &_LMAC_rx_irq_callback, &self_address);
    if (status != LMAC_SUCCESS) goto errors;
    // Check address.
    _LMAC_check_address(self_address, LMAC_ERROR_SELF_ADDRESS);
    // Update local context.
    lmac_ctx.self_address = self_address;
    lmac_ctx.init_flag = 1;
errors:
    return status;
}

/*******************************************************************/
LMAC_status_t LMAC_de_init(void) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    // Check state.
    if (lmac_ctx.init_flag == 0) {
        status = LMAC_ERROR_UNINITIALIZED;
        goto errors;
    }
    // Update initialization flag.
    lmac_ctx.init_flag = 0;
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
    // Check state.
    if (lmac_ctx.init_flag == 0) {
        status = LMAC_ERROR_UNINITIALIZED;
        goto errors;
    }
    // Release hardware interface.
    status = LMAC_HW_enable_rx();
    if (status != LMAC_SUCCESS) goto errors;
errors:
    lmac_ctx.rx_buffer_size = 0;
    return status;
}

/*******************************************************************/
LMAC_status_t LMAC_disable_rx(void) {
    // Local variables.
    LMAC_status_t status = LMAC_SUCCESS;
    // Check state.
    if (lmac_ctx.init_flag == 0) {
        status = LMAC_ERROR_UNINITIALIZED;
        goto errors;
    }
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
    uint8_t cksl = 0;
    uint8_t cksh = 0;
    uint32_t idx = 0;
    // Check state.
    if (lmac_ctx.init_flag == 0) {
        status = LMAC_ERROR_UNINITIALIZED;
        goto errors;
    }
    // Check parameters.
    if (data == NULL) {
        status = LMAC_ERROR_NULL_PARAMETER;
        goto errors;
    }
    if (data_size_bytes > LMAC_DATA_SIZE_BYTES(LMAC_DRIVER_BUFFER_SIZE)) {
        status = LMAC_ERROR_TX_DATA_SIZE;
        goto errors;
    }
    // Check address.
    _LMAC_check_address(lmac_ctx.destination_address, LMAC_ERROR_DESTINATION_ADDRESS);
    // Build address header.
    lmac_frame[lmac_frame_size++] = (lmac_ctx.destination_address | LMAC_ADDRESS_MARKER);
    lmac_frame[lmac_frame_size++] = lmac_ctx.self_address;
    // Build data.
    for (idx = 0; idx < data_size_bytes; idx++) {
        // Check end marker and byte mask.
        if (((data[idx] & LMAC_ADDRESS_MARKER) != 0) || (data[idx] == LMAC_END_MARKER)) {
            status = LMAC_ERROR_TX_DATA_INVALID;
            goto errors;
        }
        lmac_frame[lmac_frame_size++] = data[idx];
    }
    // Compute checksum.
    _LMAC_compute_checksum(lmac_frame, lmac_frame_size, &cksl, &cksh);
    lmac_frame[lmac_frame_size++] = cksh;
    lmac_frame[lmac_frame_size++] = cksl;
    // End marker.
    lmac_frame[lmac_frame_size++] = LMAC_END_MARKER;
    // Send frame.
    status = LMAC_HW_write(lmac_frame, lmac_frame_size);
    if (status != LMAC_SUCCESS) goto errors;
errors:
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
