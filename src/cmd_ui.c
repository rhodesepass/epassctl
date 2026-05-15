#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include "cmd.h"

static void print_usage(void) {
    fprintf(stderr, "epassctl ui <操作> [参数...]\n\n");
    fprintf(stderr, "可用操作:\n");
    fprintf(stderr, "  warning <title> <desc> <icon_name> <color>\n");
    fprintf(stderr, "      弹出警告/通知窗口；color 为 0xAARRGGBB 或十进制\n");
    fprintf(stderr, "      icon_name 可选值:\n");
    icon_print_options();
    fprintf(stderr, "  get_screen\n");
    fprintf(stderr, "      获取当前界面\n");
    fprintf(stderr, "  set_screen <screen_name>\n");
    fprintf(stderr, "      切换到指定界面\n");
    fprintf(stderr, "  force_dispimg <path>\n");
    fprintf(stderr, "      强制显示指定图片\n\n");
    fprintf(stderr, "screen_name 可选值:\n");
    enum_print_options(screen_map);
}

static int cmd_ui_warning(int argc, char **argv, ipc_client_t *client) {
    if (argc < 5) {
        fprintf(stderr, "用法: epassctl ui warning <title> <desc> <icon_name> <color>\n");
        fprintf(stderr, "icon_name:\n");
        icon_print_options();
        return 1;
    }
    const char *title = argv[1];
    const char *desc = argv[2];
    const char *icon_name = argv[3];
    const char *icon = icon_from_name(icon_name);
    if (!icon) {
        fprintf(stderr, "未知的图标名称: %s\n可选值:\n", icon_name);
        icon_print_options();
        return 1;
    }
    uint32_t color;
    if (parse_color(argv[4], &color) < 0) {
        print_error("无效的颜色值");
        return 1;
    }
    if (ipc_client_ui_warning(client, title, desc, icon, color) < 0) {
        print_error("ui warning 请求失败");
        return 1;
    }
    print_ok();
    return 0;
}

static int cmd_ui_get_screen(ipc_client_t *client) {
    curr_screen_t screen;
    if (ipc_client_ui_get_current_screen(client, &screen) < 0) {
        print_error("ui get_screen 请求失败");
        return 1;
    }
    const char *name = enum_to_str(screen_map, (int)screen);
    if (g_json_output)
        printf("{\"screen\":\"%s\",\"screen_id\":%d}\n", name, (int)screen);
    else
        printf("screen: %s (%d)\n", name, (int)screen);
    return 0;
}

static int cmd_ui_set_screen(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2) {
        fprintf(stderr, "用法: epassctl ui set_screen <screen_name>\n");
        fprintf(stderr, "screen_name 可选值:\n");
        enum_print_options(screen_map);
        return 1;
    }
    int val;
    if (enum_from_str(screen_map, argv[1], &val) < 0) {
        fprintf(stderr, "未知的 screen_name: %s\n", argv[1]);
        fprintf(stderr, "可选值:\n");
        enum_print_options(screen_map);
        return 1;
    }
    if (ipc_client_ui_set_current_screen(client, (curr_screen_t)val) < 0) {
        print_error("ui set_screen 请求失败");
        return 1;
    }
    print_ok();
    return 0;
}

static int cmd_ui_force_dispimg(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2) {
        fprintf(stderr, "用法: epassctl ui force_dispimg <path>\n");
        return 1;
    }
    if (ipc_client_ui_force_dispimg(client, argv[1]) < 0) {
        print_error("ui force_dispimg 请求失败");
        return 1;
    }
    print_ok();
    return 0;
}

int cmd_ui(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2 || strcmp(argv[1], "help") == 0) {
        print_usage();
        return (argc < 2) ? 1 : 0;
    }
    const char *action = argv[1];
    int sub_argc = argc - 1;
    char **sub_argv = argv + 1;

    if (strcmp(action, "warning") == 0)
        return cmd_ui_warning(sub_argc, sub_argv, client);
    if (strcmp(action, "get_screen") == 0)
        return cmd_ui_get_screen(client);
    if (strcmp(action, "set_screen") == 0)
        return cmd_ui_set_screen(sub_argc, sub_argv, client);
    if (strcmp(action, "force_dispimg") == 0)
        return cmd_ui_force_dispimg(sub_argc, sub_argv, client);

    fprintf(stderr, "未知操作: ui %s\n", action);
    print_usage();
    return 1;
}
