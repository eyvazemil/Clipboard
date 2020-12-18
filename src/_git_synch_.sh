#! /bin/sh

BASE_DIR="$1"
_PPID="$2"

git add "$BASE_DIR"
git commit -m "Update clipboard history"
git push origin master

kill -10 "$_PPID" # send a signal to daemon process
 