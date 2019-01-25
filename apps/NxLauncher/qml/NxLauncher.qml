import QtQuick 2.0
//import QtQuick.Controls 2.0

import "qrc:/qml/"


//property ListModel appListModel

Item {
    id: pane

    property string tag: "NxLauncher.qml: "

    property url imageSource: "/image/bg_background.png"
    property int maxColumnsInPage:0
    property int maxRowsInPage:0
    property int totalPage:3
    property int currentPage:0
    property int iconCellWidth: 250
    property int iconCellHeight: 250

    signal launchProgram(string name)

    Image {
        id: background
        anchors.fill: parent
        source: imageSource
    }

    ListModel {
        id: appListModel
    }

    NxSwipeView {
        id: swipe
        anchors.fill: parent
    }

    Rectangle {
        id: pageIndicator
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        color: "transparent"
        width: parent.width
        height: 30

        Row {
            spacing: 5
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            Repeater {
                model: swipe.totalPage

                Rectangle {
                    width: 10
                    height: 10
                    radius: width/2
                    color: {
                        if (index === swipe.currentIndex)
                            "#303032"
                        else
                            "#7D7D7E"
                    }
                }
            }
        }
    }

    function createAppList() {
        appListModel.clear()

        var info = gui.getPluginInfoList()
        for(var idx =0 ; idx < info.length; idx++) {
            var i = idx % info.length

            if (!info[i].visible)
                continue

            appListModel.append({
                "name": info[i].name,
                "path": info[i].path,
                "icon": info[i].icon,
                "exec": info[i].exec,
                "active": info[i].active
            })
        }
    }

    function initialize() {
        pane.createAppList()

        swipe.updateAppList()
    }

    function activeChanged() {
        var i, k;
        var info = gui.getPluginInfoList()

        for(i = 0, k = 0; i < info.length; ++i) {

            if (info[i].type !== "Application")
                continue

            appListModel.setProperty(k, "active", info[i].active)
            ++k
        }

        swipe.updateActiveChanged();
    }

    Component.onCompleted: {
        timer.start()
    }

    Timer {
        id: timer
        interval: 100
        running: false
        repeat: false
//        onTriggered: updateAppList()
        onTriggered: initialize()
    }
}
