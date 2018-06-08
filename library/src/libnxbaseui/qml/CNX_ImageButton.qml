import QtQuick 2.0

Item {
    id: imageButton

    property alias enabled: mouseArea.enabled
    property bool checkable: false
    property bool checked: false
    property alias hover: mouseArea.containsMouse
    property alias pressed: mouseArea.pressed
    property alias hit: hit;

    property url imageSource
    property url imageSourceHover
    property url imageSourcePressed

    signal clicked

    width: image.width
    height: image.height

    Image {
        id: image
        source: imageSource
        anchors.verticalCenter: parent.verticalCenter
        visible: true
    }

    Rectangle {
        id: hit
        x: 0; y: 0;
        width: parent.width; height: parent.height;
        opacity: 0.0

        MouseArea {
            id: mouseArea
            hoverEnabled: true
            anchors.fill: parent

            onPressed: {
//                console.debug("onPressed")
                image.source= imageSourcePressed
            }

            onReleased: {
//                console.debug("onReleased")
                if( containsMouse == true )
                    image.source= imageSourceHover
                else
                    image.source= imageSource
            }

            onEntered: {
//                console.debug("onEntered")
                image.source= imageSourceHover
            }

            onExited: {
//                console.debug("onExited")
                image.source= imageSource
            }

            onClicked: {
                console.debug("onClicked")
//                If the system don't have mouse,
//                    this routine is needed.
                image.source= imageSource
                imageButton.clicked()
            }
        }
    }
}
