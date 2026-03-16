#pragma once
#include "ipc_common.h"

#define APPS_IPC_SOCKET_PATH "/tmp/epass_drm_app.sock"
#define APPS_IPC_MAX_MSG 512

typedef struct {
    int fd;
} ipc_client_t;

int ipc_client_init(ipc_client_t *client);
int ipc_client_send(ipc_client_t *client, const ipc_req_t *req);
int ipc_client_recv(ipc_client_t *client, ipc_req_type_t req_type, ipc_resp_t *resp);
int ipc_client_destroy(ipc_client_t *client);

// ========== UI 子模块 ==========
int ipc_client_ui_warning(ipc_client_t *client, const char *title, const char *desc, const char *icon, uint32_t color);
int ipc_client_ui_get_current_screen(ipc_client_t *client, curr_screen_t *out_screen);
int ipc_client_ui_set_current_screen(ipc_client_t *client, curr_screen_t screen);
int ipc_client_ui_force_dispimg(ipc_client_t *client, const char *path);

// ========== PRTS 子模块 ==========
int ipc_client_prts_get_status(ipc_client_t *client, ipc_resp_prts_status_data_t *out);
int ipc_client_prts_set_operator(ipc_client_t *client, int operator_index);
int ipc_client_prts_get_operator_info(ipc_client_t *client, int operator_index, ipc_prts_operator_info_data_t *out);
int ipc_client_prts_set_blocked_auto_switch(ipc_client_t *client, bool is_blocked);

// ========== Settings 子模块 ==========
int ipc_client_settings_get(ipc_client_t *client, ipc_settings_data_t *out);
int ipc_client_settings_set(ipc_client_t *client, const ipc_settings_data_t *data);

// ========== MediaPlayer 子模块 ==========
int ipc_client_mediaplayer_get_video_path(ipc_client_t *client, char *path, size_t max_len);
int ipc_client_mediaplayer_set_video_path(ipc_client_t *client, const char *path);

// ========== Overlay 子模块 ==========
int ipc_client_overlay_schedule_transition(ipc_client_t *client, int duration, transition_type_t type, const char *image_path, uint32_t background_color);
int ipc_client_overlay_schedule_transition_video(ipc_client_t *client, const char *video_path, int duration, transition_type_t type, const char *image_path, uint32_t background_color);

// ========== 全局 ==========
int ipc_client_app_exit(ipc_client_t *client, int exit_code);
