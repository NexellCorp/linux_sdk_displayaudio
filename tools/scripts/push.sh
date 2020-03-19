#!/bin/bash

echo ""
echo "===== Displayaudio SDK pushing ====="
echo ""
RESULT_DIR="result"
if [ ! -d ${RESULT_DIR} ]; then
	echo "result/ is no exist."
	exit -1
fi

line=$(adb version)
if [[ "$line" == *"android-sdk"* ]]; then
	adb push ${RESULT_DIR}/* /
else
	adb push ${RESULT_DIR}/ /
fi

echo ""
echo "===== Pushing complete ====="
echo ""
