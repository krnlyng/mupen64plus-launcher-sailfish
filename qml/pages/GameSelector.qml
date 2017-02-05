/*
  Copyright (C) 2013 Jolla Ltd.
  Contact: Thomas Perl <thomas.perl@jollamobile.com>
  All rights reserved.

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Jolla Ltd nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

import QtQuick 2.0
import Sailfish.Silica 1.0
import RecentsHandler 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    Component.onCompleted: {
        initRecentGames()
    }

    function basename(str) {
        return (str.slice(str.lastIndexOf("/") + 1))
    }

    function initRecentGames() {
        console.log("###", "initializing recent games")
        var games = recentsHandler.getRecentGames();
        for(var game in games)
        {
            console.log("###", "adding recent game " + Qt.resolvedUrl(games[game]))
            addRecent(Qt.resolvedUrl(games[game]), basename(Qt.resolvedUrl(games[game]).toString()))
        }
    }

    function selectedROM(filename, basename) {
        console.log("###", filename)
        pageStack.pop()
        if(gameLauncher.launchGame(filename) != 0)
        {
            pageStack.push(Qt.resolvedUrl("Error.qml"), {errorMessage: "failed to launch the game"})
        }
        else
        {
            addRecent(filename, basename)
        }
    }

    function removeRecentROM(filename)
    {
        var found = false;
        console.log("###", typeof filename)

        for(var i=0;i<listModel.count;i++)
        {
            if(listModel.get(i).game_name == filename)
            {
                listModel.remove(i)
                break
            }
        }

        recentsHandler.removeRecentGame(filename)
    }

    function addRecent(filename, basename)
    {
        var found = false;
        console.log("###", typeof filename)

        for(var i=0;i<listModel.count;i++)
        {
            if(listModel.get(i).game_name == filename)
            {
                listModel.remove(i)
                break
            }
        }

        listModel.insert(2, {
                             display_text: basename,
                             function_to_call: "openRecentGame",
                             icon_to_use: "image://theme/icon-m-play",
                             section_to_use: "Recent",
                             game_name: filename,
                             menu_shown: 1
                             })

        recentsHandler.addRecentGame(filename)
    }

    function clickedSelectaROM() {
        pageStack.push(Qt.resolvedUrl("FilePicker.qml"), {
                homePath: "/home/nemo",
                showFormat: true,
                title: "Select a ROM",
                callback: selectedROM
            }
        );
    }

    function clickedConfigurePlugins() {
        settingsManager.loadSettings()
        pageStack.push(Qt.resolvedUrl("ConfigurePlugins.qml"))
    }

    SilicaListView {
        id: listView
        width: parent.width
        height: parent.height

        header: PageHeader {
                    title: "Mupen64plus Launcher"
                }

        section {
            property: "section_to_use"
            criteria: ViewSection.FullString
            delegate: SectionHeader {
                    text: section
                }
        }

        model: ListModel {
            id: listModel
            ListElement {
                display_text: "Select a ROM"
                function_to_call: "clickedSelectaROM"
                icon_to_use: "image://theme/icon-m-folder"
                section_to_use: "General"
                game_name: "none"
                menu_shown: 0
            }

            ListElement {
                display_text: "Configure Plugins"
                function_to_call: "clickedConfigurePlugins"
                icon_to_use: "image://theme/icon-m-developer-mode"
                section_to_use: "General"
                game_name: "none"
                menu_shown: 0
            }
        }

        delegate: ListItem {
            id: listItem

            anchors {
                margins: Theme.paddingMedium
            }

            Row {
                spacing: Theme.paddingMedium

                Image {
                    id: image
                    source: icon_to_use
                }

                Label {
                    text: display_text
                    color: listItem.highlighted ? Theme.highlightColor : Theme.primaryColor
                    anchors {
                        verticalCenter: image.verticalCenter
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

            showMenuOnPressAndHold: menu_shown

            onClicked: {
                if(function_to_call == "clickedSelectaROM")
                {
                    clickedSelectaROM();
                }
                else if(function_to_call == "clickedConfigurePlugins")
                {
                    clickedConfigurePlugins()
                }
                else if(function_to_call == "openRecentGame")
                {
                    selectedROM(game_name, display_text)
                }
            }

            menu: ContextMenu {
                id: context_menu

                MenuItem {
                    id: menu_item
                    text: "Remove"

                    onClicked: {
                        removeRecentROM(game_name)
                    }
                }
            }
        }

        VerticalScrollDecorator {}
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: gameLauncher.checkActive()
    }
    InfoLabel {
        id: infoLabel
        text: "Game is active"
        visible: false
    }

    onVisibleChanged: {
        console.log("###", 'visible changed')
        busyIndicator.running = gameLauncher.checkActive()
        infoLabel.visible = busyIndicator.running
        listView.visible = !busyIndicator.running
    }
}
