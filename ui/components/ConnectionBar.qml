import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts
import com.roboticus.DataVisualiser
import QtQuick.Controls.Material

Rectangle {
    id: connectionBar

    height: connectionBar.wiredMode || connectionBar.wirelessErrorMessage.length === 0 ? 92 : 116
    color: "transparent"

    required property var appController
    required property var portManager
    readonly property var udpConnection: appController.udpConnection
    readonly property bool wiredMode: appController.connectionMode === "wired"
    readonly property string wirelessErrorMessage: udpPortError
    readonly property int rowMargin: 10
    readonly property int controlRowHeight: 34
    readonly property int controlSpacing: 12
    readonly property int primaryControlWidth: 180
    readonly property color accentColor: "#98FF98"
    readonly property color darkTextColor: "#0f0f0f"
    readonly property color controlBackgroundColor: "#0f0f0f"
    readonly property color controlBorderColor: "#333333"
    property string udpPortError: ""

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

    function showUdpError(message) {
        udpPortError = message
        appController.reportConnectionError(message)
    }

    function startWirelessMonitor() {
        const portText = udpPortField.text.trim()

        if (portText.length === 0) {
            showUdpError("No UDP port entered. Please enter a UDP port to listen on.")
            return
        }

        if (!/^[0-9]+$/.test(portText)) {
            showUdpError("UDP port must contain only numbers. Remove letters or symbols.")
            return
        }

        const port = Number(portText)
        if (port <= 0 || port > 65535) {
            showUdpError("UDP port must be between 1 and 65535.")
            return
        }

        udpPortError = ""
        if (!appController.startWirelessMonitor(port)) {
            udpPortError = udpConnection.errorString.length > 0
                         ? udpConnection.errorString
                         : "Failed to start UDP listener."
        }
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
            Layout.preferredWidth: 100
            onClicked: appController.switchToWiredMode()
        }

        ModeButton {
            text: "Wireless"
            selected: !connectionBar.wiredMode
            Layout.preferredWidth: 110
            onClicked: appController.switchToWirelessMode()
        }

        Item {
            Layout.fillWidth: true
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

        TextField {
            id: udpPortField
            text: "45454"
            enabled: !udpConnection.listening
            selectByMouse: true
            inputMethodHints: Qt.ImhDigitsOnly
            verticalAlignment: TextInput.AlignVCenter
            color: connectionBar.accentColor
            selectionColor: connectionBar.accentColor
            selectedTextColor: connectionBar.darkTextColor
            leftPadding: 20
            rightPadding: 12
            topPadding: 0
            bottomPadding: 0
            Layout.preferredWidth: connectionBar.primaryControlWidth
            Layout.minimumWidth: 120
            Layout.preferredHeight: wirelessControlsRow.height
            onTextChanged: udpPortError = ""
            onAccepted: {
                if (!udpConnection.listening)
                    connectionBar.startWirelessMonitor()
            }

            Material.accent: connectionBar.accentColor
            Material.foreground: connectionBar.accentColor

            background: Rectangle {
                color: connectionBar.controlBackgroundColor
                border.width: 2
                border.color: udpPortField.hovered || udpPortField.activeFocus ? connectionBar.accentColor : connectionBar.controlBorderColor
                radius: 4

                Behavior on border.color {
                    ColorAnimation { duration: 150 }
                }
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
                    leftMargin: udpPortField.leftPadding
                    rightMargin: udpPortField.rightPadding
                }
            }
        }

        Button {
            id: wirelessMonitorButton
            Layout.preferredWidth: 230
            Layout.minimumWidth: 190
            Layout.maximumWidth: 260
            Layout.preferredHeight: wirelessControlsRow.height
            Layout.alignment: Qt.AlignVCenter

            Material.accent: connectionBar.accentColor
            Material.foreground: connectionBar.accentColor
            Material.roundedScale: Button.None
            Material.elevation: wirelessMonitorButton.hovered ? 3 : 1

            Text {
                anchors.centerIn: parent
                color: connectionBar.accentColor
                text: udpConnection.listening ? "Stop Wireless Monitor" : "Start Wireless Monitor"
                font.bold: true
                font.pixelSize: 13
                minimumPixelSize: 8
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
                    udpPortError = ""
                } else {
                    connectionBar.startWirelessMonitor()
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
            Layout.minimumWidth: 100
            Layout.preferredWidth: 120
            Layout.alignment: Qt.AlignVCenter
        }
    }

    Text {
        visible: !connectionBar.wiredMode && connectionBar.wirelessErrorMessage.length > 0
        text: connectionBar.wirelessErrorMessage
        color: "#ff8a8a"
        font.pixelSize: 12
        elide: Text.ElideRight
        anchors {
            left: parent.left
            right: parent.right
            top: wirelessControlsRow.bottom
            topMargin: 4
            leftMargin: connectionBar.rowMargin
            rightMargin: connectionBar.rowMargin
        }
        height: visible ? 16 : 0
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
            Layout.preferredWidth: connectionBar.primaryControlWidth
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
