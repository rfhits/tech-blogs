---
title: Fedora 配置
created: 2026-04-23T22:10+08:00
modified: 2026-04-23T22:12+08:00
categories:
  - applications
tags:
  - fedora
  - input-method
  - fcitx5
  - rime
  - kde
  - wayland
---

# Fedora 配置

## 输入法故障排查（Konsole/KWrite/Vim 无法输入中文）

### 现象

- 浏览器可以输入中文，但 `Konsole`、`KWrite`、`Vim` 都只能输入英文。
- `fcitx5` 图标状态不稳定，重启输入法后问题仍反复。

### 排查思路

1. 先判断是不是 Vim 本身问题：  
   结论不是，因为 `Konsole/KWrite` 也无法输入中文。
2. 检查 Vim 编译和编码：  
   `vim --version` 显示有 `+multi_byte`，编码也为 `utf-8`，不是 Vim 功能缺失。
3. 检查输入法环境变量与输入法状态：  
   `fcitx5-diagnose`、`imsettings-info` 显示会话使用了 `none/xim`，关键变量是：
   - `XMODIFIERS=@im=none`
   - `GTK_IM_MODULE=gtk-im-context-simple`
   - `QT_IM_MODULE=xim`
4. 对比 KDE Wayland 配置：  
   发现 `imsettings` 在会话里指向 `none.conf`，把输入法链路带偏。

### 根因

根因不是应用本身，而是 **KDE Wayland 会话中的输入法环境被设成了“禁用输入法”**。  
Qt/终端应用依赖这些变量，因此集体失效；浏览器因为接入链路不同，表现为“还能输入中文”。

### 解决方法

1. 固定输入法环境变量为 `fcitx`（持久化）：

```bash
~/.config/environment.d/99-fcitx5.conf
XMODIFIERS=@im=fcitx
GTK_IM_MODULE=fcitx
QT_IM_MODULE=fcitx
SDL_IM_MODULE=fcitx
```

2. 禁用会覆盖配置的 `imsettings` 自动启动项：

```bash
~/.config/autostart/imsettings-start.desktop
[Desktop Entry]
Hidden=true
```

3. 给 KDE 会话增加环境脚本（登录时生效）：

```bash
~/.config/plasma-workspace/env/fcitx5.sh
#!/usr/bin/env bash
export XMODIFIERS='@im=fcitx'
export GTK_IM_MODULE='fcitx'
export QT_IM_MODULE='fcitx'
export SDL_IM_MODULE='fcitx'
```

4. 当前会话即时生效（不等重启）：

```bash
dbus-update-activation-environment --systemd \
  XMODIFIERS=@im=fcitx GTK_IM_MODULE=fcitx QT_IM_MODULE=fcitx SDL_IM_MODULE=fcitx
pkill -x fcitx5
fcitx5 -d
```

5. 最终通过重启/重新登录彻底恢复（本次重启后恢复正常）。

### 经验总结

- 遇到“只有某些应用不能中文输入”，优先查会话输入法变量，不要先怀疑 Vim。
- 在 KDE Wayland 下，`imsettings` 与 `fcit5` 并存时要特别注意是否被 `none.conf` 覆盖。
- 浏览器能输入中文，不代表系统输入法链路整体正常。
