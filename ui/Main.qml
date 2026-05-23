    import QtQuick
    import "./components"
    import QtQuick.Controls 2.15
    import QtQuick.Controls.Material
    import com.roboticus.DataVisualiser

    Window {
        id: window
        visibility: Window.Maximized
        minimumWidth: 950
        minimumHeight: 480
        title: "Roboticus Data Visualiser"
        color: "#1a1a1a"

        Material.theme: Material.Dark
        Material.accent: "#98FF98"

        AppController {
            id: appController
            Component.onCompleted: setModels(sensorController.model, vectorController.model)


            onErrorOccurred: function (message) {
                errorPopup.show(message);
            }

            onSnapshotsChanged: function() {
                timelineBar.updateTimelineProps();
            }
        }

        Connections {
            target: appController.portManager

            function onConnectionChanged() {
                if (appController.portManager.isConnected) {
                    sensorController.setActiveLayer(monitor.selectedSensorLayer)
                    vectorController.setActiveLayer(monitor.selectedVectorLayer)
                    timelineBar.currentFrame = 0;
                } else {
                    timelineBar.currentFrame = timelineBar.maxFrames;
                }
            }
        }

        SensorController {
            id: sensorController


            onClearChartSeries: function () {
                graph.clearSensorsOnGraph();
            }

            onErrorOccurred: function (message) {
                errorPopup.show(message);
            }



            onSensorAdded: function(id, name, input, threshold, isTriggered, layer, x, y) {
                if (layer.toLowerCase() === monitor.selectedSensorLayer.toLowerCase()) {
                    graph.addSensorToGraph(name, input, threshold, isTriggered, x, y)
                }
            }

            onSensorUpdated: function(id, name, input, threshold, isTriggered, layer, x, y) {
                if (layer.toLowerCase() === monitor.selectedSensorLayer.toLowerCase()) {
                    graph.updateSensorOnGraph(name, input, threshold, isTriggered, x, y)
                }
            }
        }

        VectorController {
            id: vectorController

            onClearChartSeries: function () {
                graph.clearVectorsOnGraph();
            }

            onErrorOccurred: function (message) {
                errorPopup.show(message);
            }


            onVectorAdded: function(id, name, rotation, scale, color, layer, x, y) {
                if (layer.toLowerCase() === monitor.selectedVectorLayer.toLowerCase()) {
                    graph.addVectorToGraph(name, rotation, scale, color, x, y)
                }
            }

            onVectorUpdated: function(id, name, rotation, scale, color, layer, x, y) {
                if (layer.toLowerCase() === monitor.selectedVectorLayer.toLowerCase()) {
                    graph.updateVectorOnGraph(name, rotation, scale, color, x, y)
                }
            }
        }


        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: "#1a1a1a"
                }
                GradientStop {
                    position: 1.0
                    color: "#0f0f0f"
                }
            }
            opacity: 0.8
        }

        Graph {
            id: graph
        }

        ConnectionBar {
            id: title

            appController: appController
            portManager: appController.portManager
        }

        Monitor {
            id: monitor

            sensorController: sensorController
            vectorController: vectorController
        }

        Timeline {
            id: timelineBar

            appController: appController

            onCurrentFrameChanged: {
                graph.clearGraph()
                appController.restoreToIndex(timelineBar.currentFrame)
                timelineBar.updateTimelineProps()
            }
        }

        Error {
            id: errorPopup
        }

    }
