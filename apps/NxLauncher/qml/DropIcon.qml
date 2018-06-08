import QtQuick 2.0
import QtQuick.Controls 2.0

DropArea {
    id: dragTarget
    property string dragKey
    property alias dropProxy: dragTarget
    keys: [ dragKey ]

    Rectangle {
        id: dropRectangle

        anchors.fill: parent
        color: "transparent"
        states: [
            State {
                when: dragTarget.containsDrag
                PropertyChanges {
                    target: dropRectangle
                    color: "grey"
                    opacity: 0.3
                }
            }
        ]
    }
    onDropped: {
        console.log(drop.source)
    }
}
