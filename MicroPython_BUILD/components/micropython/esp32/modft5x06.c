#include "sdkconfig.h"


#ifdef CONFIG_MICROPY_USE_FT5X06

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "rom/ets_sys.h"
#include "esp_system.h"
#include "esp_task_wdt.h"

#include "driver/gpio.h"

#include "machine_hw_i2c.h"
#include "modmachine.h"
#include "ft5x06/ft5x06.h"

#include "py/obj.h"
#include "mpconfig.h"
#include "runtime.h"

#include "esp_log.h"

static i2c_bus_t *i2c_handle = NULL;
 ft5x06_handle_t *ft5x06_handle = NULL;

mp_obj_t touchscreen_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    enum {
        ARG_i2c,
    };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_i2c, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);


    mp_machine_i2c_obj_t *config;
    config = (mp_machine_i2c_obj_t *)args[ARG_i2c].u_obj;

    if (config == mp_const_none) {
        mp_raise_ValueError("must assignation i2c port");
    }

    if (i2c_handle) {
        return mp_const_none;
    }

    i2c_handle = (i2c_bus_t *) calloc(1, sizeof(i2c_bus_t));
    i2c_handle->i2c_port = config->bus_id;

    ft5x06_handle = iot_ft5x06_create((i2c_bus_handle_t *)i2c_handle, 0x38);
    if ( ft5x06_handle == NULL)
        mp_raise_ValueError("obj create fail");

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(touchscreen_init_obj, 0, touchscreen_init);

mp_obj_t touchscreen_deinit(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    iot_ft5x06_deinit(ft5x06_handle);
    ft5x06_handle = NULL;
    if (i2c_handle) {
        free(i2c_handle);
        i2c_handle = NULL;
    }
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(touchscreen_deinit_obj, 0, touchscreen_deinit);


mp_obj_t touchscreen_read(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args)
{
    static touch_info_t ifo;

    if (ft5x06_handle == NULL) {
        mp_raise_ValueError("ft5x06 Uninitialized");
        return mp_const_none;
    }
    memset(&ifo, 0, sizeof(ifo));
    if (iot_ft5x06_touch_report(ft5x06_handle, &ifo) != 0) {
        mp_raise_ValueError("ft5x06 read fail");
        return mp_const_none;
    }
    mp_obj_t value[3] ;

    value[0] = mp_obj_new_int(ifo.touch_event);
    value[1] = mp_obj_new_int(ifo.curx[0]);
    value[2] = mp_obj_new_int(ifo.cury[0]);
    mp_obj_t t = mp_obj_new_tuple(3, value);
    return t;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(touchscreen_read_obj, 0, touchscreen_read);


STATIC const mp_map_elem_t globals_dict_table[] = {
    {MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_touchscreen)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_init), (mp_obj_t) &touchscreen_init_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR___del__), (mp_obj_t) &touchscreen_deinit_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t) &touchscreen_read_obj},
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t mp_module_touchscreen = {
    .base = {&mp_type_module},
    .globals = (mp_obj_t) &globals_dict,
};

#endif

