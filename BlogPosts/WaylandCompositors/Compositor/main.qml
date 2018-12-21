import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.3
import QtWayland.Compositor 1.3

WaylandCompositor {
    WaylandOutput {
        sizeFollowsWindow: true

        window: ApplicationWindow {
            id: mainWindow
            visible: true
            width: 1280
            height: 800

            Item {
                id: appContainer
                anchors.fill: parent
            }

            footer: ToolBar {
                height: 80

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    height: parent.height
                    spacing: 16

                    Repeater {
                        model: gAppMgr.topApps

                        RoundButton {
                            anchors.verticalCenter: parent.verticalCenter
                            height: isHome ? 75 : 60
                            width: height
                            radius: height / 2
                            palette.button: model.color
                            onClicked: {
                                if (isHome) {
                                    console.log("@@@ Clicked home button")
                                    var comp = Qt.createComponent("ApplicationSwitcher.qml")
                                    var item = comp.createObject(null, {
                                                                     "model": gAppMgr.runningApps
                                                                 })
                                    appContainer.children = item
                                    return
                                }
                                if (isRunning) {
                                    appContainer.children = applicationItem
                                }
                                else {
                                    isRunning = true
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    IviApplication {
        onIviSurfaceCreated: {
            var comp = Qt.createComponent("ApplicationItem.qml")
            var item = comp.createObject(null, {
                                             "shellSurface": iviSurface,
                                             "processId": iviSurface.iviId
                                         })
            appContainer.children = item
            gAppMgr.insertApplicationItem(iviSurface.iviId, item)
            iviSurface.sendConfigure(Qt.size(appContainer.width, appContainer.height))
        }
    }
}
