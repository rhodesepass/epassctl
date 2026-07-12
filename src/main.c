#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include "cmd.h"

bool g_json_output = false;

typedef struct {
    const char *name;
    int (*handler)(int argc, char **argv, ipc_client_t *client);
    const char *desc;
} module_entry_t;

static const module_entry_t modules[] = {
    {"ui",          cmd_ui,          "UI 控制 (弹窗/界面切换/显示图片)"},
    {"prts",        cmd_prts,        "PRTS 干员系统控制"},
    {"settings",    cmd_settings,    "设备设置读写"},
    {"mediaplayer", cmd_mediaplayer, "媒体播放器控制"},
    {"overlay",     cmd_overlay,     "过渡动画排期"},
    {"app",         cmd_app,         "应用全局控制 (退出/重启)"},
    {"uix",         cmd_uix,         "外部交互会话 (确认框/USB选择, 轮询式)"},
    {NULL, NULL, NULL}
};

static void print_usage(const char *prog) {
    fprintf(stderr, "用法: %s [json] <模块> <操作> [参数...]\n\n", prog);
    fprintf(stderr, "位置参数:\n");
    fprintf(stderr, "  json          可选，放在最前则输出 JSON\n");
    fprintf(stderr, "  help          显示本帮助\n\n");
    fprintf(stderr, "可用模块:\n");
    for (const module_entry_t *m = modules; m->name; m++) {
        fprintf(stderr, "  %-14s%s\n", m->name, m->desc);
    }
    fprintf(stderr, "\n使用 '%s <模块> help' 查看模块内可用操作。\n", prog);
}

int main(int argc, char *argv[]) {
    g_json_output = false;
    int i = 1;
    if (i < argc && strcmp(argv[i], "json") == 0) {
        g_json_output = true;
        i++;
    }
    if (i >= argc) {
        print_usage(argv[0]);
        return 1;
    }
    if (strcmp(argv[i], "help") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    const char *module_name = argv[i];
    int sub_argc = argc - i;
    char **sub_argv = argv + i;

    for (const module_entry_t *m = modules; m->name; m++) {
        if (strcmp(module_name, m->name) == 0) {
            bool need_help = (sub_argc < 2) ||
                             (sub_argc >= 2 && strcmp(sub_argv[1], "help") == 0);
            if (need_help)
                return m->handler(sub_argc, sub_argv, NULL);

            ipc_client_t client = {.fd = -1};
            if (ipc_client_init(&client) < 0) {
                print_error("无法连接到 epass-drm-app (IPC 初始化失败)");
                return 1;
            }

            int rc = m->handler(sub_argc, sub_argv, &client);
            ipc_client_destroy(&client);
            return rc;
        }
    }

    fprintf(stderr, "未知模块: %s\n", module_name);
    print_usage(argv[0]);
    return 1;
}
