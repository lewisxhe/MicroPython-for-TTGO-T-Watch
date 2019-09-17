/*
 * This file is part of the MicroPython ESP32 project, https://github.com/lewisxhe/MicroPython_ESP32_psRAM_LoBo
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 lewisxhe (https://github.com/lewisxhe)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
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
