import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import "qrc:/qml/"

Item {
    id: itemStatusbar

    property url backgroundSource: "/image/bg_background.png"

    property url homeSource: "/image/bg_home_normal.png"
    property url homeSourceHover: "/image/bg_home_hover.png"
    property url homeSourcePressed: "/image/bg_home_pressed.png"

    property url backSource: "/image/bg_back_normal.png"
    property url backSourceHover: "/image/bg_back_hover.png"
    property url backSourcePressed: "/image/bg_back_pressed.png"

    property url volumeSource: "/image/bg_vol.png"
    property url volumeSourceMute: "/image/bg_vol_mute.png"

    property url bluetoothSourceConnected: "/image/bg_bt_connected.png"
    property url bluetoothSourceDisconnected: "/image/bg_bt_disconnected.png"

    property url phoneSourceConnected: "/image/bg_phone_connected.png"
    property url phoneSourceDisconnected: "/image/bg_phone_disconnected.png"

    property int volumeValue: 10

    property int bluetoothStatus: 0

    property string titleText: "Nexell StatusBar Title"
    property int titleTextSize: 10

    property int textSize: 10

    signal clickedHome()
    signal clickedBack()

    onBluetoothStatusChanged: {
        if( bluetoothStatus == 0 )
        {
            idBluetooth.source = bluetoothSourceDisconnected;
            idBluetoothText.text = "Disconnected"
        }
        else
        {
            idBluetooth.source = bluetoothSourceConnected;
            idBluetoothText.text = "Connected"
        }
    }

    Image {
        id: idBackround
        anchors.fill: parent
        source: backgroundSource
    }

    NxImageButton {
        id: idBtnHome

        checkable: true
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter

        imageSource: homeSource
        imageSourceHover: homeSourceHover
        imageSourcePressed: homeSourcePressed

        onClicked: {
            itemStatusbar.clickedHome()
        }
    }

    Text {
        id: idTextTitle

        text: titleText
        anchors.left: idBtnHome.right
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width * 2 / 10
        font.pointSize: textSize
    }

    GridLayout {
        id: idGridLayout

        anchors.left: idTextTitle.right
        anchors.right: idBtnBack.left

        anchors.verticalCenter: parent.verticalCenter

        // Time
        Text {
            id: idTextTime
            font.pointSize: textSize
            text: "00:00:00 AM"
        }

        // Volume
        Image {
            id: idVolume
            source: volumeSource
        }

        Text {
            id: idVolumeText
            anchors.leftMargin: 5
            anchors.left: idVolume.right
            font.pointSize: textSize
            text: volumeValue
        }

        // Bluetooth
        Image {
            id: idBluetooth
            source: bluetoothSourceDisconnected
        }

        Text {
            id: idBluetoothText
            anchors.leftMargin: 5
            anchors.left: idBluetooth.right
            font.pointSize: textSize
            text: "Disconnected"
        }

        // Phone
        Image {
            id: idPhone
            source: phoneSourceDisconnected
        }

        Text {
            id: idPhonetext
            anchors.leftMargin: 5
            anchors.left: idPhone.right
            font.pointSize: textSize
            text: "Disconnected"
        }
    }

    CNX_ImageButton {
        id: idBtnBack

        checkable: true
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: 10

        imageSource: backSource
        imageSourceHover: backSourceHover
        imageSourcePressed: backSourcePressed

        onClicked: {
            itemStatusbar.clickedBack()
        }
    }

    Timer {
        id: timer
        interval: 1000
        running: true
        repeat: true

        onTriggered: {
            updateTime()
        }
    }

    function updateTime() {
        idTextTime.text = Qt.formatTime(new Date(),"hh:mm:ss AP")
    }

    Component.onCompleted: {
        timer.start()
        updateTime()
    }
}
