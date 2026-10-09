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
                    text: I18n.t("queueTitle")
                    color: Theme.textPrimary
                    font.pixelSize: 22
                    font.family: Theme.fontFamily
                    font.weight: Font.Bold
                }
                Text {
                    text: I18n.t("queueSubtitle").arg(queueModel.activeDownloadCount).arg(queueModel.totalDownloadCount)
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
                        text: I18n.t("clearFinished")
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
                    text: I18n.t("noDownloads")
                    color: Theme.textPrimary
                    font.pixelSize: 15
                    font.family: Theme.fontFamily
                    font.weight: Font.DemiBold
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: I18n.t("noDownloadsHint")
                    color: Theme.textMuted
                    font.pixelSize: 12
                    font.family: Theme.fontFamily
                }
            }
        }

        // Download List
        ListView {
            visible: queueModel.totalDownloadCount > 0
            width: parent.width
            height: parent.height - 70
            clip: true
            spacing: 12
            model: queueModel

            delegate: DownloadCard {
                width: ListView.view.width
            }
        }
    }
}
