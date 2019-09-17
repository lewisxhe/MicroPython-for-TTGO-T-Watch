
#pragma once

#include "bma423.h"
#include "../i2c_bus/iot_i2c_bus.h"


typedef struct bma4_dev Bma_t;
typedef struct bma4_accel Accel;
typedef struct bma4_accel_config Acfg;

typedef struct {
    i2c_bus_handle_t bus;
    uint8_t dev_addr;
    Bma_t handle;
    uint16_t status;
    bool initDone;
} bma423_dev_t;

typedef void  *bma423_handle_t;

enum {
    DIRECTION_TOP_EDGE        = 0,
    DIRECTION_BOTTOM_EDGE     = 1,
    DIRECTION_LEFT_EDGE       = 2,
    DIRECTION_RIGHT_EDGE      = 3,
    DIRECTION_DISP_UP         = 4,
    DIRECTION_DISP_DOWN       = 5
} ;

enum{
    BMA_INVALID_STATE = 0,
    BMA_STEP_COUNTER = 1,
    BMA_DOUBLE_WAKEUP = 2,
};

bma423_handle_t bma423_create(i2c_bus_handle_t bus, uint16_t dev_addr);

esp_err_t bma423_deinit(bma423_handle_t dev);

void bma_reset(bma423_handle_t dev);

bool bma_getAccel(bma423_handle_t dev, Accel *acc);

bool bma423_reinit(bma423_handle_t dev);

uint8_t bma423_direction(bma423_handle_t dev);

float bma423_temperature(bma423_handle_t dev);

void bma423_enableAccel(bma423_handle_t dev);

void bma423_disalbeIrq(bma423_handle_t dev);

void bma423_enableIrq(bma423_handle_t dev);

void bma423_attachInterrupt(bma423_handle_t dev);

bool bma423_readInterrupt(bma423_handle_t dev);

uint32_t bma423_getCounter(bma423_handle_t dev);

bool bma423_clearCounter(bma423_handle_t dev);

bool bma423_isStepCounter(bma423_handle_t dev);

bool bma423_isDoubleClick(bma423_handle_t dev);
