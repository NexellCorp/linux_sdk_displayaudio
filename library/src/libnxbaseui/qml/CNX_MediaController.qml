import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import "qrc:/qml/"

Item {
    id: itemMediaController

    Image {
        id: idBackround
        anchors.fill: parent
        source: "/image/bg_background.png"
    }

    GridLayout {
        id: idGridLayout

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter

        CNX_ImageButton {
            id: idBtnPlay

            checkable: true
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter

            imageSource: "/image/ic_media_play.png"

            onClicked: {
            }
        }

    }
}
