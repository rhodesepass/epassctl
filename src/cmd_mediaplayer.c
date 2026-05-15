#include <stdio.h>
#include <string.h>
#include "cmd.h"

static void print_usage(void) {
    fprintf(stderr, "epassctl mediaplayer <操作> [参数...]\n\n");
    fprintf(stderr, "可用操作:\n");
    fprintf(stderr, "  get_video\n");
    fprintf(stderr, "      获取当前播放的视频路径\n");
    fprintf(stderr, "  set_video <path>\n");
    fprintf(stderr, "      设置并播放指定视频\n");
}

static int cmd_mediaplayer_get_video(ipc_client_t *client) {
    char path[128];
    if (ipc_client_mediaplayer_get_video_path(client, path, sizeof(path)) < 0) {
        print_error("mediaplayer get_video 请求失败");
        return 1;
    }
    if (g_json_output)
        printf("{\"video_path\":\"%s\"}\n", path);
    else
        printf("video_path: %s\n", path);
    return 0;
}

static int cmd_mediaplayer_set_video(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2) {
        fprintf(stderr, "用法: epassctl mediaplayer set_video <path>\n");
        return 1;
    }
    if (ipc_client_mediaplayer_set_video_path(client, argv[1]) < 0) {
        print_error("mediaplayer set_video 请求失败");
        return 1;
    }
    print_ok();
    return 0;
}

int cmd_mediaplayer(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2 || strcmp(argv[1], "help") == 0) {
        print_usage();
        return (argc < 2) ? 1 : 0;
    }
    const char *action = argv[1];
    int sub_argc = argc - 1;
    char **sub_argv = argv + 1;

    if (strcmp(action, "get_video") == 0)
        return cmd_mediaplayer_get_video(client);
    if (strcmp(action, "set_video") == 0)
        return cmd_mediaplayer_set_video(sub_argc, sub_argv, client);

    fprintf(stderr, "未知操作: mediaplayer %s\n", action);
    print_usage();
    return 1;
}
