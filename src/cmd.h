#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ipc_client.h"
#include "ipc_common.h"
#include "icons.h"

extern bool g_json_output;

// ========== 枚举 <-> 字符串 转换 ==========

typedef struct {
    const char *name;
    int value;
} enum_map_t;

static const enum_map_t screen_map[] = {
    {"mainmenu",    curr_screen_t_SCREEN_MAINMENU},
    {"oplist",      curr_screen_t_SCREEN_OPLIST},
    {"sysinfo",     curr_screen_t_SCREEN_SYSINFO},
    {"spinner",     curr_screen_t_SCREEN_SPINNER},
    {"displayimg",  curr_screen_t_SCREEN_DISPLAYIMG},
    {"filemanager", curr_screen_t_SCREEN_FILEMANAGER},
    {"settings",    curr_screen_t_SCREEN_SETTINGS},
    {"warning",     curr_screen_t_SCREEN_WARNING},
    {"confirm",     curr_screen_t_SCREEN_CONFIRM},
    {"applist",     curr_screen_t_SCREEN_APPLIST},
    {NULL, -1}
};

static const enum_map_t transition_map[] = {
    {"fade",  TRANSITION_TYPE_FADE},
    {"move",  TRANSITION_TYPE_MOVE},
    {"swipe", TRANSITION_TYPE_SWIPE},
    {"none",  TRANSITION_TYPE_NONE},
    {NULL, -1}
};

static const enum_map_t prts_state_map[] = {
    {"idle",            PRTS_STATE_IDLE},
    {"transition_in",   PRTS_STATE_TRANSITION_IN},
    {"intro",           PRTS_STATE_INTRO},
    {"transition_loop", PRTS_STATE_TRANSITION_LOOP},
    {"pre_opinfo",      PRTS_STATE_PRE_OPINFO},
    {NULL, -1}
};

static const enum_map_t sw_mode_map[] = {
    {"sequence", sw_mode_t_SW_MODE_SEQUENCE},
    {"random",   sw_mode_t_SW_MODE_RANDOM},
    {"manual",   sw_mode_t_SW_RANDOM_MANUAL},
    {NULL, -1}
};

static const enum_map_t sw_interval_map[] = {
    {"1min",  sw_interval_t_SW_INTERVAL_1MIN},
    {"3min",  sw_interval_t_SW_INTERVAL_3MIN},
    {"5min",  sw_interval_t_SW_INTERVAL_5MIN},
    {"10min", sw_interval_t_SW_INTERVAL_10MIN},
    {"30min", sw_interval_t_SW_INTERVAL_30MIN},
    {NULL, -1}
};

static const enum_map_t usb_mode_map[] = {
    {"mtp",       usb_mode_t_MTP},
    {"serial",    usb_mode_t_SERIAL},
    {"rndis",     usb_mode_t_RNDIS},
    {"none",      usb_mode_t_NONE},
    {"epmanager", usb_mode_t_EPMANAGER},
    {NULL, -1}
};

static const enum_map_t exitcode_map[] = {
    {"normal",      EXITCODE_NORMAL},
    {"restart",     EXITCODE_RESTART_APP},
    {"appstart",    EXITCODE_APPSTART},
    {"shutdown",    EXITCODE_SHUTDOWN},
    {"format_sd",   EXITCODE_FORMAT_SD_CARD},
    {"srgn_config", EXITCODE_SRGN_CONFIG},
    {NULL, -1}
};

static inline int enum_from_str(const enum_map_t *map, const char *name, int *out) {
    for (const enum_map_t *e = map; e->name; e++) {
        if (strcasecmp(e->name, name) == 0) {
            *out = e->value;
            return 0;
        }
    }
    return -1;
}

static inline const char *enum_to_str(const enum_map_t *map, int value) {
    for (const enum_map_t *e = map; e->name; e++) {
        if (e->value == value)
            return e->name;
    }
    return "unknown";
}

static inline void enum_print_options(const enum_map_t *map) {
    for (const enum_map_t *e = map; e->name; e++) {
        fprintf(stderr, "  %s (%d)\n", e->name, e->value);
    }
}

// ========== 图标名称映射 ==========

typedef struct {
    const char *name;
    const char *value;
} icon_map_entry_t;

static const icon_map_entry_t icon_map[] = {
    {"heart_crack",             UI_ICON_HEART_CRACK},
    {"dumpster_fire",           UI_ICON_DUMPSTER_FIRE},
    {"gear",                    UI_ICON_GEAR},
    {"user",                    UI_ICON_USER},
    {"mobile_screen",           UI_ICON_MOBILE_SCREEN},
    {"file",                    UI_ICON_FILE},
    {"battery_full",            UI_ICON_BATTERY_FULL},
    {"battery_three_quarters",  UI_ICON_BATTERY_THREE_QUARTERS},
    {"battery_half",            UI_ICON_BATTERY_HALF},
    {"battery_quarter",         UI_ICON_BATTERY_QUARTER},
    {"battery_empty",           UI_ICON_BATTERY_EMPTY},
    {"triangle_exclamation",    UI_ICON_TRIANGLE_EXCLAMATION},
    {"bolt",                    UI_ICON_BOLT},
    {"image_portrait",          UI_ICON_IMAGE_PORTRAIT},
    {"box_archive",             UI_ICON_BOX_ARCHIVE},
    {"images",                  UI_ICON_IMAGES},
    {"sun",                     UI_ICON_SUN},
    {"cat",                     UI_ICON_CAT},
    {"location_crosshairs",     UI_ICON_LOCATION_CROSSHAIRS},
    {"xmark",                   UI_ICON_XMARK},
    {"check",                   UI_ICON_CHECK},
    {"question",                UI_ICON_QUESTION},
    {"shield",                  UI_ICON_SHIELD},
    {"car_burst",               UI_ICON_CAR_BURST},
    {"border_none",             UI_ICON_BORDER_NONE},
    {"thumbs_down",             UI_ICON_THUMBS_DOWN},
    {"thumbs_up",               UI_ICON_THUMBS_UP},
    {"fire_extinguisher",       UI_ICON_FIRE_EXTINGUISHER},
    {"square_arrow_up_right",   UI_ICON_SQUARE_ARROW_UP_RIGHT},
    {"heart",                   UI_ICON_HEART},
    {"face_grin_beam_sweat",    UI_ICON_FACE_GRIN_BEAM_SWEAT},
    {"face_smile_wink",         UI_ICON_FACE_SMILE_WINK},
    {"face_kiss_beam",          UI_ICON_FACE_KISS_BEAM},
    {"poo",                     UI_ICON_POO},
    {"volcano",                 UI_ICON_VOLCANO},
    {"code_pull_request",       UI_ICON_CODE_PULL_REQUEST},
    {"face_flushed",            UI_ICON_FACE_FLUSHED},
    {"face_sad_cry",            UI_ICON_FACE_SAD_CRY},
    {"sd_card",                 UI_ICON_SD_CARD},
    {NULL, NULL}
};

static inline const char *icon_from_name(const char *name) {
    for (const icon_map_entry_t *e = icon_map; e->name; e++) {
        if (strcasecmp(e->name, name) == 0)
            return e->value;
    }
    return NULL;
}

static inline void icon_print_options(void) {
    for (const icon_map_entry_t *e = icon_map; e->name; e++) {
        fprintf(stderr, "  %s\n", e->name);
    }
}

// ========== 颜色解析 ==========

static inline int parse_color(const char *str, uint32_t *out) {
    if (!str || !out) return -1;
    char *end;
    unsigned long v = strtoul(str, &end, 0);
    if (*end != '\0') return -1;
    *out = (uint32_t)v;
    return 0;
}

// ========== JSON 输出辅助 ==========

static inline void print_ok(void) {
    if (g_json_output)
        printf("{\"status\":\"ok\"}\n");
    else
        printf("ok\n");
}

static inline void print_error(const char *msg) {
    if (g_json_output)
        printf("{\"status\":\"error\",\"message\":\"%s\"}\n", msg);
    else
        printf("error: %s\n", msg);
}

// ========== 模块 handler 原型 ==========

int cmd_ui(int argc, char **argv, ipc_client_t *client);
int cmd_prts(int argc, char **argv, ipc_client_t *client);
int cmd_settings(int argc, char **argv, ipc_client_t *client);
int cmd_mediaplayer(int argc, char **argv, ipc_client_t *client);
int cmd_overlay(int argc, char **argv, ipc_client_t *client);
int cmd_app(int argc, char **argv, ipc_client_t *client);
