#!/bin/bash
# Spike #50 — the runnable minimal example. Opens a nested kwin_wayland session
# with a throwaway HOME, loads origin.js into that KWin, opens three foreign
# windows with titles of its own making, starts denkzetteld and asks it twice
# for the capture window. sink.py stands in for the daemon and writes down what
# KWin tells it.
#
#   docs/spikes/50-active-window/run.sh <path-to-denkzetteld> [output-dir]
#
# Never run against the session you are working in: window titles are personal
# data and this repository is public. The nested session is the only variant.
set -u

DZ="${1:?usage: run.sh <path-to-denkzetteld> [output-dir]}"
OUT="${2:-$(mktemp -d)}"
HERE="$(cd "$(dirname "$0")" && pwd)"
SAND="$(mktemp -d)"

mkdir -p "$OUT" "$SAND/.config"
printf '[Theme]\nname=breeze-dark\n' > "$SAND/.config/plasmarc"
printf '[General]\nColorScheme=BreezeDark\n' > "$SAND/.config/kdeglobals"

cat > "$SAND/inner.sh" <<'INNER'
#!/bin/sh
O="$OUT"
python3 "$HERE/sink.py" "$O/sink.log" > "$O/sink-stderr.txt" 2>&1 &
SINK=$!
sleep 3

# Read the two answers back rather than trusting the call: loadScript returns an
# id even for a file KWin never runs, isScriptLoaded is the one that says.
busctl --user call org.kde.KWin /Scripting org.kde.kwin.Scripting \
    loadScript ss "$HERE/origin.js" denkzettel-origin > "$O/loadscript.txt" 2>&1
busctl --user call org.kde.KWin /Scripting org.kde.kwin.Scripting \
    start > "$O/start.txt" 2>&1
busctl --user call org.kde.KWin /Scripting org.kde.kwin.Scripting \
    isScriptLoaded s denkzettel-origin > "$O/isloaded.txt" 2>&1
sleep 2

# Three switches between foreign windows. Nothing may reach the sink here.
kdialog --title "Fenster A" --msgbox "A" > /dev/null 2>&1 &
sleep 4
kdialog --title "Fenster B" --msgbox "B" > /dev/null 2>&1 &
sleep 4
kdialog --title "Fenster C" --msgbox "C" > /dev/null 2>&1 &
sleep 4

"$DZ" > "$O/daemon.txt" 2>&1 &
DAEMON=$!
sleep 8
dbus-send --session --print-reply --reply-timeout=5000 \
    --dest=io.github.hnsstrk.denkzettel /Daemon \
    io.github.hnsstrk.denkzettel.Daemon.ShowCapture \
    > "$O/showcapture1.txt" 2>&1
sleep 5

kdialog --title "Fenster D" --msgbox "D" > /dev/null 2>&1 &
sleep 5
dbus-send --session --print-reply --reply-timeout=5000 \
    --dest=io.github.hnsstrk.denkzettel /Daemon \
    io.github.hnsstrk.denkzettel.Daemon.ShowCapture \
    > "$O/showcapture2.txt" 2>&1
sleep 5

kill $DAEMON > /dev/null 2>&1
sleep 2
pkill -f "kdialog --title" > /dev/null 2>&1
sleep 2
kill $SINK > /dev/null 2>&1
sleep 1
INNER
# The execute bit is not decoration: kwin_wayland hangs without a word when the
# program behind its "--" cannot be executed.
chmod +x "$SAND/inner.sh"

env OUT="$OUT" HERE="$HERE" DZ="$DZ" HOME="$SAND" \
    XDG_CONFIG_HOME="$SAND/.config" XDG_DATA_HOME="$SAND/.local/share" \
    XDG_CACHE_HOME="$SAND/.cache" \
    dbus-run-session -- kwin_wayland --virtual --width 1200 --height 800 \
    --socket "denkzettel-spike50-$$" --no-lockscreen -- "$SAND/inner.sh" \
    > "$OUT/kwin-stderr.txt" 2>&1 &
KWIN=$!

# kwin_wayland does not always exit when its session program does.
for i in $(seq 1 24); do
    sleep 5
    [ -f "$OUT/showcapture2.txt" ] && break
done
sleep 12
kill $KWIN 2>/dev/null
pkill -f "denkzettel-spike50-$$" 2>/dev/null

echo "--- $OUT/sink.log"
cat "$OUT/sink.log"
