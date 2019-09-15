// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <stdio.h>
#include "driver/i2c.h"
#include "ft5x06.h"
#include "esp_log.h"
#include "string.h"

esp_err_t iot_ft5x06_read(ft5x06_handle_t dev, uint8_t start_addr,
                          uint8_t read_num, uint8_t *data_buf)
{
    esp_err_t ret = ESP_FAIL;

    if (data_buf != NULL) {
        i2c_cmd_handle_t cmd = NULL;
        ft5x06_dev_t *device = (ft5x06_dev_t *) dev;
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (device->dev_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, start_addr, ACK_CHECK_EN);
        i2c_master_stop(cmd);
        ret = iot_i2c_bus_cmd_begin(device->bus, cmd, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);
        if (ret != ESP_OK) {
            return ESP_FAIL;
        }
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (device->dev_addr << 1) | READ_BIT, ACK_CHECK_EN);
        if (read_num > 1) {
            i2c_master_read(cmd, data_buf, read_num - 1, ACK_VAL);
        }
        i2c_master_read_byte(cmd, &data_buf[read_num - 1], NACK_VAL);
        i2c_master_stop(cmd);
        ret = iot_i2c_bus_cmd_begin(device->bus, cmd, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);
    }
    return ret;
}

esp_err_t iot_ft5x06_write(ft5x06_handle_t dev, uint8_t start_addr,
                           uint8_t write_num, uint8_t *data_buf)
{
    esp_err_t ret = ESP_FAIL;
    if (data_buf != NULL) {
        ft5x06_dev_t *device = (ft5x06_dev_t *) dev;
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (device->dev_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, start_addr, ACK_CHECK_EN);
        i2c_master_write(cmd, data_buf, write_num, ACK_CHECK_EN);
        i2c_master_stop(cmd);
        ret = iot_i2c_bus_cmd_begin(device->bus, cmd, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);
    }
    return ret;
}

static long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    long divisor = (in_max - in_min);
    if (divisor == 0) {
        return -1; //AVR returns -1, SAM returns 0
    }
    return (x - in_min) * (out_max - out_min) / divisor + out_min;
}

esp_err_t iot_ft5x06_touch_report(ft5x06_handle_t device, touch_info_t *ifo)
{
    if (device == NULL || ifo == NULL) {
        return ESP_FAIL;
    }
    ft5x06_dev_t *dev = (ft5x06_dev_t *) device;
    uint8_t data[32];
    memset(data, 0, sizeof(data));
    if (iot_ft5x06_read(dev, 0x00, 32, data) == ESP_OK) {
        ifo->touch_point = data[2] & 0x7;
        if (ifo->touch_point > 0 && ifo->touch_point <= 5) {

            for (uint8_t i = 0; i < 1; i++) {
                ifo->curx[i] = data[0x03 + i * 6] & 0x0F;
                ifo->curx[i] <<= 8;
                ifo->curx[i] |= data[0x04 + i * 6];
                ifo->cury[i] = data[0x05 + i * 6] & 0x0F;
                ifo->cury[i] <<= 8;
                ifo->cury[i] |= data[0x06 + i * 6];
            }
            ifo->curx[0] = map(ifo->curx[0], 0, 320, 0, 240);
            ifo->cury[0] = map(ifo->cury[0], 0, 320, 0, 240);
            ifo->touch_event = TOUCH_EVT_PRESS;
        } else {
            ifo->touch_event = TOUCH_EVT_RELEASE;
            ifo->touch_point = 0;
        }
    } else {
        ifo->touch_event = TOUCH_EVT_RELEASE;
        ifo->touch_point = 0;
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t iot_ft5x06_init(ft5x06_handle_t dev, ft5x06_cfg_t *cfg)
{
    return ESP_OK;
}

ft5x06_handle_t iot_ft5x06_create(i2c_bus_handle_t bus, uint16_t dev_addr)
{
    ft5x06_dev_t *dev = (ft5x06_dev_t *) calloc(1, sizeof(ft5x06_dev_t));
    if (dev == NULL) {
        ESP_LOGI("FT5X06:", "dev handle calloc fail\n");
        return NULL;
    }
    dev->bus = bus;
    dev->dev_addr = dev_addr;
    dev->x_size = 320;
    dev->y_size = 320;
    dev->xy_swap = false;
    return (ft5x06_handle_t) dev;
}


esp_err_t iot_ft5x06_deinit(ft5x06_handle_t dev)
{
    if (dev == NULL)return ESP_OK;
    free(dev);
    return ESP_OK;
}