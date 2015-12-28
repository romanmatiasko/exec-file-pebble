#!/usr/bin/env bash

is_muted='osascript -e "output muted of (get volume settings)"'

if `eval $is_muted`; then
  muted=false
else
  muted=true
fi

osascript -e "set volume output muted $muted" &&
echo "Volume muted: `eval $is_muted`"
