#! /bin/sh

BASE_DIR="$1"
_PPID="$2"

git pull

kill -10 "$_PPID" # send a signal to daemon process
