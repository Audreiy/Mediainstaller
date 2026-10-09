import QtQuick
import QtQuick.Controls
import MediaGrab

Item {
    id: root

    Column {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 20

        // Header Line
        Item {
            width: parent.width
            height: 44

            Column {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4
                Text {
                    text: "Download Queue"
                    color: Theme.textPrimary
                    font.pixelSize: 22
                    font.family: Theme.fontFamily
                    font.weight: Font.Bold
                }
                Text {
                    text: queueModel.activeDownloadCount + " active • Total " + queueModel.totalDownloadCount + " items"
                    color: Theme.textSecondary
                    font.pixelSize: 12
                    font.family: Theme.fontFamily
                }
            }

            // Clear Completed Button
            Rectangle {
                visible: queueModel.totalDownloadCount > 0
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: 140
                height: 38
                radius: Theme.radiusMd
                color: clearMouse.containsMouse ? Theme.bgCardHover : Theme.bgElevated
                border.color: clearMouse.containsMouse ? Theme.accent : Theme.borderDim
                border.width: 1

                MouseArea {
                    id: clearMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        queueModel.clearCompleted();
                    }
                }

                Row {
                    anchors.centerIn: parent
                    spacing: 6
                    Text { text: "🧹"; font.pixelSize: 13 }
                    Text {
                        text: "Clear Finished"
                        color: clearMouse.containsMouse ? Theme.textPrimary : Theme.textSecondary
                        font.pixelSize: 12
                        font.family: Theme.fontFamily
                        font.weight: Font.Medium
                    }
                }
            }
        }

        // Empty state
        Rectangle {
            visible: queueModel.totalDownloadCount === 0
            width: parent.width
            height: 280
            radius: Theme.radiusLg
            color: Theme.bgCard
            border.color: Theme.borderDim
            border.width: 1

            Column {
                anchors.centerIn: parent
                spacing: 12

                Rectangle {
                    width: 56
                    height: 56
                    radius: 28
                    color: Theme.bgElevated
                    anchors.horizontalCenter: parent.horizontalCenter

                    Text {
                        anchors.centerIn: parent
                        text: "📥"
                        font.pixelSize: 26
                    }
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "No Downloads in Queue"
                    color: Theme.textPrimary
                    font.pixelSize: 15
                    font.family: Theme.fontFamily
                    font.weight: Font.DemiBold
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Paste a media link in the Downloader tab to begin."
                    color: Theme.textMuted
                    font.pixelSize: 12
                    font.family: Theme.fontFamily
                }
            }
        }

        // Active List
        ListView {
            visible: queueModel.totalDownloadCount > 0
            width: parent.width
            height: parent.height - 80
            model: queueModel
            spacing: 10
            clip: true

            delegate: DownloadCard {
                width: parent.width
            }
        }
    }
}
