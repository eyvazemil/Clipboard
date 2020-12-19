#! /bin/sh

BASE_DIR="$1"
_PPID="$2"

git pull &>/dev/null

kill -10 "$_PPID" # send a signal to daemon process
