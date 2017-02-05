import QtQuick 2.2
import Sailfish.Silica 1.0

ConfirmDialog {
    property Item errorMessage
    allowedOrientations: Orientation.All

    text: "Error " + errorMessage
    acceptDestination: Component {
        ConfigDialog {
            acceptDestination: gameSelector
            acceptDestinationAction: PageStackAction.Pop
        }
    }
}
