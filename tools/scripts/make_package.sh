#!/bin/bash

TOP=`pwd`
RESULT_DIR="result"
QT_APP_ROOT="nexell/daudio"
PACKAGE_DIR="Package"
USR_BIN="usr/bin"
USR_LIB="usr/lib"
QT_BUILD_TOP="qt_build"
BUILT_LIB="library/lib"
BUILT_BIN="bin"
PREBUILT_LIB="library/prebuilt/lib"

function package_sdk_prebuilt_all()
{
	package_sdk_prebuilt_libaries
}

function package_sdk_built_all()
{
	package_sdk_binaries
	package_sdk_libraries
	package_sdk_qtapplications
}

function package_sdk_prebuilt_libaries()
{
	echo "<< Package SDK prebuilt libraries >>"

	mkdir -p ${RESULT_DIR}/${USR_LIB}
	cp -apvR ${TOP}/${PREBUILT_LIB}/* ${RESULT_DIR}/${USR_LIB}/
}

function package_sdk_binaries()
{
	echo "<< Package SDK binaries >>"

	mkdir -p ${RESULT_DIR}/${USR_BIN}
	cp -apvR ${TOP}/${BUILT_BIN}/* ${RESULT_DIR}/${USR_BIN}/
}

function package_sdk_libraries()
{
	echo "<< Package SDK libraries >>"

	mkdir -p ${RESULT_DIR}/${USR_LIB}
	cp -apvR ${TOP}/${BUILT_LIB}/* ${RESULT_DIR}/${USR_LIB}/
}

function package_sdk_qtapplications()
{
	echo "<< Package SDK Qt applications >>"

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
		package_sdk_qtapplication NxAVIn
		package_sdk_qtapplication NxRearCam
	elif [ ${SDK_ENABLE_CAM} == "no" ]; then
		unpackage_sdk_qtapplication NxAVIn
		unpackage_sdk_qtapplication NxRearCam
	fi

	package_sdk_qtapplication NxAudioPlayer
	package_sdk_qtapplication NxVideoPlayer
}

function package_luncher_application()
{
	local app_name=NxLauncher
	echo "<< Package ${app_name} >>"

	cp -apvR ${TOP}/${QT_BUILD_TOP}/build-${app_name}/${app_name} ${RESULT_DIR}/${USR_BIN}/
}

function package_bt_service_application()
{
	local app_name=NxBTService
	echo "<< Package ${app_name} >>"

	mkdir -p ${RESULT_DIR}/${QT_APP_ROOT}/${app_name}
	cp -apvR ${TOP}/apps/${app_name}/*.so ${RESULT_DIR}/${QT_APP_ROOT}/${app_name}/
	cp -apvR ${TOP}/apps/${app_name}/${PACKAGE_DIR}/${app_name}.desktop ${RESULT_DIR}/${QT_APP_ROOT}/${app_name}/
	if [ ${TARGET_MACHINE} == "s5p4418-convergence-daudio" ]; then
		cp -apvR ${TOP}/apps/${app_name}/${PACKAGE_DIR}/nxbtservice_config_convergence.xml ${RESULT_DIR}/${QT_APP_ROOT}/${app_name}/nxbtservice_config.xml
	else
		cp -apvR ${TOP}/apps/${app_name}/${PACKAGE_DIR}/nxbtservice_config_daudio_ref.xml ${RESULT_DIR}/${QT_APP_ROOT}/${app_name}/nxbtservice_config.xml
	fi
}

function unpackage_bt_service_application()
{
	local app_name=NxBTService
	if [ -d ${RESULT_DIR}/${QT_APP_ROOT}/${app_name} ]; then
		rm -rfvR ${RESULT_DIR}/${QT_APP_ROOT}/${app_name}
	fi
}

function package_sdk_qtapplication()
{
	local app_name=${1}
	echo "<< Package ${app_name} >>"

	mkdir -p ${RESULT_DIR}/${QT_APP_ROOT}/${app_name}
	cp -apvR ${TOP}/${QT_BUILD_TOP}/build-${app_name}/*.so ${RESULT_DIR}/${QT_APP_ROOT}/${app_name}/
	cp -apvR ${TOP}/apps/${app_name}/${PACKAGE_DIR}/*.png ${RESULT_DIR}/${QT_APP_ROOT}/${app_name}/
	cp -apvR ${TOP}/apps/${app_name}/${PACKAGE_DIR}/${app_name}.desktop ${RESULT_DIR}/${QT_APP_ROOT}/${app_name}/
	if [ ${app_name} == "NxRearCam" ]; then
		if [ ${TARGET_MACHINE} == "s5p4418-convergence-daudio" ]; then
			cp -apvR ${TOP}/apps/${app_name}/${PACKAGE_DIR}/rearcam_config_convergence.xml ${RESULT_DIR}/${QT_APP_ROOT}/${app_name}/rearcam_config.xml
		else
			cp -apvR ${TOP}/apps/${app_name}/${PACKAGE_DIR}/rearcam_config_daudio_ref.xml ${RESULT_DIR}/${QT_APP_ROOT}/${app_name}/rearcam_config.xml
		fi
	fi
}

function unpackage_sdk_qtapplication()
{
	local app_name=${1}
	if [ -d ${RESULT_DIR}/${QT_APP_ROOT}/${app_name} ]; then
		rm -rfvR ${RESULT_DIR}/${QT_APP_ROOT}/${app_name}
	fi
}

echo ""
echo "===== Displayaudio SDK packaging ====="
echo ""

if [ -d ${RESULT_DIR} ]; then
	rm -rfR ${RESULT_DIR}
fi

mkdir -p ${RESULT_DIR}

package_sdk_prebuilt_all
package_sdk_built_all

echo ""
echo "===== Packaging complete ====="
echo ""
