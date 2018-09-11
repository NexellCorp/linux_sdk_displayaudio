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

function package_podo_compositor()
{
	echo "<< Package podo compositor"
	mkdir -p ${RESULT_DIR}/podo/apps/pdwindowcompositor
	cp -av ${TOP}/${BUILD_TOP}/build-core/podo ${RESULT_DIR}/podo/
	cp -av ${TOP}/${BUILD_TOP}/build-pdwindowcompositor/pdwindowcompositor ${RESULT_DIR}/podo/apps/pdwindowcompositor/
}

function package_luncher()
{
	echo "<< Package Launcher Application>>"
	cp -va ${TOP}/${BUILD_TOP}/build-NxLauncher/NxLauncher ${RESULT_DIR}/${USR_BIN}/
}

function pakcage_daudiomanger()
{
    local app_name=NxDAudioManager
	echo "<< Package ${app_name} >>"
	cp -va ${TOP}/apps/${app_name}/${app_name} ${RESULT_DIR}/${USR_BIN}/
	echo "<< Package ipc libraries >>"
	cp -va ${TOP}/library/lib/libnxdaudioipc.so ${RESULT_DIR}/${USR_LIB}/
	cp -va ${TOP}/library/lib/libnxconfig.so ${RESULT_DIR}/${USR_LIB}/
	cp -va ${TOP}/library/lib/libnxdaudioutils.so ${RESULT_DIR}/${USR_LIB}/
}

function package_btr_service()
{
	local app_name=NxBTServiceR
	echo "<< Package ${app_name} >>"
	cp -va ${TOP}/${BUILD_TOP}/build-${app_name}/${app_name} ${RESULT_DIR}/${USR_BIN}/
}

function unpackage_btr_service()
{
	local app_name=NxBTServiceR
	echo "<< Unpackage ${app_name} >>"
	if [ -f ${RESULT_DIR}/${USR_BIN}/${app_name} ]; then
		rm -rf ${RESULT_DIR}/${USR_BIN}/${app_name}
	fi
}

function package_btr_application()
{
	local app_name=${1}
	echo "<< Package ${1} >>"
	mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name};
	cp -va ${TOP}/apps/${app_name}/${PACKAGE_DIR}/* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;
	cp -va ${TOP}/${BUILD_TOP}/build-${app_name}/${app_name} ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;
}

function unpackage_btr_application()
{
	local app_name=${1}
	echo "<< Unpackage ${1} >>"
	if [ -d ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name} ]; then
		rm -rf ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name};
	fi
}

function package_application()
{
    local app_name=${1}
    echo "<< Package ${1} >>"
    mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name};
    cp -va ${TOP}/apps/${app_name}/${PACKAGE_DIR}/* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;
    cp -va ${TOP}/${BUILD_TOP}/build-${app_name}/${app_name} ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;
}

function package_audioplayer_application()
{
    local app_name=NxAudioPlayer
    echo "<< Package ${app_name} >>"
    mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name};
	mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/lib;

	echo "<< Copy ${app_name} Binary & Package Informations >>"
    cp -va ${TOP}/apps/${app_name}/${PACKAGE_DIR}/* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;
    cp -va ${TOP}/${BUILD_TOP}/build-${app_name}/${app_name} ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;

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
	cp -va ${TOP}/apps/${app_name}/${PACKAGE_DIR}/* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;
	cp -va ${TOP}/${BUILD_TOP}/build-${app_name}/${app_name} ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;

	echo "<< Copy Rear Camera Libraries >>"
	cp -va ${TOP}/library/lib/libnxrearcam*  ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/lib/
}

function unpackage_quickrearcam_application()
{
	local app_name=NxQuickRearCam
	echo "<< Unpackage ${app_name} >>"
	if [ -d ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name} ]; then
		rm -rf ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name};
	fi
}

function package_avin_application()
{
	local app_name=NxAVIn
	echo "<< Package ${app_name} >>"
	mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name};
	mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/lib;

	echo "<< Copy ${app_name} Binary & Package Informations >>"
	cp -va ${TOP}/apps/${app_name}/${PACKAGE_DIR}/* ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;
	cp -va ${TOP}/${BUILD_TOP}/build-${app_name}/${app_name} ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/;

	echo "<< Copy Rear Camera Libraries >>"
	cp -va ${TOP}/library/lib/libnxavin*  ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name}/lib/
}

function unpackage_avin_application()
{
	local app_name=NxAVIn
	echo "<< Unpackage ${app_name} >>"
	if [ -d ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name} ]; then
		rm -rf ${RESULT_DIR}/${PACKAGE_ROOT}/${app_name};
	fi
}

function package_common_libraries()
{
	mkdir -p ${RESULT_DIR}/${USR_LIB}
	mkdir -p ${RESULT_DIR}/${USR_BIN}

	echo "<< Package Nexell Base UI >>"
	cp -aR ${TOP}/${BUILD_TOP}/build-libnxbaseui/libnxbaseui.so* ${RESULT_DIR}/${USR_LIB}/

	echo "<< Package Nexell Pacp Client Wapprer >> "
	cp -aR ${TOP}/${BUILD_TOP}/build-libnxpacpclient/libnxpacpclient.so* ${RESULT_DIR}/${USR_LIB}/

	echo "<< Package Nexell Video Libraries >>"
	cp -va ${TOP}/library/lib/libnxfilter* ${RESULT_DIR}/${USR_LIB}/
	cp -va ${TOP}/library/lib/libnxsubtitleparser.so ${RESULT_DIR}/${USR_LIB}/
	cp -va ${TOP}/library/lib/libnxmpmanager.so ${RESULT_DIR}/${USR_LIB}/

	echo "<< Package Key Input Receiver >>"
	cp -va ${TOP}/library/lib/libnxkeyreceiver.so ${RESULT_DIR}/${USR_LIB}/

	if [ ${SDK_ENABLE_BT} == "yes" ]; then
		echo "<< Package Nexell BT Libraries >>"
		cp -va ${TOP}/library/lib/nxbt1xx/lib/libappbt.so ${RESULT_DIR}/${USR_LIB}/
		cp -va ${TOP}/library/lib/nxbt1xx/lib/libnxbt.so ${RESULT_DIR}/${USR_LIB}/
		cp -va ${TOP}/library/lib/nxbt1xx/lib/libnxalsa.so ${RESULT_DIR}/${USR_LIB}/
		cp -va ${TOP}/library/lib/nxbt1xx/lib/libnrec_hf.so ${RESULT_DIR}/${USR_LIB}/
	elif [ ${SDK_ENABLE_BT} == "no" ]; then
		echo "<< Unpackage Nexell BT Libraries >>"
		if [ -f ${RESULT_DIR}/${USR_LIB}/libappbt.so ]; then
			rm -rf ${RESULT_DIR}/${USR_LIB}/libappbt.so
		fi
		if [ -f ${RESULT_DIR}/${USR_LIB}/libnxbt.so ]; then
			rm -rf ${RESULT_DIR}/${USR_LIB}/libnxbt.so
		fi
		if [ -f ${RESULT_DIR}/${USR_LIB}/libnxalsa.so ]; then
			rm -rf ${RESULT_DIR}/${USR_LIB}/libnxalsa.so
		fi
		if [ -f ${RESULT_DIR}/${USR_LIB}/libnrec_hf.so ]; then
			rm -rf ${RESULT_DIR}/${USR_LIB}/libnrec_hf.so
		fi
	else
		echo "[NXBT-libs] Invalid bluetooth-related SDK argument, default is "yes""
		echo "<< Package Nexell BT Libraries >>"
		cp -va ${TOP}/library/lib/nxbt1xx/lib/libappbt.so ${RESULT_DIR}/${USR_LIB}/
		cp -va ${TOP}/library/lib/nxbt1xx/lib/libnxbt.so ${RESULT_DIR}/${USR_LIB}/
		cp -va ${TOP}/library/lib/nxbt1xx/lib/libnxalsa.so ${RESULT_DIR}/${USR_LIB}/
		cp -va ${TOP}/library/lib/nxbt1xx/lib/libnrec_hf.so ${RESULT_DIR}/${USR_LIB}/
	fi
}

function package_etc()
{
	echo "<< Package etc Applications >>"
	mkdir -p ${RESULT_DIR}/${USR_BIN}
	cp -aR ${TOP}/bin/* ${RESULT_DIR}/${USR_BIN}/

	if [ ${SDK_ENABLE_BT} == "yes" ]; then
		cp -va ${TOP}/apps/NxService/NxService_use_bt.sh ${RESULT_DIR}/${USR_BIN}/NxService.sh
	elif [ ${SDK_ENABLE_BT} == "no" ]; then
		cp -va ${TOP}/apps/NxService/NxService_nouse_bt.sh ${RESULT_DIR}/${USR_BIN}/NxService.sh
		if [ -f ${RESULT_DIR}/${USR_BIN}/NxBTServiceConsole ]; then
			rm -rf ${RESULT_DIR}/${USR_BIN}/NxBTServiceConsole
		fi
	else
		echo "[NxService] Invalid bluetooth-related SDK argument, default is "yes""
		cp -va ${TOP}/apps/NxService/NxService_use_bt.sh ${RESULT_DIR}/${USR_BIN}/NxService.sh
	fi

	mkdir -p ${RESULT_DIR}/${USR_SBIN}
}

echo ""
echo "===== Application Packaging ====="
echo ""

mkdir -p ${RESULT_DIR}/${PACKAGE_ROOT};

package_common_libraries
package_luncher
pakcage_daudiomanger
package_audioplayer_application

if [ ${SDK_ENABLE_CAM} == "yes" ]; then
	package_quickrearcam_application
	package_avin_application
elif [ ${SDK_ENABLE_CAM} == "no" ]; then
	unpackage_quickrearcam_application
	unpackage_avin_application
else
	echo "[NXCAM] Invalid nexell camera SDK argument, default is "yes""
	package_quickrearcam_application
	package_avin_application
fi

package_application NxVideoPlayer

if [ ${SDK_ENABLE_BT} == "yes" ]; then
	package_btr_service
	package_btr_application NxBTAudioR
	package_btr_application NxBTPhoneR
	package_btr_application NxBTSettingsR
elif [ ${SDK_ENABLE_BT} == "no" ]; then
	unpackage_btr_service
	unpackage_btr_application NxBTAudioR
	unpackage_btr_application NxBTPhoneR
	unpackage_btr_application NxBTSettingsR
else
	echo "[NXBT] Invalid nexell bluetooth SDK argument, default is "yes""
	package_btr_service
	package_btr_application NxBTAudioR
	package_btr_application NxBTPhoneR
	package_btr_application NxBTSettingsR
fi

package_etc

if [ ${OECORE_SDK_VERSION} != "2.3.1" ]; then
	package_podo_compositor
fi

