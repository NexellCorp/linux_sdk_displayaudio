import QtQuick 2.0

import "qrc:/qml/"

Rectangle {
    id: swipeView
    width: 1024
    height: 540
    color: "transparent"

    property int currentIndex   : -1
    property int iconCellWidth  : 250
    property int iconCellHeight : 250
    property int totalPage      : 0
    property int maxColumnsInPage : 0
    property int maxRowsInPage    : 0

    // -1: rightSwiping
    //  0: no swiping
    //  1: leftSwiping
    property int swiping: 0

    MouseArea {
        id: mouseArea
        anchors.fill: parent;

        property int startX: 0
        property int startY: 0
        property var startTime;

        onPressed: {
            startTime = new Date();
            startX = mouseX;
            startY = mouseY;
        }

        onReleased: {
            var xDiff = startX - mouseX;
            var flicking = checkFlicking(xDiff);

            if (swiping === 1)
                onLeftSwipeDone(flicking);
            else if (swiping === -1)
                onRightSwipeDone(flicking);
        }

        onMouseXChanged: {
            var xDiff = startX - mouseX;
            if (xDiff > 0)
                onLeftSwipe(xDiff);
            else if (xDiff < 0)
                onRightSwipe(xDiff);
        }

        function checkFlicking(offset) {
            var currTime = new Date();
            var diffTime = currTime - startTime;

            if (diffTime < 120 && Math.abs(offset) > 30) {
                if (offset > 0)
                    onLeftSwipeDone(true);
                else if (offset < 0)
                    onRightSwipeDone(true);

                return true;
            }

            return false;
        }
    }

    Rectangle {
        id          : page
        anchors.fill: parent
        anchors.leftMargin: parent.width % iconCellWidth
        color: "transparent"

        Repeater {
            id      : totalPageRepeater
            model   : totalPage

            GridView {
                width       : parent.width
                height      : parent.height
                cellWidth   : iconCellWidth
                cellHeight  : iconCellHeight
                clip        : true
                interactive : false
                model       : maxColumnsInPage * maxRowsInPage

                delegate: DropIcon {
                    width   : Math.floor(iconCellWidth * 0.8)
                    height  : Math.floor(iconCellHeight * 0.8)
                    dragKey: "icon"
                }
            }
        }
    }



    Repeater {
        id: iconRepeater
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

    function updateAppList() {
        var tag = "updateAppList: "
        var i, k, p, item

        maxColumnsInPage = Math.floor(page.width / iconCellWidth)
        maxRowsInPage = Math.floor(page.height / iconCellHeight)
        totalPage = Math.ceil(appListModel.count / (maxColumnsInPage * maxRowsInPage))

        if (totalPage > 0) {
            totalPageRepeater.itemAt(0).x = 0
            totalPageRepeater.itemAt(0).y = 0

            for (k = 1; k < totalPage; ++k) {
                totalPageRepeater.itemAt(k).x = parent.width
                totalPageRepeater.itemAt(k).y = 0
            }
        }

        // set current index
        currentIndex = appListModel.count ? 0 : -1

        for (k = 0; k < appListModel.count; ++k) {
            p = Math.floor(k / (maxColumnsInPage * maxRowsInPage))
            i = k % (maxColumnsInPage * maxRowsInPage)
            item = totalPageRepeater.itemAt(p).contentItem.children[i]

            if (!item)
                continue

            iconRepeater.itemAt(k).x = item.x
            iconRepeater.itemAt(k).y = item.y
            iconRepeater.itemAt(k).mouseArea.parent = item
        }
    }

    function onLeftSwipe(offset) {
        swiping = 1;
        // moving next screen: 1 -> 2 page moving
        if (currentIndex < totalPage-1) {
            totalPageRepeater.itemAt(currentIndex).x = -offset
            totalPageRepeater.itemAt(currentIndex+1).x = width - offset
        }
    }

    function onRightSwipe(offset) {
        swiping = -1;
        // moving prev screen: 2 -> 1 page moving
        if (currentIndex > 0) {
            totalPageRepeater.itemAt(currentIndex-1).x = -width - offset
            totalPageRepeater.itemAt(currentIndex).x = -offset
        }
    }

    function onLeftSwipeDone(forceUpdate) {
        if (currentIndex >= totalPage-1)
            return;

        var next = currentIndex + 1;

        if (forceUpdate) {
            totalPageRepeater.itemAt(currentIndex).x = -width
            totalPageRepeater.itemAt(next).x = 0
            currentIndex = next;
        } else {
            var currWeight = width - Math.abs(totalPageRepeater.itemAt(currentIndex).x)
            var nextWeight = width - Math.abs(totalPageRepeater.itemAt(next).x)
            if (currWeight < nextWeight) {
                totalPageRepeater.itemAt(currentIndex).x = -width
                totalPageRepeater.itemAt(next).x = 0
                currentIndex = next;
            } else {
                totalPageRepeater.itemAt(currentIndex).x = 0
                totalPageRepeater.itemAt(next).x = width
            }
        }

        swiping = 0;
    }

    function onRightSwipeDone(forceUpdate) {
        if (currentIndex <= 0)
            return;

        var prev = currentIndex - 1

        if (forceUpdate) {
            totalPageRepeater.itemAt(currentIndex).x = -parent.width
            totalPageRepeater.itemAt(prev).x = 0;
            currentIndex = prev;
        } else {
            var currWeight = width - Math.abs(totalPageRepeater.itemAt(currentIndex).x)
            var prevWeight = width - Math.abs(totalPageRepeater.itemAt(prev).x)
            if (currWeight < prevWeight) {
                totalPageRepeater.itemAt(currentIndex).x = -width
                totalPageRepeater.itemAt(prev).x = 0
                currentIndex = prev;
            } else {
                totalPageRepeater.itemAt(currentIndex).x = 0;
                totalPageRepeater.itemAt(prev).x = -width
            }
        }

        swiping = 0;
    }
}
