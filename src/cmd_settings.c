#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmd.h"

static void print_usage(void) {
    fprintf(stderr, "epassctl settings <操作> [参数...]\n\n");
    fprintf(stderr, "可用操作:\n");
    fprintf(stderr, "  get\n");
    fprintf(stderr, "      获取当前设备设置\n");
    fprintf(stderr, "  set <key> <value> [<key> <value> ...]\n");
    fprintf(stderr, "      修改设备设置 (read-modify-write)\n\n");
    fprintf(stderr, "set 支持的 key:\n");
    fprintf(stderr, "  brightness <int>\n");
    fprintf(stderr, "  interval   <name>   取值: ");
    for (const enum_map_t *e = sw_interval_map; e->name; e++)
        fprintf(stderr, "%s ", e->name);
    fprintf(stderr, "\n");
    fprintf(stderr, "  mode       <name>   取值: ");
    for (const enum_map_t *e = sw_mode_map; e->name; e++)
        fprintf(stderr, "%s ", e->name);
    fprintf(stderr, "\n");
    fprintf(stderr, "  usb        <name>   取值: ");
    for (const enum_map_t *e = usb_mode_map; e->name; e++)
        fprintf(stderr, "%s ", e->name);
    fprintf(stderr, "\n");
    fprintf(stderr, "  lowbat     <0|1>\n");
    fprintf(stderr, "  no_intro   <0|1>\n");
    fprintf(stderr, "  no_overlay <0|1>\n");
}

static void print_settings(const ipc_settings_data_t *s) {
    const char *interval_str = enum_to_str(sw_interval_map, (int)s->switch_interval);
    const char *mode_str = enum_to_str(sw_mode_map, (int)s->switch_mode);
    const char *usb_str = enum_to_str(usb_mode_map, (int)s->usb_mode);

    if (g_json_output) {
        printf("{\"brightness\":%d,"
               "\"switch_interval\":\"%s\",\"switch_interval_id\":%d,"
               "\"switch_mode\":\"%s\",\"switch_mode_id\":%d,"
               "\"usb_mode\":\"%s\",\"usb_mode_id\":%d,"
               "\"lowbat_trip\":%u,\"no_intro_block\":%u,\"no_overlay_block\":%u}\n",
               s->brightness,
               interval_str, (int)s->switch_interval,
               mode_str, (int)s->switch_mode,
               usb_str, (int)s->usb_mode,
               s->ctrl_word.lowbat_trip,
               s->ctrl_word.no_intro_block,
               s->ctrl_word.no_overlay_block);
    } else {
        printf("brightness: %d\n", s->brightness);
        printf("switch_interval: %s (%d)\n", interval_str, (int)s->switch_interval);
        printf("switch_mode: %s (%d)\n", mode_str, (int)s->switch_mode);
        printf("usb_mode: %s (%d)\n", usb_str, (int)s->usb_mode);
        printf("lowbat_trip: %u\n", s->ctrl_word.lowbat_trip);
        printf("no_intro_block: %u\n", s->ctrl_word.no_intro_block);
        printf("no_overlay_block: %u\n", s->ctrl_word.no_overlay_block);
    }
}

static int cmd_settings_get(ipc_client_t *client) {
    ipc_settings_data_t settings;
    if (ipc_client_settings_get(client, &settings) < 0) {
        print_error("settings get 请求失败");
        return 1;
    }
    print_settings(&settings);
    return 0;
}

static int apply_settings_kv(ipc_settings_data_t *settings, const char *key, const char *val) {
    int iv;
    if (strcasecmp(key, "brightness") == 0) {
        settings->brightness = atoi(val);
        return 0;
    }
    if (strcasecmp(key, "interval") == 0) {
        if (enum_from_str(sw_interval_map, val, &iv) < 0) {
            fprintf(stderr, "无效的 interval: %s\n可选值:\n", val);
            enum_print_options(sw_interval_map);
            return -1;
        }
        settings->switch_interval = (sw_interval_t)iv;
        return 0;
    }
    if (strcasecmp(key, "mode") == 0) {
        if (enum_from_str(sw_mode_map, val, &iv) < 0) {
            fprintf(stderr, "无效的 mode: %s\n可选值:\n", val);
            enum_print_options(sw_mode_map);
            return -1;
        }
        settings->switch_mode = (sw_mode_t)iv;
        return 0;
    }
    if (strcasecmp(key, "usb") == 0) {
        if (enum_from_str(usb_mode_map, val, &iv) < 0) {
            fprintf(stderr, "无效的 usb: %s\n可选值:\n", val);
            enum_print_options(usb_mode_map);
            return -1;
        }
        settings->usb_mode = (usb_mode_t)iv;
        return 0;
    }
    if (strcasecmp(key, "lowbat") == 0) {
        settings->ctrl_word.lowbat_trip = (unsigned int)atoi(val);
        return 0;
    }
    if (strcasecmp(key, "no_intro") == 0) {
        settings->ctrl_word.no_intro_block = (unsigned int)atoi(val);
        return 0;
    }
    if (strcasecmp(key, "no_overlay") == 0) {
        settings->ctrl_word.no_overlay_block = (unsigned int)atoi(val);
        return 0;
    }
    fprintf(stderr, "未知的 key: %s\n", key);
    return -1;
}

static int cmd_settings_set(int argc, char **argv, ipc_client_t *client) {
    ipc_settings_data_t settings;
    if (ipc_client_settings_get(client, &settings) < 0) {
        print_error("settings get 请求失败 (read-modify-write 读取阶段)");
        return 1;
    }

    if (argc < 3 || ((argc - 1) % 2) != 0) {
        fprintf(stderr, "用法: epassctl settings set <key> <value> [<key> <value> ...]\n");
        print_usage();
        return 1;
    }

    for (int i = 1; i + 1 < argc; i += 2) {
        if (apply_settings_kv(&settings, argv[i], argv[i + 1]) < 0)
            return 1;
    }

    if (ipc_client_settings_set(client, &settings) < 0) {
        print_error("settings set 请求失败");
        return 1;
    }
    print_ok();
    return 0;
}

int cmd_settings(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2 || strcmp(argv[1], "help") == 0) {
        print_usage();
        return (argc < 2) ? 1 : 0;
    }
    const char *action = argv[1];
    int sub_argc = argc - 1;
    char **sub_argv = argv + 1;

    if (strcmp(action, "get") == 0)
        return cmd_settings_get(client);
    if (strcmp(action, "set") == 0)
        return cmd_settings_set(sub_argc, sub_argv, client);

    fprintf(stderr, "未知操作: settings %s\n", action);
    print_usage();
    return 1;
}
