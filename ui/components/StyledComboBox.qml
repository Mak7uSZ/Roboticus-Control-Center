import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

ComboBox {
    id: root

    Material.accent: "#98FF98"
    Material.foreground: "#98FF98"
    leftPadding: 14
    rightPadding: 32

    contentItem: Text {
        text: root.displayText
        color: "#98FF98"
        font: root.font
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        clip: true
    }

    background: Rectangle {
        color: "#0f0f0f"
        border.width: 2
        border.color: root.hovered ? "#98FF98" : "#333333"
        radius: 4

        Behavior on border.color {
            ColorAnimation { duration: 150 }
        }
    }

    popup: Popup {
        y: root.height
        width: root.width
        height: implicitHeight
        padding: 3

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex

            ScrollIndicator.vertical: ScrollIndicator {}
        }

        background: Rectangle {
            color: "#1a1a1a"
            border.color: "#98FF98"
            border.width: 2
            radius: 4
        }
    }

    delegate: ItemDelegate {
        width: root.width
        hoverEnabled: true
        highlighted: root.highlightedIndex === index

        contentItem: Text {
            text: modelData
            color: parent.highlighted || parent.hovered ? "#98FF98" : "#888888"
        }

        background: Rectangle {
            color: parent.highlighted || parent.hovered ? "#0f0f0f" : "transparent"

            Behavior on color {
                ColorAnimation { duration: 100 }
            }
        }
    }
}
