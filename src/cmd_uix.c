#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmd.h"

static void print_usage(void) {
    fprintf(stderr, "epassctl uix <操作> [参数...]\n\n");
    fprintf(stderr, "外部交互会话（轮询式）。START 立即返回 session_id，\n");
    fprintf(stderr, "结果需用 poll <session_id> 轮询。同时只允许一个会话。\n\n");
    fprintf(stderr, "可用操作:\n");
    fprintf(stderr, "  confirm <title> <desc> [timeout_ms]\n");
    fprintf(stderr, "      弹出是/否确认框，返回 session_id\n");
    fprintf(stderr, "  usb_select <功能列表> [timeout_ms]\n");
    fprintf(stderr, "      弹出 USB 功能选择框，返回 session_id\n");
    fprintf(stderr, "      功能列表为逗号分隔，可选:\n");
    fprintf(stderr, "        mtp epass fido charge\n");
    fprintf(stderr, "  poll <session_id>\n");
    fprintf(stderr, "      轮询会话结果 (state / choice)\n");
    fprintf(stderr, "  cancel <session_id>\n");
    fprintf(stderr, "      撤回会话\n\n");
    fprintf(stderr, "timeout_ms 省略或 0 表示不超时（发起方自管）。\n");
}

static int parse_u32(const char *s, uint32_t *out) {
    char *end;
    unsigned long v = strtoul(s, &end, 0);
    if (*end != '\0') return -1;
    *out = (uint32_t)v;
    return 0;
}

// 逗号分隔的功能名 -> 位掩码
static int parse_func_mask(const char *s, uint32_t *out) {
    uint32_t mask = 0;
    char buf[128];
    snprintf(buf, sizeof(buf), "%s", s);
    for (char *tok = strtok(buf, ","); tok; tok = strtok(NULL, ",")) {
        int bit;
        if (enum_from_str(uix_usb_func_map, tok, &bit) < 0) {
            fprintf(stderr, "未知 USB 功能: %s\n可选:\n", tok);
            enum_print_options(uix_usb_func_map);
            return -1;
        }
        mask |= (uint32_t)bit;
    }
    *out = mask;
    return 0;
}

static void print_session_id(uint32_t id) {
    if (g_json_output)
        printf("{\"status\":\"ok\",\"session_id\":%u}\n", id);
    else
        printf("session_id: %u\n", id);
}

static int cmd_uix_confirm(int argc, char **argv, ipc_client_t *client) {
    if (argc < 3) {
        fprintf(stderr, "用法: epassctl uix confirm <title> <desc> [timeout_ms]\n");
        return 1;
    }
    uint32_t timeout_ms = 0;
    if (argc >= 4 && parse_u32(argv[3], &timeout_ms) < 0) {
        fprintf(stderr, "无效的 timeout_ms: %s\n", argv[3]);
        return 1;
    }
    uint32_t session_id;
    if (ipc_client_uix_confirm_start(client, argv[1], argv[2], timeout_ms, &session_id) < 0) {
        print_error("uix confirm 请求失败（可能已有会话在进行）");
        return 1;
    }
    print_session_id(session_id);
    return 0;
}

static int cmd_uix_usb_select(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2) {
        fprintf(stderr, "用法: epassctl uix usb_select <功能列表> [timeout_ms]\n");
        fprintf(stderr, "功能列表为逗号分隔，可选:\n");
        enum_print_options(uix_usb_func_map);
        return 1;
    }
    uint32_t func_mask;
    if (parse_func_mask(argv[1], &func_mask) < 0)
        return 1;
    uint32_t timeout_ms = 0;
    if (argc >= 3 && parse_u32(argv[2], &timeout_ms) < 0) {
        fprintf(stderr, "无效的 timeout_ms: %s\n", argv[2]);
        return 1;
    }
    uint32_t session_id;
    if (ipc_client_uix_usb_select_start(client, func_mask, timeout_ms, &session_id) < 0) {
        print_error("uix usb_select 请求失败（可能已有会话在进行）");
        return 1;
    }
    print_session_id(session_id);
    return 0;
}

static int cmd_uix_poll(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2) {
        fprintf(stderr, "用法: epassctl uix poll <session_id>\n");
        return 1;
    }
    uint32_t session_id;
    if (parse_u32(argv[1], &session_id) < 0) {
        fprintf(stderr, "无效的 session_id: %s\n", argv[1]);
        return 1;
    }
    uint32_t state, choice;
    if (ipc_client_uix_session_poll(client, session_id, &state, &choice) < 0) {
        print_error("uix poll 请求失败");
        return 1;
    }
    const char *state_str = enum_to_str(uix_state_map, (int)state);
    // choice 仅在 usb_select 且 confirmed 时有效
    const char *choice_str = enum_to_str(uix_usb_choice_map, (int)choice);
    if (g_json_output) {
        printf("{\"state\":\"%s\",\"state_id\":%u,"
               "\"choice\":%u,\"choice_name\":\"%s\"}\n",
               state_str, state, choice, choice_str);
    } else {
        printf("state: %s (%u)\n", state_str, state);
        printf("choice: %s (%u)\n", choice_str, choice);
    }
    return 0;
}

static int cmd_uix_cancel(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2) {
        fprintf(stderr, "用法: epassctl uix cancel <session_id>\n");
        return 1;
    }
    uint32_t session_id;
    if (parse_u32(argv[1], &session_id) < 0) {
        fprintf(stderr, "无效的 session_id: %s\n", argv[1]);
        return 1;
    }
    if (ipc_client_uix_session_cancel(client, session_id) < 0) {
        print_error("uix cancel 请求失败");
        return 1;
    }
    print_ok();
    return 0;
}

int cmd_uix(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2 || strcmp(argv[1], "help") == 0) {
        print_usage();
        return (argc < 2) ? 1 : 0;
    }
    const char *action = argv[1];
    int sub_argc = argc - 1;
    char **sub_argv = argv + 1;

    if (strcmp(action, "confirm") == 0)
        return cmd_uix_confirm(sub_argc, sub_argv, client);
    if (strcmp(action, "usb_select") == 0)
        return cmd_uix_usb_select(sub_argc, sub_argv, client);
    if (strcmp(action, "poll") == 0)
        return cmd_uix_poll(sub_argc, sub_argv, client);
    if (strcmp(action, "cancel") == 0)
        return cmd_uix_cancel(sub_argc, sub_argv, client);

    fprintf(stderr, "未知操作: uix %s\n", action);
    print_usage();
    return 1;
}
