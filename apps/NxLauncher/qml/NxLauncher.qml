import QtQuick 2.0
import QtQuick.Controls 2.0

import "qrc:/qml/"

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

        function sort() {
            for(var i = 0; i < count; i++)
            {
                for(var j = 0; j < i; j++)
                {
                    if( get(i).name < get(j).name )
                    {
                        move( i, j, 1 )
                        break
                    }
                }
            }
        }
    }

    SwipeView {
        id: view
        currentIndex: currentPage
        anchors.fill: parent
        anchors.leftMargin: parent.width % iconCellWidth / 2

        Repeater {
            id: totalPageRepeater
            model: totalPage

            GridView {
                id: pageGridView
                cellWidth:  iconCellWidth
                cellHeight: iconCellHeight
                interactive: false
                clip: true
                model: maxColumnsInPage * maxRowsInPage

                delegate: DropIcon {
                    width:  Math.floor(iconCellWidth * 8 / 10)
                    height:  Math.floor(iconCellWidth * 8 / 10)
                    dragKey: "icon"

                    Component.onCompleted: {
                    }
                }
            }
        }
    }

    PageIndicator {
        count: view.count
        currentIndex: view.currentIndex
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }

    function updateAppList() {
        appListModel.clear()

        var info = packageManager.getAppInfoVariantList()
        for(var idx =0 ; idx < info.length; idx++) {
            var i = idx % info.length

            appListModel.append({
                "name": info[i].name,
                "path": info[i].path,
                "icon": info[i].icon,
                "exec": info[i].exec
            })

//            console.log(tag + "updateAppList():",
//                        "name: " + info[i].name + ",",
//                        "path: " + info[i].path + ",",
//                        "icon: " + info[i].icon + ",",
//                        "exec: " + info[i].exec )
        }
        appListModel.sort()

        maxColumnsInPage = Math.floor(pane.width / iconCellWidth)
        maxRowsInPage = Math.floor(pane.height / iconCellHeight)

        dragIconList.iconListUpdate()
    }

    Row {
        id: dragIconList
        anchors.left: parent.left
        anchors.bottom: parent.top

        Repeater {
            id:iconRepeater
            model: appListModel
            delegate: DragIcon {
                dragKey:  "icon"
                width: iconCellWidth
                height: iconCellHeight

                Connections {
                    target: mouseArea
                    onClicked: {
                        console.log( tag + "launch: " + path + "/" + exec )
                        launchProgram( path + "/" + exec )
                    }
                }
            }
        }

        function iconListUpdate()
        {
            for(var i =0 ; i < iconRepeater.count; i++) {
                var page = currentPage + Math.floor(i / (maxColumnsInPage * maxRowsInPage))
                var idx = i % (maxColumnsInPage * maxRowsInPage)
                var item = totalPageRepeater.itemAt(page).contentItem.children[idx]

                if( !item ) continue;

                iconRepeater.itemAt(i).mouseArea.parent = item
            }
        }
    }

    Component.onCompleted: {
//        console.log( tag + "Component.onCompleted" )
        timer.start()
    }

    Timer {
        id: timer
        interval: 100
        running: false
        repeat: false
        onTriggered: updateAppList()
    }
}
