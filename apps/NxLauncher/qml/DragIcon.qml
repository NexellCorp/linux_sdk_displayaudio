import QtQuick 2.0
import QtQuick.Controls 2.0

Item {
    id: root
    property string tag: "DragIcon.xml: "
    property string dragKey
    property alias mouseArea: mouseArea

    MouseArea {
        id: mouseArea
        objectName: "dropIconMouseArea"
        anchors.fill: parent

        drag.target: iconRect
        onReleased: {
            var  target = iconRect.Drag.target
            if(target!==null)
            {
                var itemList = target.children
                for(var i = 0; i<itemList.length; i++)
                {
                    if(itemList[i].objectName ===objectName)
                        return
                }
                parent =  target
            }
        }
        Rectangle {
            id: iconRect
            color:"transparent"
            anchors.fill: parent
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            Drag.keys: [ dragKey ]
            Drag.active: mouseArea.drag.active
            Drag.hotSpot.x: mouseArea.mouseX
            Drag.hotSpot.y: mouseArea.mouseY
                Image {
                    id: iconImage
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 5

                    height: parent.height - iconText.height - iconText.anchors.topMargin*2
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    source: {
                        if (path.length === 0 || iconRect.length === 0) {
                            return "qrc:/PodonamuUI/images/noicon.png"
                        }
                        var iconPath = "file:/" + path + "/" + icon
//                        console.log( tag + "iconPath: ", iconPath)
                        return iconPath
                    }
                }
                Text {
                    id: iconText
                    text: name
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.margins: 5
                    horizontalAlignment :  Text.AlignHCenter
                }
            states: State {
                when: mouseArea.drag.active
            }
        }
    }
}
