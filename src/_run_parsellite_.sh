#! /usr/bin/bash

FILE="$1"
_PPID="$2"
parcellite -c 1> "$FILE" 2>/dev/null
kill -10 "$_PPID" # send a signal to daemon process
