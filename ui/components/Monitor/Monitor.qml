import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQml.Models
import com.roboticus.DataVisualiser

Flickable {
    id: flickable

    width: parent.width /2
    flickableDirection: Flickable.VerticalFlick
    contentWidth: width
    contentHeight: column.height
    clip: true
    boundsBehavior: Flickable.StopAtBounds

    anchors {
        left: parent.left
        top: title.bottom
        bottom: timelineBar.top
        bottomMargin: 20
        leftMargin: 20
        rightMargin: 20
    }

    required property var sensorController
    required property var vectorController

    property string selectedSensorLayer: sensorController.layers.length > 0 ? sensorController.layers[0] : "Layer 1"
    property string selectedVectorLayer: vectorController.layers.length > 0 ? vectorController.layers[0] : "Layer 1"

    ScrollBar.vertical: ScrollBar {
        id: verticalScrollBar
        policy: ScrollBar.AsNeeded
        width: 8
        visible: flickable.contentHeight > flickable.height

        contentItem: Rectangle {
            implicitWidth: 6
            radius: 3
            color: verticalScrollBar.pressed ? "#22FF98" : "#98FF98"
            opacity: verticalScrollBar.active ? 0.8 : 0.4
            Behavior on opacity {
                NumberAnimation { duration: 200 }
            }
        }
    }

    SortFilterProxyModel {
        id: filteredSensorModel
        model: sensorController.model
        filters: ValueFilter {
            roleName: "layer"
            value: flickable.selectedSensorLayer
        }
    }

    SortFilterProxyModel {
        id: filteredVectorModel
        model: vectorController.model
        filters: ValueFilter {
            roleName: "layer"
            value: flickable.selectedVectorLayer
        }
    }

    // Sensors | Vectors |
    Column {
        id: column
        width: parent.width
        spacing: 12

        property var selection: null // unused

        ItemListView {
            id: sensorList
            title: "Sensors"
            model: filteredSensorModel
            layers: sensorController.layers
            selectedLayer: flickable.selectedSensorLayer
            onLayerChanged: (layer) => {
                flickable.selectedSensorLayer = layer
                sensorController.setActiveLayer(layer)
            }

            delegate: SensorInfo {
                id: sensorDelegate
                width: ListView.view.width
                height: 60
                color: index % 2 === 0 ? "#1a1a1a" : "#151515"
                sensorName: model.name
                inputValue: model.inputValue
                thresholdValue: model.threshold
                isTriggered: model.isTriggered
                xLocation: model.x
                yLocation: model.y
                selected: column.selection === sensorDelegate
                onClicked: column.selection = column.selection === sensorDelegate ? null : sensorDelegate
            }
        }

        ItemListView {
            id: vectorList
            title: "Vectors"
            model: filteredVectorModel
            layers: vectorController.layers
            selectedLayer: flickable.selectedVectorLayer
            onLayerChanged: (layer) => {
                flickable.selectedVectorLayer = layer
                vectorController.setActiveLayer(layer)
            }

            delegate: VectorInfo {
                id: vectorDelegate
                width: ListView.view.width
                height: 60
                color: index % 2 === 0 ? "#1a1a1a" : "#151515"
                vectorName: model.name
                rotationValue: model.rotation
                vectorColor: model.color
                xLocation: model.x
                yLocation: model.y
                selected: column.selection === vectorDelegate
                onClicked: column.selection = column.selection === vectorDelegate ? null : vectorDelegate
            }
        }
    }
}