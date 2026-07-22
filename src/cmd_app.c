#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmd.h"

static void print_usage(void) {
    fprintf(stderr, "epassctl app <操作> [参数...]\n\n");
    fprintf(stderr, "可用操作:\n");
    fprintf(stderr, "  exit <code>\n");
    fprintf(stderr, "      使主应用以指定退出码退出\n");
    fprintf(stderr, "  reload_list\n");
    fprintf(stderr, "      重新从磁盘扫描并刷新应用列表\n\n");
    fprintf(stderr, "code 可选值 (数字或名称):\n");
    enum_print_options(exitcode_map);
}

static int cmd_app_reload_list(ipc_client_t *client) {
    if (ipc_client_app_reload_list(client) < 0) {
        print_error("app reload_list 请求失败");
        return 1;
    }
    print_ok();
    return 0;
}

static int cmd_app_exit(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2) {
        fprintf(stderr, "用法: epassctl app exit <code>\n");
        fprintf(stderr, "code 可选值:\n");
        enum_print_options(exitcode_map);
        return 1;
    }

    int code;
    if (enum_from_str(exitcode_map, argv[1], &code) < 0) {
        char *end;
        long v = strtol(argv[1], &end, 10);
        if (*end != '\0') {
            fprintf(stderr, "无效的退出码: %s\n可选值:\n", argv[1]);
            enum_print_options(exitcode_map);
            return 1;
        }
        code = (int)v;
    }

    if (ipc_client_app_exit(client, code) < 0) {
        print_error("app exit 请求失败");
        return 1;
    }
    print_ok();
    return 0;
}

int cmd_app(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2 || strcmp(argv[1], "help") == 0) {
        print_usage();
        return (argc < 2) ? 1 : 0;
    }
    const char *action = argv[1];
    int sub_argc = argc - 1;
    char **sub_argv = argv + 1;

    if (strcmp(action, "exit") == 0)
        return cmd_app_exit(sub_argc, sub_argv, client);
    if (strcmp(action, "reload_list") == 0)
        return cmd_app_reload_list(client);

    fprintf(stderr, "未知操作: app %s\n", action);
    print_usage();
    return 1;
}
