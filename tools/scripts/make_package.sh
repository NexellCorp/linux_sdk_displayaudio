#!/bin/bash

TOP=`pwd`
RESULT_DIR="./result"
PACKAGE_ROOT="nexell/daudio"
PACKAGE_DIR="Package"
USR_BIN="usr/bin"
USR_SBIN="usr/sbin"
USR_LIB="usr/lib"
NXBT_ETC="etc/bluetooth"
BUILD_TOP="qt_build"

function package_luncher()
{
	echo "<< Package Launcher Application>>"
	cp -aR ${TOP}/${BUILD_TOP}/build-NxLauncher/NxLauncher ${RESULT_DIR}/${USR_BIN}/

	echo "<< Package ipc libraries >>"
	cp -aR ${TOP}/library/lib/libnxconfig.so ${RESULT_DIR}/${USR_LIB}/
	cp -aR ${TOP}/library/lib/libnxdaudioutils.so ${RESULT_DIR}/${USR_LIB}/
}

function package_bt_service()
{
	local app_name=NxBTService
	echo "<< Package ${app_name} >>"
	mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}
	cp -aR ${TOP}/apps/${app_name}/*.so ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/
	cp -aR ${TOP}/apps/${app_name}/${PACKAGE_DIR}/* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/
}

function package_bt_application()
{
	local app_name=${1}
	echo "<< Package ${1} >>"
	mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name};
	cp -aR ${TOP}/apps/${app_name}/${PACKAGE_DIR}/* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;
	cp -aR ${TOP}/${BUILD_TOP}/build-${app_name}/*.so ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;
}

function package_videoplayer_application()
{
    local app_name=NxVideoPlayer
    echo "<< Package ${app_name} >>"
    mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name};
    cp -aR ${TOP}/apps/${app_name}/${PACKAGE_DIR}/* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;
    cp -aR ${TOP}/${BUILD_TOP}/build-${app_name}/*.so* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;
}

function package_audioplayer_application()
{
    local app_name=NxAudioPlayer
    echo "<< Package ${app_name} >>"
    mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name};
	mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/lib;

	echo "<< Copy ${app_name} Binary & Package Informations >>"
    cp -aR ${TOP}/apps/${app_name}/${PACKAGE_DIR}/* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;
    cp -aR ${TOP}/${BUILD_TOP}/build-${app_name}/*.so* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;

	echo "<< Package ID3 Libraries >>"
	cp -aR ${TOP}/library/lib/id3-3.8/lib/libid3.so* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/lib/;
	cp -aR ${TOP}/library/lib/id3-3.8/lib/libid3-3.8.* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/lib/;
}

function package_quickrearcam_application()
{
	local app_name=NxQuickRearCam
	echo "<< Package ${app_name} >>"
	mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name};
	mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/lib;

	echo "<< Copy ${app_name} Binary & Package Informations >>"
	cp -aR ${TOP}/apps/${app_name}/${PACKAGE_DIR}/* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;
	cp -aR ${TOP}/${BUILD_TOP}/build-${app_name}/${app_name} ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;

	echo "<< Copy Rear Camera Libraries >>"
	cp -aR ${TOP}/library/lib/libnxrearcam*  ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/lib/
}

function package_avin_application()
{
	local app_name=NxAVIn
	echo "<< Package ${app_name} >>"
	mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name};
	mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/lib;

	echo "<< Copy ${app_name} Binary & Package Informations >>"
	cp -aR ${TOP}/apps/${app_name}/${PACKAGE_DIR}/* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;
	cp -aR ${TOP}/${BUILD_TOP}/build-${app_name}/${app_name} ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;

	echo "<< Copy Rear Camera Libraries >>"
	cp -aR ${TOP}/library/lib/libnxavin*  ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/lib/
}

function package_common_libraries()
{
	mkdir -p ${RESULT_DIR}/${USR_LIB}
	mkdir -p ${RESULT_DIR}/${USR_BIN}

	echo "<< Package Nexell Base UI >>"
	cp -aR ${TOP}/${BUILD_TOP}/build-libnxbaseui/libnxbaseui.so* ${RESULT_DIR}/${USR_LIB}/

	echo "<< Package Nexell Video Libraries >>"
	cp -aR ${TOP}/library/lib/libnxfilter* ${RESULT_DIR}/${USR_LIB}/
	cp -aR ${TOP}/library/lib/libnxsubtitleparser.so ${RESULT_DIR}/${USR_LIB}/
	cp -aR ${TOP}/library/lib/libnxmpmanager.so ${RESULT_DIR}/${USR_LIB}/
	cp -aR ${TOP}/library/lib/libnxdaudioutils.so ${RESULT_DIR}/${USR_LIB}/

	echo "<< Package Key Input Receiver >>"
	cp -aR ${TOP}/library/lib/libnxkeyreceiver.so ${RESULT_DIR}/${USR_LIB}/

	echo "<< Package Nexell BT Libraries, server & HCD file >>"
	cp -aR ${TOP}/library/lib/nxbt1xx/lib/libappbt.so ${RESULT_DIR}/${USR_LIB}/
	cp -aR ${TOP}/library/lib/nxbt1xx/lib/libnxbt.so ${RESULT_DIR}/${USR_LIB}/
	cp -aR ${TOP}/library/lib/nxbt1xx/lib/libnxalsa.so ${RESULT_DIR}/${USR_LIB}/
	cp -aR ${TOP}/library/lib/nxbt1xx/lib/libnrec_hf.so ${RESULT_DIR}/${USR_LIB}/
}

function package_etc()
{
	local app_name=${1}
	echo "<< Package etc Applications >>"
	mkdir -p ${RESULT_DIR}/${USR_BIN}
	cp -aR ${TOP}/bin/* ${RESULT_DIR}/${USR_BIN}/
	mkdir -p ${RESULT_DIR}/${USR_SBIN}
}

echo ""
echo "===== Application Packaging ====="
echo ""

mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT};

package_common_libraries
package_luncher
package_audioplayer_application
package_videoplayer_application
#package_quickrearcam_application
#package_avin_application

package_bt_service
package_bt_application NxBTAudio
package_bt_application NxBTPhone
package_bt_application NxBTSettings
package_etc
