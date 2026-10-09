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
                    spacing: 8
                    Canvas {
                        id: clearCanvas
                        width: 12; height: 12
                        anchors.verticalCenter: parent.verticalCenter
                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.reset();
                            ctx.strokeStyle = clearMouse.containsMouse ? Theme.textPrimary : Theme.textSecondary;
                            ctx.lineWidth = 1.5;
                            ctx.lineCap = "round";
                            ctx.beginPath();
                            ctx.moveTo(2, 2); ctx.lineTo(10, 10);
                            ctx.moveTo(10, 2); ctx.lineTo(2, 10);
                            ctx.stroke();
                        }
                        Connections {
                            target: clearMouse
                            function onContainsMouseChanged() { clearCanvas.requestPaint(); }
                        }
                    }
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
                    border.color: Theme.borderDim
                    border.width: 1
                    anchors.horizontalCenter: parent.horizontalCenter

                    Canvas {
                        anchors.centerIn: parent
                        width: 24; height: 24
                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.reset();
                            ctx.strokeStyle = Theme.accent;
                            ctx.fillStyle = Theme.accent;
                            ctx.lineWidth = 2;
                            ctx.lineCap = "round";
                            ctx.lineJoin = "round";

                            ctx.beginPath();
                            ctx.moveTo(12, 3); ctx.lineTo(12, 15);
                            ctx.moveTo(7, 10); ctx.lineTo(12, 15); ctx.lineTo(17, 10);
                            ctx.stroke();

                            ctx.beginPath();
                            ctx.moveTo(4, 15); ctx.lineTo(4, 20); ctx.lineTo(20, 20); ctx.lineTo(20, 15);
                            ctx.stroke();
                        }
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
