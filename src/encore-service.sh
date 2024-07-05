#!/bin/sh
su -lp 2000 -c "cmd notification post -S bigtext -t 'ENCORE' \"Tag$(date +%s) Tweaks applied successfully\""
sleep 1
mkdir -p /dev/encore/AI
printf unset >/dev/encore/AI/mode
while true; do
  mode_apply=$(cat /dev/encore/AI/mode)
  sleep 1
  gamestart=$(dumpsys window | grep -E 'mCurrentFocus|mFocusedApp' | grep -Eo "$(cat /data/encore/gamelist.txt)" | tail -n 1)
  sleep 1
  screenoff=$(dumpsys display | grep "mScreenState" | awk -F'=' '{print $2}')
  sleep 1
  if [ "$gamestart" ]; then
    if [ ! "$mode_apply" = performance ]; then
      sleep 1
      for pid in $(pgrep -f "$gamestart"); do
        renice -n -20 -p "$pid"
        ionice -c 1 -n 0 -p "$pid"
        chrt -f -p 98 "$pid"
      done
      am start -a android.intent.action.MAIN -e toasttext "Boosting game $1" -n bellavita.toast/.MainActivity
      sh /system/bin/encore-performance
      sleep 1
      printf performance >/dev/encore/AI/mode
      sleep 1
    fi
  elif [ "$screenoff" = off ]; then
    if [ ! "$mode_apply" = sleep ]; then
      sleep 1
      sh /system/bin/encore-powersave
      sleep 1
      printf lowpower >/dev/encore/AI/mode
      sleep 1
    fi
  else
    if [ ! "$mode_apply" = normal ]; then
      sleep 1
      sh /system/bin/encore-normal
      printf normal >/dev/encore/AI/mode
    fi
  fi
  sleep 12
done