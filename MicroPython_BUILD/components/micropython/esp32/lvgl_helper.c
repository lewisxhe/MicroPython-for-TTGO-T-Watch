#include "binary.h"
#include "libs/lvgl/lv_binding/lvgl/lvgl.h"
#include "mperrno.h"
#include "py/runtime.h"
#include "libs/tft/tftspi.h"
#include "libs/ft5x06/ft5x06.h"

typedef struct mp_ptr_t {
    mp_obj_base_t base;
    void *ptr;
} mp_ptr_t;

STATIC mp_int_t mp_ptr_get_buffer(mp_obj_t self_in, mp_buffer_info_t *bufinfo, mp_uint_t flags)
{
    mp_ptr_t *self = MP_OBJ_TO_PTR(self_in);

    if (flags & MP_BUFFER_WRITE) {
        // read-only ptr
        return 1;
    }
    bufinfo->buf = &self->ptr;
    bufinfo->len = sizeof(self->ptr);
    bufinfo->typecode = BYTEARRAY_TYPECODE;
    return 0;
}

#define PTR_OBJ(ptr_global) ptr_global ## _obj
#define DEFINE_PTR_OBJ_TYPE(ptr_obj_type, ptr_type_qstr)\
STATIC const mp_obj_type_t ptr_obj_type = {\
    { &mp_type_type },\
    .name = ptr_type_qstr,\
    .buffer_p = { .get_buffer = mp_ptr_get_buffer }\
}

#define DEFINE_PTR_OBJ(ptr_global)\
DEFINE_PTR_OBJ_TYPE(ptr_global ## _type, MP_QSTR_ ## ptr_global);\
STATIC const mp_ptr_t PTR_OBJ(ptr_global) = {\
    { &ptr_global ## _type },\
    &ptr_global\
}

STATIC void lcd_flush(struct _disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t size = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1) ;
    color_t *p = (color_t *)malloc(sizeof(color_t) * (size));
    if (!p) {
        return;
    }
    for (int i = 0; i < size ; i++) {
        p[i].r = (color_p[i].full >> 8) & 0x0f8;
        p[i].g = (color_p[i].full >> 3) & 0x0fc;
        p[i].b = (color_p[i].full << 3) & 0x0f8;
    }
    if (disp_select()) return;
    send_data(area->x1, area->y1, area->x2, area->y2, size, p, 1);
    if (disp_deselect()) return;
    free(p);
    lv_disp_flush_ready(disp_drv);
}


extern ft5x06_handle_t *ft5x06_handle;
STATIC bool touch_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static touch_info_t ifo;
    iot_ft5x06_touch_report(ft5x06_handle, &ifo);
    if (ifo.touch_event != TOUCH_EVT_RELEASE) {
        data->point.x =  ifo.curx[0];
        data->point.y =  ifo.cury[0];
        data->state = LV_INDEV_STATE_PR ;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
    return false;
}


DEFINE_PTR_OBJ(lcd_flush);
DEFINE_PTR_OBJ(touch_read);


STATIC const mp_rom_map_elem_t lvgl_helper_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_lvgl_helper) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_flush), MP_ROM_PTR(&PTR_OBJ(lcd_flush)) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read), MP_ROM_PTR(&PTR_OBJ(touch_read))},
};


STATIC MP_DEFINE_CONST_DICT (
    mp_module_lvgl_helper_globals,
    lvgl_helper_globals_table
);

const mp_obj_module_t mp_module_lvgl_helper = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *) &mp_module_lvgl_helper_globals
};

