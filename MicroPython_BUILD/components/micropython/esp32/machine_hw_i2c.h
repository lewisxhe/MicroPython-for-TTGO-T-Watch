
#ifndef _MACHINE_HW_SPI_H_
#define __MACHINE_HW_SPI_H_

#include "sdkconfig.h"

#include "py/runtime.h"
#include "driver/i2c.h"

typedef struct _mp_machine_i2c_obj_t {
    mp_obj_base_t base;
    uint32_t speed;
    uint8_t mode;
    uint8_t scl;
    uint8_t sda;
    int8_t bus_id;
    i2c_cmd_handle_t cmd;
    uint16_t rx_buflen;			// low level commands receive buffer length
    uint16_t rx_bufidx;			// low level commands receive buffer index
    uint8_t *rx_data;			// low level commands receive buffer
    int8_t slave_addr;			// slave only, slave 8-bit address
    uint16_t slave_buflen;		// slave only, data buffer length
    uint16_t slave_rolen;       // slave only, read only buffer area length
    uint32_t *slave_cb;	        // slave only, slave callback function
    bool slave_busy;
    uint8_t slave_cbtype;
} mp_machine_i2c_obj_t;

#endif