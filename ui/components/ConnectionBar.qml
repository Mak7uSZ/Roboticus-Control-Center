import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts
import com.roboticus.DataVisualiser
import QtQuick.Controls.Material

Rectangle {
    id: connectionBar

    height: 92
    color: "transparent"

    required property var appController
    required property var portManager
    readonly property bool wiredMode: appController.connectionMode === "wired"

    anchors {
        left: monitor.left
        right: monitor.right
        leftMargin: 20
        rightMargin: 20
    }

    clip: true


    Component.onCompleted: {
        portManager.setBaudRate(baudSelection.model[baudSelection.currentIndex])
        if (portManager.availablePortsList.length > 0)
            portManager.setComPort(portManager.availablePortsList[0])
    }
    // Mode selection
    RowLayout {
        id: modeRow
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            topMargin: 6
            leftMargin: 10
            rightMargin: 10
        }
        height: 34
        spacing: 12

        Button {
            text: "Wired"
            highlighted: connectionBar.wiredMode
            Layout.preferredWidth: 100
            Layout.preferredHeight: modeRow.height
            onClicked: appController.switchToWiredMode()
        }

        Button {
            text: "Wireless"
            highlighted: !connectionBar.wiredMode
            Layout.preferredWidth: 110
            Layout.preferredHeight: modeRow.height
            onClicked: appController.switchToWirelessMode()
        }

        Item {
            Layout.fillWidth: true
        }
    }

    // Serial controls
    RowLayout {
        id: serialControlsRow
        visible: connectionBar.wiredMode
        enabled: connectionBar.wiredMode
        anchors {
            left: parent.left
            right: parent.right
            top: modeRow.bottom
            topMargin: 8
            leftMargin: 10
            rightMargin: 10
        }
        height: 34
        spacing: 12

        StyledComboBox {
            id: comSelection
            Layout.preferredWidth: 180
            Layout.minimumWidth: 120
            Layout.preferredHeight: serialControlsRow.height
            // Layout.fillWidth: true
            model: portManager.availablePortsList.length > 0 ? portManager.availablePortsList : ["No COM Port found"]
            currentIndex: 0

            Connections {
                target: portManager
                function onAvailablePortsChanged() {
                    if (portManager.availablePortsList.length > 0) {
                        comSelection.currentIndex = 0
                        portManager.setComPort(portManager.availablePortsList[0])
                    }
                }
            }

            onActivated: (index) => {
                portManager.setComPort(model[index])
                focus = false
            }
        }

        StyledComboBox {
            id: baudSelection
            Layout.preferredWidth: 120
            Layout.minimumWidth: 90
            Layout.preferredHeight: serialControlsRow.height
            // Layout.fillWidth: true
            model: [9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600]
            currentIndex: 4

            onActivated: (index) => {
                portManager.setBaudRate(model[index])
                focus = false
            }
        }

        Button {
            id: startMonitor
            Layout.preferredWidth: 170
            Layout.minimumWidth: 130
            // Layout.fillWidth: true
            Layout.preferredHeight: serialControlsRow.height
            Layout.alignment: Qt.AlignVCenter

            Material.accent: "#98FF98"
            Material.foreground: "#98FF98"
            Material.roundedScale: Button.None
            Material.elevation: startMonitor.hovered ? 3 : 1

            Text {
                anchors.centerIn: parent
                color: "#98FF98"
                text: portManager.isConnected ? "Stop Monitor" : "Start Monitor"
                font.bold: true
                font.pixelSize: 14
                minimumPixelSize: 8
            }

            background: Rectangle {
                anchors.fill: parent
                color: "#0f0f0f"
                border.width: 2
                border.color: startMonitor.hovered ? "#98FF98" : "#333333"
                radius: 4

                // Pressed effect
                Rectangle {
                    anchors.fill: parent
                    color: "#1a1a1a"
                    radius: 4
                    opacity: startMonitor.down ? 0.2 : 0
                    Behavior on opacity {
                        NumberAnimation { duration: 100 }
                    }
                }
            }

            onClicked: {
                if (portManager.isConnected)
                    portManager.disconnectPort()
                else
                    portManager.connectToPort()
            }

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }

    // Green line below to divide the connection bar
    Rectangle {
        width: parent.width - 20
        height: 2
        color: "#98FF98"
        opacity: 0.6
        anchors {
            left: parent.left
            bottom: parent.bottom
            leftMargin: 10
        }
    }
}
