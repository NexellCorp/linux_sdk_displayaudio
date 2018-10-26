#!/bin/bash

TOP=`pwd`
RESULT_DIR="./result"
PACKAGE_ROOT="nexell/daudio"
PACKAGE_DIR="Package"
USR_BIN="usr/bin"
USR_LIB="usr/lib"
BUILD_TOP="qt_build"
USER_LIB="library/lib"
PREBUILT_LIB="library/prebuilt/lib"

function package_sdk_libraries()
{
	echo "<< Package SDK libraries >>"

	package_sdk_common_libraries
	package_sdk_prebuilt_libaries
}

function package_sdk_common_libraries()
{
	echo "<< Package SDK common libraries >>"

	mkdir -p ${RESULT_DIR}/${USR_LIB}
	cp -apvR ${TOP}/${USER_LIB}/* ${RESULT_DIR}/${USR_LIB}/
}

function package_sdk_prebuilt_libaries()
{
	echo "<< Package SDK prebuilt libraries >>"

	cp -apvR ${TOP}/${PREBUILT_LIB}/* ${RESULT_DIR}/${USR_LIB}/
}

function package_sdk_binaries()
{
	echo "<< Package SDK common binaries >>"

	mkdir -p ${RESULT_DIR}/${USR_BIN}
	cp -apvR ${TOP}/bin/* ${RESULT_DIR}/${USR_BIN}/
}

function package_sdk_applications()
{
	echo "<< Package SDK applications >>"

	package_luncher_application

	if [ ${SDK_ENABLE_BT} == "yes" ]; then
		package_bt_service_application
		package_sdk_qtapplication NxBTAudio
		package_sdk_qtapplication NxBTPhone
		package_sdk_qtapplication NxBTSettings
	elif [ ${SDK_ENABLE_BT} == "no" ]; then
		unpackage_bt_service_application
		unpackage_sdk_qtapplication NxBTAudio
		unpackage_sdk_qtapplication NxBTPhone
		unpackage_sdk_qtapplication NxBTSettings
	fi

	if [ ${SDK_ENABLE_CAM} == "yes" ]; then
		echo "<< Camera related features are not ready! >>"
	elif [ ${SDK_ENABLE_CAM} == "no" ]; then
		echo ""
	fi

	package_sdk_qtapplication NxAudioPlayer
	package_sdk_qtapplication NxVideoPlayer
}

function package_luncher_application()
{
	local app_name=NxLauncher
	echo "<< Package ${app_name} >>"

	cp -apvR ${TOP}/${BUILD_TOP}/build-${app_name}/${app_name} ${RESULT_DIR}/${USR_BIN}/
}

function package_bt_service_application()
{
	local app_name=NxBTService
	echo "<< Package ${app_name} >>"

	mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}
	cp -apvR ${TOP}/apps/${app_name}/*.so ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/
	cp -apvR ${TOP}/apps/${app_name}/${PACKAGE_DIR}/* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/
}

function unpackage_bt_service_application()
{
	local app_name=NxBTService
	if [ -d ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name} ]; then
		rm -rfvR ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}
	fi
}

function package_sdk_qtapplication()
{
	local app_name=${1}
	echo "<< Package ${app_name} >>"

	mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}
	cp -apvR ${TOP}/${BUILD_TOP}/build-${app_name}/*.so ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/
	cp -apvR ${TOP}/apps/${app_name}/${PACKAGE_DIR}/* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/
}

function unpackage_sdk_qtapplication()
{
	local app_name=${1}
	if [ -d ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name} ]; then
		rm -rfvR ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}
	fi
}

echo ""
echo "===== Displayaudio SDK packaging ====="
echo ""

if [ -d ${RESULT_DIR} ]; then
	rm -rf ${RESULT_DIR}
fi

package_sdk_libraries
package_sdk_binaries
package_sdk_applications

echo ""
echo "===== Packaging complete ====="
echo ""
