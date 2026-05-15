#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmd.h"

static void print_usage(void) {
    fprintf(stderr, "epassctl overlay <操作> [参数...]\n\n");
    fprintf(stderr, "可用操作:\n");
    fprintf(stderr, "  transition <duration_ms> <type> <image_path_or_-> <color>\n");
    fprintf(stderr, "      纯过渡排期；无图片时用 \"-\" 占位\n");
    fprintf(stderr, "  transition_video <video_path> <duration_ms> <type> <image_path_or_-> <color>\n");
    fprintf(stderr, "      带视频更换的过渡排期；无图片时用 \"-\"\n\n");
    fprintf(stderr, "type 可选值:\n");
    enum_print_options(transition_map);
}

static const char *image_or_null(const char *s) {
    if (!s || strcmp(s, "-") == 0)
        return NULL;
    return s;
}

static int cmd_overlay_transition(int argc, char **argv, ipc_client_t *client) {
    if (argc < 5) {
        fprintf(stderr, "用法: epassctl overlay transition <duration_ms> <type> <image_path_or_-> <color>\n");
        return 1;
    }
    int duration = atoi(argv[1]);
    int type_val;
    if (enum_from_str(transition_map, argv[2], &type_val) < 0) {
        fprintf(stderr, "未知的过渡类型: %s\n可选值:\n", argv[2]);
        enum_print_options(transition_map);
        return 1;
    }
    const char *image = image_or_null(argv[3]);
    uint32_t color;
    if (parse_color(argv[4], &color) < 0) {
        print_error("无效的颜色值");
        return 1;
    }
    if (ipc_client_overlay_schedule_transition(client, duration, (transition_type_t)type_val, image, color) < 0) {
        print_error("overlay transition 请求失败");
        return 1;
    }
    print_ok();
    return 0;
}

static int cmd_overlay_transition_video(int argc, char **argv, ipc_client_t *client) {
    if (argc < 6) {
        fprintf(stderr, "用法: epassctl overlay transition_video <video_path> <duration_ms> <type> <image_path_or_-> <color>\n");
        return 1;
    }
    const char *video_path = argv[1];
    int duration = atoi(argv[2]);
    int type_val;
    if (enum_from_str(transition_map, argv[3], &type_val) < 0) {
        fprintf(stderr, "未知的过渡类型: %s\n可选值:\n", argv[3]);
        enum_print_options(transition_map);
        return 1;
    }
    const char *image = image_or_null(argv[4]);
    uint32_t color;
    if (parse_color(argv[5], &color) < 0) {
        print_error("无效的颜色值");
        return 1;
    }
    if (ipc_client_overlay_schedule_transition_video(client, video_path, duration, (transition_type_t)type_val, image, color) < 0) {
        print_error("overlay transition_video 请求失败");
        return 1;
    }
    print_ok();
    return 0;
}

int cmd_overlay(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2 || strcmp(argv[1], "help") == 0) {
        print_usage();
        return (argc < 2) ? 1 : 0;
    }
    const char *action = argv[1];
    int sub_argc = argc - 1;
    char **sub_argv = argv + 1;

    if (strcmp(action, "transition") == 0)
        return cmd_overlay_transition(sub_argc, sub_argv, client);
    if (strcmp(action, "transition_video") == 0)
        return cmd_overlay_transition_video(sub_argc, sub_argv, client);

    fprintf(stderr, "未知操作: overlay %s\n", action);
    print_usage();
    return 1;
}
