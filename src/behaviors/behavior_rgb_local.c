/*
 * Copyright (c) 2026
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_rgb_local

#include <errno.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <dt-bindings/zmk/rgb.h>
#include <zmk/rgb_underglow.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#define RGB_LOC_SYNC_CMD 1000

static int rgb_apply_sync_defaults(void) {
    int err;
    int rc = 0;

    struct zmk_led_hsb sync_hsb = {
        .h = CONFIG_ZMK_RGB_UNDERGLOW_HUE_START,
        .s = CONFIG_ZMK_RGB_UNDERGLOW_SAT_START,
        .b = CONFIG_ZMK_RGB_UNDERGLOW_BRT_START,
    };

    err = zmk_rgb_underglow_on();
    if (err < 0 && rc == 0) {
        rc = err;
    }

    err = zmk_rgb_underglow_set_hsb(sync_hsb);
    if (err < 0 && rc == 0) {
        rc = err;
    }

    err = zmk_rgb_underglow_select_effect(CONFIG_ZMK_RGB_UNDERGLOW_EFF_START);
    if (err < 0 && rc == 0) {
        rc = err;
    }

    return rc;
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    if (!IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW)) {
        return ZMK_BEHAVIOR_OPAQUE;
    }

    switch (binding->param1) {
    case RGB_TOG_CMD:
        return zmk_rgb_underglow_toggle();
    case RGB_ON_CMD:
        return zmk_rgb_underglow_on();
    case RGB_OFF_CMD:
        return zmk_rgb_underglow_off();
    case RGB_HUI_CMD:
        return zmk_rgb_underglow_change_hue(1);
    case RGB_HUD_CMD:
        return zmk_rgb_underglow_change_hue(-1);
    case RGB_SAI_CMD:
        return zmk_rgb_underglow_change_sat(1);
    case RGB_SAD_CMD:
        return zmk_rgb_underglow_change_sat(-1);
    case RGB_BRI_CMD:
        return zmk_rgb_underglow_change_brt(1);
    case RGB_BRD_CMD:
        return zmk_rgb_underglow_change_brt(-1);
    case RGB_SPI_CMD:
        return zmk_rgb_underglow_change_spd(1);
    case RGB_SPD_CMD:
        return zmk_rgb_underglow_change_spd(-1);
    case RGB_EFF_CMD:
        return zmk_rgb_underglow_cycle_effect(1);
    case RGB_EFR_CMD:
        return zmk_rgb_underglow_cycle_effect(-1);
    case RGB_LOC_SYNC_CMD:
        return rgb_apply_sync_defaults();
    default:
        return -ENOTSUP;
    }
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_rgb_local_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &behavior_rgb_local_driver_api);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
