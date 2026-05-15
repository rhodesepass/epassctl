# epassctl

通过 Unix 域套接字 IPC 控制设备上的 **epass-drm-app**（电子通行证主程序）的命令行工具。

- **套接字路径**：`/tmp/epass_drm_app.sock`（与 [`src/ipc_client.h`](src/ipc_client.h) 中定义一致）
- **前提**：主程序已启动并监听该套接字；否则 `epassctl` 会报错并返回非零退出码。

## 构建

```bash
mkdir -p build && cd build
cmake ..
make
```

可执行文件为 `build/epassctl`（若已 `cmake --install`，则按安装路径使用）。

## 命令行总览

所有参数均为**位置参数**（不使用 `getopt` 风格的长选项）。

```text
epassctl [json] <模块> <操作> [参数...]
```

| 位置 | 含义 |
|------|------|
| `json` | **可选**。若作为**第一个**参数出现，则查询类命令输出 JSON，成功/失败信息也使用 JSON（见下文「输出格式」）。 |
| `<模块>` | `ui`、`prts`、`settings`、`mediaplayer`、`overlay`、`app` 之一。 |
| `<操作>` | 各模块下的子命令。 |
| 其余 | 各子命令要求的参数，按顺序书写。 |

**帮助**：

- 总帮助：`epassctl help` 或不带参数运行 `epassctl`
- 模块帮助：`epassctl <模块> help`（例如 `epassctl ui help`；**不连接 IPC**，可在无设备时查看用法）

**含空格的参数**：在 shell 中用引号包裹，例如：`epassctl ui warning "标题" "描述" cat 0xFF004400`

## 输出格式

- **默认（未加 `json`）**
  - 写操作成功：打印一行 `ok`
  - 查询类：多行 `key: value` 文本
  - 错误：向 stderr 打印 `error: ...`，退出码非 0
- **首参为 `json`**
  - 写操作成功：`{"status":"ok"}`
  - 错误：`{"status":"error","message":"..."}`
  - 各查询命令输出一行 JSON（字段名见各节说明）

## 模块：`ui`

| 操作 | 用法 |
|------|------|
| `warning` | `epassctl ui warning <title> <desc> <icon_name> <color>` |
| `get_screen` | `epassctl ui get_screen` |
| `set_screen` | `epassctl ui set_screen <screen_name>` |
| `force_dispimg` | `epassctl ui force_dispimg <path>` |

### `warning`

弹出 UI 警告/通知。`icon_name` 与 `color` **均为必填**。

- **`color`**：`0xAARRGGBB` 形式（十六进制，可带 `0x`）或十进制整数。
- **`icon_name`**：与主程序图标资源对应的名字（小写、下划线），大小写不敏感。可用名称包括：

`heart_crack`、`dumpster_fire`、`gear`、`user`、`mobile_screen`、`file`、`battery_full`、`battery_three_quarters`、`battery_half`、`battery_quarter`、`battery_empty`、`triangle_exclamation`、`bolt`、`image_portrait`、`box_archive`、`images`、`sun`、`cat`、`location_crosshairs`、`xmark`、`check`、`question`、`shield`、`car_burst`、`border_none`、`thumbs_down`、`thumbs_up`、`fire_extinguisher`、`square_arrow_up_right`、`heart`、`face_grin_beam_sweat`、`face_smile_wink`、`face_kiss_beam`、`poo`、`volcano`、`code_pull_request`、`face_flushed`、`face_sad_cry`、`sd_card`

示例：

```bash
epassctl ui warning "注意" "电量偏低" battery_quarter 0xFFFF6600
epassctl json ui warning "提示" "操作完成" check 4278255360
```

### `get_screen`

查询当前界面。默认输出形如：`screen: mainmenu (0)`。`json` 模式下：`{"screen":"...","screen_id":N}`。

### `set_screen`

`screen_name` 取值（名称与内部枚举 ID 对应关系）：

| 名称 | ID |
|------|-----|
| `mainmenu` | 0 |
| `oplist` | 1 |
| `sysinfo` | 2 |
| `spinner` | 3 |
| `displayimg` | 4 |
| `filemanager` | 5 |
| `settings` | 6 |
| `warning` | 7 |
| `confirm` | 8 |
| `applist` | 9 |

示例：`epassctl ui set_screen sysinfo`

### `force_dispimg`

将扩列图路径设为指定文件（不校验路径是否存在），通常与切到 `displayimg` 界面配合使用。

示例：`epassctl ui force_dispimg /mnt/sdcard/picture.png`

## 模块：`prts`

| 操作 | 用法 |
|------|------|
| `status` | `epassctl prts status` |
| `set_operator` | `epassctl prts set_operator <index>` |
| `get_operator_info` | `epassctl prts get_operator_info <index>` |
| `block_auto_switch` | `epassctl prts block_auto_switch <true 或 false 或 1 或 0>` |

- **`status`**：返回状态机状态、干员数量、当前干员索引等。`json` 下含 `state`、`state_id`、`operator_count`、`operator_index`。
- **`block_auto_switch`**：`true` / `false`（大小写不敏感）或 `1` / `0`，用于闭锁/解锁 PRTS 按时间自动切换干员（与主程序内设置行为一致）。

## 模块：`settings`

| 操作 | 用法 |
|------|------|
| `get` | `epassctl settings get` |
| `set` | `epassctl settings set <key> <value> [<key> <value> ...]` |

`set` 为 **read-modify-write**：先从设备读取完整设置，再仅覆盖你给出的字段，最后写回。

支持的 `key`（大小写不敏感）：

| key | value 说明 |
|-----|------------|
| `brightness` | 整数亮度 |
| `interval` | `1min`、`3min`、`5min`、`10min`、`30min` |
| `mode` | `sequence`、`random`、`manual` |
| `usb` | `mtp`、`serial`、`rndis`、`none` |
| `lowbat` | `0` 或 `1` |
| `no_intro` | `0` 或 `1` |
| `no_overlay` | `0` 或 `1` |

示例：

```bash
epassctl settings get
epassctl settings set brightness 8 mode random
```

## 模块：`mediaplayer`

| 操作 | 用法 |
|------|------|
| `get_video` | `epassctl mediaplayer get_video` |
| `set_video` | `epassctl mediaplayer set_video <path>` |

`get_video` 在 `json` 模式下输出：`{"video_path":"..."}`。

## 模块：`overlay`

过渡动画排期；时长单位为**毫秒**。

| 操作 | 用法 |
|------|------|
| `transition` | `epassctl overlay transition <duration_ms> <type> <image_path_or_-> <color>` |
| `transition_video` | `epassctl overlay transition_video <video_path> <duration_ms> <type> <image_path_or_-> <color>` |

- **`type`**：`fade`、`move`、`swipe`、`none`（名称不区分大小写）。
- **无过渡图**：图片参数位置写 **`-`**（单独一个减号），表示不向 IPC 传图片路径。

示例：

```bash
epassctl overlay transition 300000 fade - 0xFF000000
epassctl overlay transition 500000 swipe /tmp/ov.jpg 0xFFFF0000
epassctl overlay transition_video /mnt/video.mp4 300000 fade - 0xFF0000FF
```

## 模块：`app`

| 操作 | 用法 |
|------|------|
| `exit` | `epassctl app exit <code>` |

`code` 可为**十进制数字**或下列**名称**（与主程序约定一致）：

| 名称 | 数值 | 含义（与主程序一致） |
|------|------|------------------------|
| `normal` | 0 | 正常退出 |
| `restart` | 1 | 重启应用 |
| `appstart` | 2 | 应用启动相关 |
| `shutdown` | 3 | 关机 |
| `format_sd` | 4 | 格式化 SD |
| `srgn_config` | 5 | SRGN 配置等 |

示例：`epassctl app exit restart` 或 `epassctl app exit 1`

**警告**：`app exit` 会导致主程序按退出码执行相应行为，请在明确后果后使用。

## 退出码

- **0**：命令执行成功（含 IPC 返回成功）。
- **非 0**：用法错误、未知模块/操作、连接失败、IPC 错误或主程序返回错误响应。

## 许可

MIT
