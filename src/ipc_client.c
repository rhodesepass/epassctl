#include <fcntl.h>
#include <stdint.h>
#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "ipc_client.h"
#include "log.h"

static int connect_sock(const char *path) {
    int fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (fd < 0) {
        log_error("socket: %s", strerror(errno));
        return -1;
    }

    int flags = fcntl(fd, F_GETFD);
    if (flags >= 0) {
        fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        log_error("connect: %s", strerror(errno));
        close(fd);
        return -1;
    }
    return fd;
}

static int ipc_client_request(ipc_client_t *client, const ipc_req_t *req, ipc_resp_t *resp) {
    if (ipc_client_send(client, req) < 0) {
        return -1;
    }

    ipc_resp_t tmp_resp;
    ipc_resp_t *out = resp ? resp : &tmp_resp;
    if (ipc_client_recv(client, req->type, out) < 0) {
        return -1;
    }
    if (out->type != IPC_RESP_OK) {
        log_error("ipc response error: %d", out->type);
        return -1;
    }
    return 0;
}

int ipc_client_init(ipc_client_t *client) {
    if (!client) {
        return -1;
    }
    client->fd = connect_sock(APPS_IPC_SOCKET_PATH);
    return (client->fd < 0) ? -1 : 0;
}

int ipc_client_destroy(ipc_client_t *client) {
    if (!client || client->fd < 0) {
        return -1;
    }
    close(client->fd);
    client->fd = -1;
    return 0;
}

int ipc_client_send(ipc_client_t *client, const ipc_req_t *req) {
    if (!client || client->fd < 0 || !req) {
        return -1;
    }
    if (req->type >= IPC_REQ_MAX) {
        log_error("invalid ipc request type: %d", req->type);
        return -1;
    }

    size_t len = calculate_ipc_req_size(req->type);
    if (len > APPS_IPC_MAX_MSG) {
        log_error("ipc request too large: %zu > %d", len, APPS_IPC_MAX_MSG);
        return -1;
    }

    ssize_t w = send(client->fd, req, len, 0);
    if (w < 0 || (size_t)w != len) {
        log_error("send: %s", (w < 0) ? strerror(errno) : "short write");
        return -1;
    }
    return 0;
}

int ipc_client_recv(ipc_client_t *client, ipc_req_type_t req_type, ipc_resp_t *resp) {
    if (!client || client->fd < 0 || !resp) {
        return -1;
    }
    if (req_type >= IPC_REQ_MAX) {
        log_error("invalid ipc request type: %d", req_type);
        return -1;
    }

    size_t expected = calculate_ipc_resp_size_by_req(req_type);
    if (expected > APPS_IPC_MAX_MSG) {
        log_error("ipc response too large: %zu > %d", expected, APPS_IPC_MAX_MSG);
        return -1;
    }

    uint8_t buf[APPS_IPC_MAX_MSG];
    ssize_t r = recv(client->fd, buf, sizeof(buf), 0);
    if (r < 0) {
        log_error("recv: %s", strerror(errno));
        return -1;
    }
    if (r == 0) {
        log_error("ipc server closed");
        return -1;
    }
    if ((size_t)r != expected) {
        log_error("ipc response length mismatch: got %zd, expect %zu", r, expected);
        return -1;
    }

    memset(resp, 0, sizeof(*resp));
    memcpy(resp, buf, (size_t)r);
    return 0;
}

// ========== UI 子模块 ==========
int ipc_client_ui_warning(ipc_client_t *client, const char *title, const char *desc, const char *icon, uint32_t color) {
    if (!title || !desc || !icon) {
        return -1;
    }
    ipc_req_t req = {0};
    req.type = IPC_REQ_UI_WARNING;
    snprintf(req.ui_warning.title, sizeof(req.ui_warning.title), "%s", title);
    snprintf(req.ui_warning.desc, sizeof(req.ui_warning.desc), "%s", desc);
    snprintf(req.ui_warning.icon, sizeof(req.ui_warning.icon), "%s", icon);
    req.ui_warning.color = color;
    return ipc_client_request(client, &req, NULL);
}

int ipc_client_ui_get_current_screen(ipc_client_t *client, curr_screen_t *out_screen) {
    if (!out_screen) {
        return -1;
    }
    ipc_req_t req = {0};
    ipc_resp_t resp = {0};
    req.type = IPC_REQ_UI_GET_CURRENT_SCREEN;
    if (ipc_client_request(client, &req, &resp) < 0) {
        return -1;
    }
    *out_screen = resp.ui_current_screen.screen;
    return 0;
}

int ipc_client_ui_set_current_screen(ipc_client_t *client, curr_screen_t screen) {
    ipc_req_t req = {0};
    req.type = IPC_REQ_UI_SET_CURRENT_SCREEN;
    req.ui_set_current_screen.screen = screen;
    return ipc_client_request(client, &req, NULL);
}

int ipc_client_ui_force_dispimg(ipc_client_t *client, const char *path) {
    if (!path) {
        return -1;
    }
    ipc_req_t req = {0};
    req.type = IPC_REQ_UI_FORCE_DISPIMG;
    snprintf(req.ui_force_dispimg.path, sizeof(req.ui_force_dispimg.path), "%s", path);
    return ipc_client_request(client, &req, NULL);
}

// ========== PRTS 子模块 ==========
int ipc_client_prts_get_status(ipc_client_t *client, ipc_resp_prts_status_data_t *out) {
    if (!out) {
        return -1;
    }
    ipc_req_t req = {0};
    ipc_resp_t resp = {0};
    req.type = IPC_REQ_PRTS_GET_STATUS;
    if (ipc_client_request(client, &req, &resp) < 0) {
        return -1;
    }
    *out = resp.prts_status;
    return 0;
}

int ipc_client_prts_set_operator(ipc_client_t *client, int operator_index) {
    ipc_req_t req = {0};
    req.type = IPC_REQ_PRTS_SET_OPERATOR;
    req.prts_set_operator.operator_index = operator_index;
    return ipc_client_request(client, &req, NULL);
}

int ipc_client_prts_get_operator_info(ipc_client_t *client, int operator_index, ipc_prts_operator_info_data_t *out) {
    if (!out) {
        return -1;
    }
    ipc_req_t req = {0};
    ipc_resp_t resp = {0};
    req.type = IPC_REQ_PRTS_GET_OPERATOR_INFO;
    req.prts_get_operator.operator_index = operator_index;
    if (ipc_client_request(client, &req, &resp) < 0) {
        return -1;
    }
    *out = resp.prts_operator_info;
    return 0;
}

int ipc_client_prts_set_blocked_auto_switch(ipc_client_t *client, bool is_blocked) {
    ipc_req_t req = {0};
    req.type = IPC_REQ_PRTS_SET_BLOCKED_AUTO_SWITCH;
    req.prts_set_blocked_auto_switch.is_blocked = is_blocked;
    return ipc_client_request(client, &req, NULL);
}

// ========== Settings 子模块 ==========
int ipc_client_settings_get(ipc_client_t *client, ipc_settings_data_t *out) {
    if (!out) {
        return -1;
    }
    ipc_req_t req = {0};
    ipc_resp_t resp = {0};
    req.type = IPC_REQ_SETTINGS_GET;
    if (ipc_client_request(client, &req, &resp) < 0) {
        return -1;
    }
    *out = resp.settings;
    return 0;
}

int ipc_client_settings_set(ipc_client_t *client, const ipc_settings_data_t *data) {
    if (!data) {
        return -1;
    }
    ipc_req_t req = {0};
    req.type = IPC_REQ_SETTINGS_SET;
    req.settings = *data;
    return ipc_client_request(client, &req, NULL);
}

// ========== MediaPlayer 子模块 ==========
int ipc_client_mediaplayer_get_video_path(ipc_client_t *client, char *path, size_t max_len) {
    if (!path || max_len == 0) {
        return -1;
    }
    ipc_req_t req = {0};
    ipc_resp_t resp = {0};
    req.type = IPC_REQ_MEDIAPLAYER_GET_VIDEO_PATH;
    if (ipc_client_request(client, &req, &resp) < 0) {
        return -1;
    }
    snprintf(path, max_len, "%s", resp.mediaplayer_video_path.path);
    return 0;
}

int ipc_client_mediaplayer_set_video_path(ipc_client_t *client, const char *path) {
    if (!path) {
        return -1;
    }
    ipc_req_t req = {0};
    req.type = IPC_REQ_MEDIAPLAYER_SET_VIDEO_PATH;
    snprintf(req.mediaplayer_video_path.path, sizeof(req.mediaplayer_video_path.path), "%s", path);
    return ipc_client_request(client, &req, NULL);
}

// ========== Overlay 子模块 ==========
int ipc_client_overlay_schedule_transition(ipc_client_t *client, int duration, transition_type_t type, const char *image_path, uint32_t background_color) {
    ipc_req_t req = {0};
    req.type = IPC_REQ_OVERLAY_SCHEDULE_TRANSITION;
    req.overlay_schedule_transition.duration = duration;
    req.overlay_schedule_transition.type = type;
    if (image_path) {
        snprintf(req.overlay_schedule_transition.image_path, sizeof(req.overlay_schedule_transition.image_path), "%s", image_path);
    }
    req.overlay_schedule_transition.background_color = background_color;
    return ipc_client_request(client, &req, NULL);
}

int ipc_client_overlay_schedule_transition_video(ipc_client_t *client, const char *video_path, int duration, transition_type_t type, const char *image_path, uint32_t background_color) {
    if (!video_path) {
        return -1;
    }
    ipc_req_t req = {0};
    req.type = IPC_REQ_OVERLAY_SCHEDULE_TRANSITION_VIDEO;
    snprintf(req.overlay_schedule_transition_video.video_path, sizeof(req.overlay_schedule_transition_video.video_path), "%s", video_path);
    req.overlay_schedule_transition_video.duration = duration;
    req.overlay_schedule_transition_video.type = type;
    if (image_path) {
        snprintf(req.overlay_schedule_transition_video.image_path, sizeof(req.overlay_schedule_transition_video.image_path), "%s", image_path);
    }
    req.overlay_schedule_transition_video.background_color = background_color;
    return ipc_client_request(client, &req, NULL);
}

// ========== 全局 ==========
int ipc_client_app_exit(ipc_client_t *client, int exit_code) {
    ipc_req_t req = {0};
    req.type = IPC_REQ_APP_EXIT;
    req.app_exit.exit_code = exit_code;
    return ipc_client_request(client, &req, NULL);
}