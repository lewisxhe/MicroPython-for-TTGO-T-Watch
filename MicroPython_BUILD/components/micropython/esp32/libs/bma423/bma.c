#include "bma.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>


static const char *TAG = "[BMA423]";
static bma423_dev_t *bma_dev = NULL;

static esp_err_t bma_read(bma423_handle_t dev, uint8_t addr, uint8_t start_addr,
                          uint8_t read_num, uint8_t *data_buf)
{
    esp_err_t ret = ESP_FAIL;
    bma423_dev_t *device = (bma423_dev_t *) dev;
    if (data_buf != NULL) {
        i2c_cmd_handle_t cmd = NULL;
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
        if (ret != ESP_OK) {
        }
    }
    return ret;
}

static esp_err_t bma_write(bma423_handle_t dev, uint8_t addr, uint8_t start_addr,
                           uint8_t write_num, uint8_t *data_buf)
{
    uint32_t i = 0;
    i2c_cmd_handle_t cmd;
    esp_err_t ret = ESP_FAIL;
    uint32_t writeNum = write_num;
    bma423_dev_t *device = (bma423_dev_t *) dev;
    if (data_buf != NULL) {
        for (uint32_t j = 0; j < write_num; j += 8) {
            cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (device->dev_addr << 1) | WRITE_BIT,
                                  ACK_CHECK_EN);
            i2c_master_write_byte(cmd, start_addr, ACK_CHECK_EN);
            for (i = j; i < ((writeNum >= 8) ? 8 : writeNum); i++) {
                i2c_master_write_byte(cmd, data_buf[i], ACK_CHECK_EN);
            }
            i2c_master_stop(cmd);
            ret = iot_i2c_bus_cmd_begin(device->bus, cmd, 1000 / portTICK_RATE_MS);
            i2c_cmd_link_delete(cmd);

            writeNum -= 8;              //write num count
            if (ret == ESP_FAIL) {
                ESP_LOGE(TAG, "bma_write FAILED :%x", ret);
                return ret;
            }
        }
    }
    return ret;
}

static void delay(uint32_t millisecond)
{
    vTaskDelay(millisecond / portTICK_PERIOD_MS);
}

static uint16_t read(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    return bma_read(bma_dev, addr, reg, len, data);
}

static uint16_t write(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    return bma_write(bma_dev, addr, reg, len, data);
}

bma423_handle_t bma423_create(i2c_bus_handle_t bus, uint16_t dev_addr)
{
    if (bma_dev)return (bma423_handle_t) bma_dev;

    bma_dev = (bma423_dev_t *) calloc(1, sizeof(bma423_dev_t));
    if (bma_dev == NULL) {
        ESP_LOGI("bma423:", "dev handle calloc fail\n");
        return NULL;
    }
    memset(bma_dev, 0, sizeof(bma423_dev_t));

    bma_dev->bus = bus;
    bma_dev->dev_addr = dev_addr;
    return (bma423_handle_t) bma_dev;
}

void bma_reset(bma423_handle_t dev)
{
    if (dev == NULL)return 0;
    uint8_t reg = 0xB6;
    bma423_dev_t *device = (bma423_dev_t *) dev;
    bma_write(dev, device->dev_addr, 0x7E, 1, &reg);
}

uint16_t bma_config(bma423_handle_t dev)
{
    if (dev == NULL)return 0;
    bma423_dev_t *device = (bma423_dev_t *) dev;
    uint16_t rslt =  bma423_write_config_file(&device->handle);
    if (rslt != BMA4_OK) {
        ESP_LOGE(TAG, "BMA423 Configure FAILED %x", rslt);
    }
    return rslt;
}

bool bma_getAccel(bma423_handle_t dev, Accel *acc)
{
    if (dev == NULL)return 0;
    if (acc == NULL)return 0;
    bma423_dev_t *device = (bma423_dev_t *) dev;
    memset(acc, 0, sizeof(Accel));
    if (bma4_read_accel_xyz(acc, &device->handle) != BMA4_OK) {
        return false;
    }
    return true;
}

esp_err_t bma423_deinit(bma423_handle_t dev)
{
    if (dev != NULL)
        free(dev);
    if (bma_dev != NULL) {
        free(bma_dev);
        bma_dev = NULL;
    }
    return ESP_OK;
}

bool bma423_reinit(bma423_handle_t dev)
{
    if (dev == NULL)return 0;
    bma423_dev_t *mdev = (bma423_dev_t *)dev;


    mdev->handle.dev_addr        = mdev->dev_addr;
    mdev->handle.interface       = BMA4_I2C_INTERFACE;
    mdev->handle.bus_read        = read;
    mdev->handle.bus_write       = write;
    mdev->handle.delay           = delay;
    mdev->handle.read_write_len  = 8;
    mdev->handle.resolution      = 12;
    mdev->handle.feature_len     = BMA423_FEATURE_SIZE;

    bma_reset(dev);

    vTaskDelay(30 / portTICK_PERIOD_MS);

    if (bma423_init(&mdev->handle) != BMA4_OK) {
        ESP_LOGE(TAG, "BMA Init FAILED");
        return false;
    }

    if (bma_config(dev) == BMA4_OK) {
        mdev->initDone = true;
        return true;
    }
    return false;
}

uint8_t bma423_direction(bma423_handle_t dev)
{
    if (dev == NULL)return 0;
    bma423_dev_t *mdev = (bma423_dev_t *)dev;

    if (!mdev->initDone)return 0;

    Accel acc;
    if (bma4_read_accel_xyz(&acc, &mdev->handle) != BMA4_OK) {
        return 0;
    }
    uint16_t absX = abs(acc.x);
    uint16_t absY = abs(acc.y);
    uint16_t absZ = abs(acc.z);

    if ((absZ > absX) && (absZ > absY)) {
        if (acc.z > 0) {
            return  DIRECTION_DISP_DOWN;
        } else {
            return DIRECTION_DISP_UP;
        }
    } else if ((absY > absX) && (absY > absZ)) {
        if (acc.y > 0) {
            return DIRECTION_BOTTOM_EDGE;
        } else {
            return  DIRECTION_TOP_EDGE;
        }
    } else {
        if (acc.x < 0) {
            return  DIRECTION_RIGHT_EDGE;
        } else {
            return DIRECTION_LEFT_EDGE;
        }
    }
}

float bma423_temperature(bma423_handle_t dev)
{
    if (dev == NULL)return 0;
    bma423_dev_t *mdev = (bma423_dev_t *)dev;
    if (!mdev->initDone)return 0;
    int32_t data = 0;
    uint16_t rslt = bma4_get_temperature(&data, BMA4_DEG, &mdev->handle);
    if (rslt != BMA4_OK) return 0;
    float res = (float)data / (float)BMA4_SCALE_TEMP;
    /* 0x80 - temp read from the register and 23 is the ambient temp added.
     * If the temp read from register is 0x80, it means no valid
     * information is available */
    if (((data - 23) / BMA4_SCALE_TEMP) == 0x80) {
        res = 0;
    }
    return res;
}

void bma423_enableAccel(bma423_handle_t dev)
{
    if (dev == NULL)return;
    bma423_dev_t *mdev = (bma423_dev_t *)dev;
    if (!mdev->initDone)return;
    if (bma4_set_accel_enable(BMA4_ENABLE, &mdev->handle)) {
        return;
    }
    Acfg cfg;
    cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    cfg.range = BMA4_ACCEL_RANGE_2G;
    cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    cfg.perf_mode = BMA4_CONTINUOUS_MODE;

    if (bma4_set_accel_config(&cfg, &mdev->handle)) {
        ESP_LOGE(TAG, "BMA423 Accel Config FAILED");
    }
}

void bma423_disalbeIrq(bma423_handle_t dev)
{
    if (dev == NULL)return;
    bma423_dev_t *mdev = (bma423_dev_t *)dev;
    if (!mdev->initDone)return ;
    bma423_map_interrupt(BMA4_INTR1_MAP, BMA423_STEP_CNTR_INT /* |BMA423_WAKEUP_INT*/, BMA4_DISABLE, &mdev->handle);
}

void bma423_enableIrq(bma423_handle_t dev)
{
    if (dev == NULL)return;
    bma423_dev_t *mdev = (bma423_dev_t *)dev;
    if (!mdev->initDone)return ;
    bma423_map_interrupt(BMA4_INTR1_MAP, BMA423_STEP_CNTR_INT /* |BMA423_WAKEUP_INT*/, BMA4_ENABLE, &mdev->handle);
}

//attachInterrupt bma423 int1
void bma423_attachInterrupt(bma423_handle_t dev)
{
    if (dev == NULL)return;
    bma423_dev_t *mdev = (bma423_dev_t *)dev;
    if (!mdev->initDone)return;
    uint16_t rslt = BMA4_OK;
    bma423_enableAccel(dev);
    // rslt |= bma423_reset_step_counter(&mdev->handle);
    rslt |= bma423_step_detector_enable(BMA4_ENABLE, &mdev->handle);
    rslt |= bma423_feature_enable(BMA423_STEP_CNTR, BMA4_ENABLE, &mdev->handle);
    rslt |= bma423_feature_enable(BMA423_WAKEUP, BMA4_ENABLE, &mdev->handle);
    rslt |= bma423_step_counter_set_watermark(100, &mdev->handle);

    rslt |= bma423_map_interrupt(BMA4_INTR1_MAP, BMA423_STEP_CNTR_INT | BMA423_WAKEUP_INT, BMA4_ENABLE, &mdev->handle);
    struct bma4_int_pin_config config ;
    config.edge_ctrl = BMA4_LEVEL_TRIGGER;
    config.lvl = BMA4_ACTIVE_HIGH;
    config.od = BMA4_PUSH_PULL;
    config.output_en = BMA4_OUTPUT_ENABLE;
    config.input_en = BMA4_INPUT_DISABLE;
    rslt |= bma4_set_int_pin_config(&config, BMA4_INTR1_MAP, &mdev->handle);

    if (rslt != BMA4_OK) {
        ESP_LOGE(TAG, "BMA423 attachInterrupt FAILED");
    }
}

bool bma423_readInterrupt(bma423_handle_t dev)
{
    if (dev == NULL)return 0;
    bma423_dev_t *mdev = (bma423_dev_t *)dev;
    if (!mdev->initDone)return false;
    return bma423_read_int_status(&mdev->status, &mdev->handle) == BMA4_OK;
}

uint32_t bma423_getCounter(bma423_handle_t dev)
{
    if (dev == NULL)return 0;
    bma423_dev_t *mdev = (bma423_dev_t *)dev;
    if (!mdev->initDone)return 0;
    uint32_t stepCount;
    if (bma423_step_counter_output(&stepCount, &mdev->handle) == BMA4_OK) {
        return stepCount;
    }
    return 0;
}

bool bma423_clearCounter(bma423_handle_t dev)
{
    if (dev == NULL)return 0;
    bma423_dev_t *mdev = (bma423_dev_t *)dev;
    if (!mdev->initDone)return 0;
    return bma423_reset_step_counter(&mdev->handle) == BMA4_OK;
}

bool bma423_isStepCounter(bma423_handle_t dev)
{
    if (dev == NULL)return false;
    bma423_dev_t *mdev = (bma423_dev_t *)dev;
    if (!mdev->initDone)return false;
    return (bool)(BMA423_STEP_CNTR_INT & mdev->status);
}

bool bma423_isDoubleClick(bma423_handle_t dev)
{
    if (dev == NULL)return false;
    bma423_dev_t *mdev = (bma423_dev_t *)dev;
    if (!mdev->initDone)return false;
    return (bool)(BMA423_WAKEUP_INT & mdev->status);
}
