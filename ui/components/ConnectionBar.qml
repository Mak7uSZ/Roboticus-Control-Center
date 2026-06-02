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
    readonly property var udpConnection: appController.udpConnection
    readonly property bool wiredMode: appController.connectionMode === AppController.Wired
    readonly property int rowMargin: 10
    readonly property int controlRowHeight: 34
    readonly property int controlSpacing: 12
    readonly property int primaryControlWidth: 180
    readonly property int secondaryControlWidth: 120
    readonly property int monitorControlWidth: 220
    readonly property color accentColor: "#98FF98"
    readonly property color darkTextColor: "#0f0f0f"
    readonly property color controlBackgroundColor: "#0f0f0f"
    readonly property color controlBorderColor: "#333333"

    anchors {
        left: monitor.left
        right: monitor.right
        leftMargin: 20
        rightMargin: 20
    }

    clip: true

    component ModeButton: Button {
        id: modeButton

        required property bool selected

        Layout.preferredHeight: connectionBar.controlRowHeight
        Material.roundedScale: Button.None
        Material.elevation: hovered ? 2 : 0

        contentItem: Text {
            text: modeButton.text
            color: modeButton.selected ? connectionBar.darkTextColor : "#ffffff"
            font.bold: true
            font.pixelSize: 13
            minimumPixelSize: 8
            fontSizeMode: Text.Fit
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            color: modeButton.selected ? connectionBar.accentColor : connectionBar.controlBackgroundColor
            border.width: 2
            border.color: modeButton.selected || modeButton.hovered ? connectionBar.accentColor : connectionBar.controlBorderColor
            radius: 4

            Rectangle {
                anchors.fill: parent
                color: modeButton.selected ? "#ffffff" : connectionBar.accentColor
                radius: 4
                opacity: modeButton.down ? 0.16 : modeButton.hovered && !modeButton.selected ? 0.08 : 0
                Behavior on opacity {
                    NumberAnimation { duration: 100 }
                }
            }

            Behavior on color {
                ColorAnimation { duration: 120 }
            }
            Behavior on border.color {
                ColorAnimation { duration: 120 }
            }
        }

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }
    }

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
            leftMargin: connectionBar.rowMargin
            rightMargin: connectionBar.rowMargin
        }
        height: connectionBar.controlRowHeight
        spacing: connectionBar.controlSpacing

        ModeButton {
            text: "Wired"
            selected: connectionBar.wiredMode
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            Layout.minimumWidth: 72
            onClicked: appController.switchToWiredMode()
        }

        ModeButton {
            text: "Wireless"
            selected: !connectionBar.wiredMode
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            Layout.minimumWidth: 84
            onClicked: appController.switchToWirelessMode()
        }
    }

    // Wireless controls
    RowLayout {
        id: wirelessControlsRow
        visible: !connectionBar.wiredMode
        enabled: !connectionBar.wiredMode
        anchors {
            left: parent.left
            right: parent.right
            top: modeRow.bottom
            topMargin: 8
            leftMargin: connectionBar.rowMargin
            rightMargin: connectionBar.rowMargin
        }
        height: connectionBar.controlRowHeight
        spacing: connectionBar.controlSpacing

        Rectangle {
            id: udpPortFieldContainer
            Layout.fillWidth: true
            Layout.preferredWidth: connectionBar.primaryControlWidth
            Layout.minimumWidth: 80
            Layout.preferredHeight: wirelessControlsRow.height
            color: connectionBar.controlBackgroundColor
            border.width: 2
            border.color: udpPortField.activeFocus || containerHover.hovered
                          ? connectionBar.accentColor
                          : connectionBar.controlBorderColor
            radius: 4

            Behavior on border.color {
                ColorAnimation { duration: 150 }
            }

            HoverHandler {
                id: containerHover
            }

            Text {
                visible: udpPortField.text.length === 0 && !udpPortField.activeFocus
                text: "UDP port"
                color: "#777777"
                font.pixelSize: 13
                elide: Text.ElideRight
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    leftMargin: 20
                    rightMargin: 12
                }
            }

            TextInput {
                id: udpPortField
                text: "8080"
                enabled: !udpConnection.listening
                activeFocusOnPress: true
                selectByMouse: true
                validator: RegularExpressionValidator { regularExpression: /^[0-9]{1,5}$/ } // only allow integers
                color: connectionBar.accentColor
                selectionColor: connectionBar.accentColor
                selectedTextColor: connectionBar.darkTextColor
                font.pixelSize: 13
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    leftMargin: 20
                    rightMargin: 12
                }
                onAccepted: {
                    if (!udpConnection.listening)
                        appController.startWirelessMonitor(udpPortField.text)
                }
            }
        }

        Button {
            id: wirelessMonitorButton
            Layout.fillWidth: true
            Layout.preferredWidth: connectionBar.monitorControlWidth
            Layout.minimumWidth: 120
            Layout.preferredHeight: wirelessControlsRow.height
            Layout.alignment: Qt.AlignVCenter

            Material.accent: connectionBar.accentColor
            Material.foreground: connectionBar.accentColor
            Material.roundedScale: Button.None
            Material.elevation: wirelessMonitorButton.hovered ? 3 : 1

            Text {
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    leftMargin: 10
                    rightMargin: 10
                }
                color: connectionBar.accentColor
                text: udpConnection.listening ? "Stop Wireless Monitor" : "Start Wireless Monitor"
                font.bold: true
                font.pixelSize: 13
                minimumPixelSize: 8
                fontSizeMode: Text.Fit
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                anchors.fill: parent
                color: connectionBar.controlBackgroundColor
                border.width: 2
                border.color: wirelessMonitorButton.hovered ? connectionBar.accentColor : connectionBar.controlBorderColor
                radius: 4

                Rectangle {
                    anchors.fill: parent
                    color: "#1a1a1a"
                    radius: 4
                    opacity: wirelessMonitorButton.down ? 0.2 : 0
                    Behavior on opacity {
                        NumberAnimation { duration: 100 }
                    }
                }
            }

            onClicked: {
                if (udpConnection.listening) {
                    appController.stopWirelessMonitor()
                } else {
                    appController.startWirelessMonitor(udpPortField.text)
                }
            }

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }

        Text {
            text: udpConnection.listening ? "Listening" : "Stopped"
            color: udpConnection.listening ? connectionBar.accentColor : "#aaaaaa"
            font.bold: true
            font.pixelSize: 13
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            Layout.fillWidth: true
            Layout.minimumWidth: 58
            Layout.preferredWidth: 90
            Layout.alignment: Qt.AlignVCenter
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
            leftMargin: connectionBar.rowMargin
            rightMargin: connectionBar.rowMargin
        }
        height: connectionBar.controlRowHeight
        spacing: connectionBar.controlSpacing

        StyledComboBox {
            id: comSelection
            Layout.fillWidth: true
            Layout.preferredWidth: connectionBar.primaryControlWidth
            Layout.minimumWidth: 88
            Layout.preferredHeight: serialControlsRow.height
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
            Layout.fillWidth: true
            Layout.preferredWidth: connectionBar.secondaryControlWidth
            Layout.minimumWidth: 68
            Layout.preferredHeight: serialControlsRow.height
            model: [9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600]
            currentIndex: 4

            onActivated: (index) => {
                portManager.setBaudRate(model[index])
                focus = false
            }
        }

        Button {
            id: startMonitor
            Layout.fillWidth: true
            Layout.preferredWidth: connectionBar.monitorControlWidth
            Layout.minimumWidth: 96
            Layout.preferredHeight: serialControlsRow.height
            Layout.alignment: Qt.AlignVCenter

            Material.accent: "#98FF98"
            Material.foreground: "#98FF98"
            Material.roundedScale: Button.None
            Material.elevation: startMonitor.hovered ? 3 : 1

            Text {
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    leftMargin: 10
                    rightMargin: 10
                }
                color: "#98FF98"
                text: portManager.isConnected ? "Stop Monitor" : "Start Monitor"
                font.bold: true
                font.pixelSize: 14
                minimumPixelSize: 8
                fontSizeMode: Text.Fit
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
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
