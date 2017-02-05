import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page
    allowedOrientations: Orientation.All
    Component.onCompleted: {
        loadPossibleSettings()
    }

    property string plugin_type
    property string display_header_text

    function loadPossibleSettings() {
        var plugin_name = settingsManager.getCurrentPlugin(plugin_type)
        console.log("###", "plugin name " + plugin_name)
        var settings = settingsManager.getPossibleSettings(plugin_type, plugin_name)
        var general_settings = settingsManager.getPossibleGeneralSettings(plugin_type)

        for(var setting in general_settings) {
            console.log('###', "possible general setting: " + general_settings[setting] + " " + settingsManager.getGeneralSettingType(plugin_type, general_settings[setting]) + " " + settingsManager.getGeneralSettingValue(plugin_type, plugin_name, general_settings[setting]))

            listModel.append({setting_name: general_settings[setting], section: "General", setting_value: settingsManager.getGeneralSettingValue(plugin_type, plugin_name, general_settings[setting]), setting_hint: settingsManager.getGeneralSettingHint(plugin_type, plugin_name, general_settings[setting])})
        }

        for(var setting in settings) {
            console.log('###', "possible setting: " + settings[setting] + " " + settingsManager.getSettingType(plugin_type, plugin_name, settings[setting]) + " " + settingsManager.getSettingValue(plugin_type, plugin_name, settings[setting]))

            listModel.append({setting_name: settings[setting], section: plugin_name, setting_value: settingsManager.getSettingValue(plugin_type, plugin_name, settings[setting]), setting_hint: settingsManager.getSettingHint(plugin_type, plugin_name, settings[setting])})
        }
    }

    SilicaListView {
        width: parent.width
        height: parent.height

        section {
            property: "section"
            criteria: ViewSection.FullString
            delegate: SectionHeader {
                    text: section
                }
        }

        header: PageHeader {
                    title: display_header_text
                }

        model: ListModel {
            id: listModel
        }

        delegate: ListItem {
            id: listItem

            anchors {
                margins: Theme.paddingLarge
            }

            Row {
                spacing: Theme.paddingMedium

                TextField {
                    width: parent.width
                    label: setting_name
                    text: setting_value
                    labelVisible: true
                    placeholderText: setting_hint
                    color: listItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                    anchors.verticalCenter: parent.verticalCenter
                    readOnly: false
                    onActiveFocusChanged: {
                        console.log('###', 'saving setting ' + setting_name + " " + text)
                        if(section == "General")
                        {
                            settingsManager.saveGeneralSetting(plugin_type, setting_name, text)
                        }
                        else
                        {
                            settingsManager.saveSetting(plugin_type, settingsManager.getCurrentPlugin(plugin_type), setting_name, text)
                        }
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
        }
    }
}
