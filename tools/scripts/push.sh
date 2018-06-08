#!/bin/bash
echo "<<< Push Binaries >>>"
RESULT_DIR="./result"
adb push ${RESULT_DIR} /
adb shell sync
