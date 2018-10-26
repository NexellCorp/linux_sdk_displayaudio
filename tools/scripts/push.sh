#!/bin/bash
echo "===== Displayaudio SDK pushing ====="
RESULT_DIR="./result"
adb push ${RESULT_DIR} /
adb shell sync
echo "===== Pushing complete ====="
