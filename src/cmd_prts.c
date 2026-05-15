#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmd.h"

static void print_usage(void) {
    fprintf(stderr, "epassctl prts <操作> [参数...]\n\n");
    fprintf(stderr, "可用操作:\n");
    fprintf(stderr, "  status\n");
    fprintf(stderr, "      获取 PRTS 当前状态\n");
    fprintf(stderr, "  set_operator <index>\n");
    fprintf(stderr, "      切换到指定干员\n");
    fprintf(stderr, "  get_operator_info <index>\n");
    fprintf(stderr, "      获取指定干员信息\n");
    fprintf(stderr, "  block_auto_switch <true|false>\n");
    fprintf(stderr, "      闭锁/解锁 PRTS 自动切换\n");
}

static int cmd_prts_status(ipc_client_t *client) {
    ipc_resp_prts_status_data_t st;
    if (ipc_client_prts_get_status(client, &st) < 0) {
        print_error("prts status 请求失败");
        return 1;
    }
    const char *state_str = enum_to_str(prts_state_map, (int)st.state);
    if (g_json_output) {
        printf("{\"state\":\"%s\",\"state_id\":%d,"
               "\"operator_count\":%d,\"operator_index\":%d}\n",
               state_str, (int)st.state, st.operator_count, st.operator_index);
    } else {
        printf("state: %s (%d)\n", state_str, (int)st.state);
        printf("operator_count: %d\n", st.operator_count);
        printf("operator_index: %d\n", st.operator_index);
    }
    return 0;
}

static int cmd_prts_set_operator(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2) {
        fprintf(stderr, "用法: epassctl prts set_operator <index>\n");
        return 1;
    }
    int idx = atoi(argv[1]);
    if (ipc_client_prts_set_operator(client, idx) < 0) {
        print_error("prts set_operator 请求失败");
        return 1;
    }
    print_ok();
    return 0;
}

static int cmd_prts_get_operator_info(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2) {
        fprintf(stderr, "用法: epassctl prts get_operator_info <index>\n");
        return 1;
    }
    int idx = atoi(argv[1]);
    ipc_prts_operator_info_data_t info;
    if (ipc_client_prts_get_operator_info(client, idx, &info) < 0) {
        print_error("prts get_operator_info 请求失败");
        return 1;
    }

    char uuid_str[37];
    snprintf(uuid_str, sizeof(uuid_str),
             "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             info.uuid.data[0], info.uuid.data[1], info.uuid.data[2], info.uuid.data[3],
             info.uuid.data[4], info.uuid.data[5], info.uuid.data[6], info.uuid.data[7],
             info.uuid.data[8], info.uuid.data[9], info.uuid.data[10], info.uuid.data[11],
             info.uuid.data[12], info.uuid.data[13], info.uuid.data[14], info.uuid.data[15]);

    const char *source_str = (info.source == PRTS_SOURCE_NAND) ? "nand" : "sd";

    if (g_json_output) {
        printf("{\"operator_index\":%d,\"operator_name\":\"%s\","
               "\"uuid\":\"%s\",\"description\":\"%s\","
               "\"icon_path\":\"%s\",\"source\":\"%s\"}\n",
               info.operator_index, info.operator_name,
               uuid_str, info.description,
               info.icon_path, source_str);
    } else {
        printf("operator_index: %d\n", info.operator_index);
        printf("operator_name: %s\n", info.operator_name);
        printf("uuid: %s\n", uuid_str);
        printf("description: %s\n", info.description);
        printf("icon_path: %s\n", info.icon_path);
        printf("source: %s\n", source_str);
    }
    return 0;
}

static int cmd_prts_block_auto_switch(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2) {
        fprintf(stderr, "用法: epassctl prts block_auto_switch <true|false>\n");
        return 1;
    }
    bool blocked;
    if (strcasecmp(argv[1], "true") == 0 || strcmp(argv[1], "1") == 0)
        blocked = true;
    else if (strcasecmp(argv[1], "false") == 0 || strcmp(argv[1], "0") == 0)
        blocked = false;
    else {
        fprintf(stderr, "无效参数: %s (应为 true/false/1/0)\n", argv[1]);
        return 1;
    }
    if (ipc_client_prts_set_blocked_auto_switch(client, blocked) < 0) {
        print_error("prts block_auto_switch 请求失败");
        return 1;
    }
    print_ok();
    return 0;
}

int cmd_prts(int argc, char **argv, ipc_client_t *client) {
    if (argc < 2 || strcmp(argv[1], "help") == 0) {
        print_usage();
        return (argc < 2) ? 1 : 0;
    }
    const char *action = argv[1];
    int sub_argc = argc - 1;
    char **sub_argv = argv + 1;

    if (strcmp(action, "status") == 0)
        return cmd_prts_status(client);
    if (strcmp(action, "set_operator") == 0)
        return cmd_prts_set_operator(sub_argc, sub_argv, client);
    if (strcmp(action, "get_operator_info") == 0)
        return cmd_prts_get_operator_info(sub_argc, sub_argv, client);
    if (strcmp(action, "block_auto_switch") == 0)
        return cmd_prts_block_auto_switch(sub_argc, sub_argv, client);

    fprintf(stderr, "未知操作: prts %s\n", action);
    print_usage();
    return 1;
}
