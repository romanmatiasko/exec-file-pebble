#!/usr/bin/env bash

get_volume='osascript -e "output volume of (get volume settings)"'

osascript -e "set volume output volume $((`eval $get_volume` + 100 / 16))" &&
echo "Volume: `eval $get_volume`"
