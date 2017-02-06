import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    allowedOrientations: Orientation.All
    id: page

    SilicaListView {
        id: listView
        width: parent.width
        height: parent.height

        header: PageHeader {
                    title: "Configure Plugins"
                }

        model: ListModel {
            id: listModel

            ListElement {
                display_text: "Video Plugin"
                configure_page: "ConfigureVideo.qml"
                plugin_type: "video"
                show_menu: 1
            }

            ListElement {
                display_text: "Audio Plugin"
                configure_page: "ConfigureAudio.qml"
                plugin_type: "audio"
                show_menu: 1
            }

            ListElement {
                display_text: "Input Plugin"
                configure_page: "ConfigureInput.qml"
                plugin_type: "input"
                show_menu: 1
            }

            ListElement {
                display_text: "Rsp Plugin"
                configure_page: "ConfigureRsp.qml"
                plugin_type: "rsp"
                show_menu: 1
            }

            ListElement {
                display_text: "Core"
                configure_page: "ConfigureCore.qml"
                plugin_type: "core"
                show_menu: 0
            }
        }

        delegate: ListItem {
            id: listItem

            Row {
                spacing: Theme.paddingMedium

                Label {
                    text: display_text + ": "
                    anchors.verticalCenter: parent.verticalCenter
                }

                Label {
                    id: selectedPlugin
                    anchors.verticalCenter: parent.verticalCenter

                    text: {
                        var plug = settingsManager.getCurrentPlugin(plugin_type)
                        return plug;
                    }
                }

                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
            }

            showMenuOnPressAndHold: show_menu

            menu: ContextMenu {
                id: context_menu
                Repeater {
                    model: settingsManager.pluginCount(plugin_type)

                    MenuItem {
                        text: settingsManager.getPlugin(plugin_type, model.index)

                        onClicked: {
                            settingsManager.selectPlugin(plugin_type, model.index)

                            selectedPlugin.text = settingsManager.getCurrentPlugin(plugin_type)
                        }
                    }
                }
            }

            onClicked: {
                pageStack.push(Qt.resolvedUrl(configure_page))
            }
        }
    }

    InteractionHintLabel {
        id: interactionHint
        text: "Long press on a plugin type to select a plugin, short press to configure the plugin. If no config options are available start the emulator once."
        visible: timer.running
        anchors.bottom: page.bottom
    }

    Timer {
        id: timer
        interval: 10000
        repeat: false
        running: true
    }

    Component.onDestruction: {
        settingsManager.unloadSettings()
    }
}
