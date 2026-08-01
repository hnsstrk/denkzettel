#!/bin/bash
# Startet eine verschachtelte KWin-Sitzung mit eigenem D-Bus und eigenen
# XDG-Verzeichnissen und ruft darin nachweis.sh auf. Die Plasma-Sitzung des
# Rechners wird nicht angefasst: plasma-apply-colorscheme schreibt
# ausschliesslich in das kdeglobals dieser Sandbox.
#
# Aufruf:  SANDBOX=/pfad/zur/sandbox ./sitzung.sh
# Erwartet den installierten Dienst unter $SANDBOX/root/usr/bin/denkzetteld
# (DESTDIR=$SANDBOX/root cmake --install build).
set -u

export SANDBOX="${SANDBOX:-$(mktemp -d)}"
export XDG_CONFIG_HOME="$SANDBOX/config"
export XDG_DATA_HOME="$SANDBOX/data"
export XDG_CACHE_HOME="$SANDBOX/cache"
export XDG_STATE_HOME="$SANDBOX/state"
export XDG_DATA_DIRS="$SANDBOX/root/usr/share:/usr/share"
mkdir -p "$XDG_CONFIG_HOME" "$XDG_DATA_HOME" "$XDG_CACHE_HOME" "$XDG_STATE_HOME"

unset WAYLAND_DISPLAY
unset DISPLAY

exec dbus-run-session -- kwin_wayland \
    --virtual --width 1000 --height 700 --xwayland \
    --socket denkzettel-probe \
    "$(dirname "$(readlink -f "$0")")/nachweis.sh"
