import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

Button {
    id: root

    background: Rectangle {
        color: "#1a1a1a"
        radius: 8
        border.width: 1
        border.color: root.hovered ? "#98FF98" : "#333333"

        Behavior on border.color {
            ColorAnimation { duration: 150 }
        }

        // Hover glow
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "#98FF98"
            opacity: root.hovered ? 0.15 : 0
            Behavior on opacity {
                NumberAnimation { duration: 150 }
            }
        }

        // Pressed effect
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "#1a1a1a"
            opacity: root.down ? 0.2 : 0
            Behavior on opacity {
                NumberAnimation { duration: 100 }
            }
        }
    }

    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }
}