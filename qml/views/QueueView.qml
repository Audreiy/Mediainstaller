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

            // Header Controls: Mode Selector & Clear Completed
            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 10

                // Mode Selector: All at Once | Sequential | Limit (with Stepper)
                Rectangle {
                    height: 38
                    width: modeRow.width + 8
                    radius: Theme.radiusMd
                    color: Theme.bgElevated
                    border.color: Theme.borderDim
                    border.width: 1

                    Row {
                        id: modeRow
                        anchors.centerIn: parent
                        spacing: 4

                        // Mode 1: All at Once
                        Rectangle {
                            height: 30
                            width: concurrentContent.width + 18
                            radius: Theme.radiusSm
                            color: !downloadManager.downloadLimitEnabled ? (Theme.accent + "2D") : (concurrentHover.containsMouse ? Theme.bgCardHover : "transparent")
                            border.color: !downloadManager.downloadLimitEnabled ? Theme.accent : "transparent"
                            border.width: 1

                            Behavior on color { ColorAnimation { duration: 150 } }

                            MouseArea {
                                id: concurrentHover
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: downloadManager.setDownloadLimitEnabled(false)
                            }

                            Row {
                                id: concurrentContent
                                anchors.centerIn: parent
                                Text {
                                    text: I18n.t("downloadModeConcurrent")
                                    color: !downloadManager.downloadLimitEnabled ? Theme.textPrimary : Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    font.weight: !downloadManager.downloadLimitEnabled ? Font.Bold : Font.Normal
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                        }

                        // Mode 2: Sequential (1 by 1)
                        Rectangle {
                            height: 30
                            width: seqContent.width + 18
                            radius: Theme.radiusSm
                            color: (downloadManager.downloadLimitEnabled && downloadManager.maxConcurrentDownloads === 1) ? (Theme.accent + "2D") : (seqHover.containsMouse ? Theme.bgCardHover : "transparent")
                            border.color: (downloadManager.downloadLimitEnabled && downloadManager.maxConcurrentDownloads === 1) ? Theme.accent : "transparent"
                            border.width: 1

                            Behavior on color { ColorAnimation { duration: 150 } }

                            MouseArea {
                                id: seqHover
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: downloadManager.setSequentialMode(true)
                            }

                            Row {
                                id: seqContent
                                anchors.centerIn: parent
                                Text {
                                    text: I18n.t("downloadModeSequential")
                                    color: (downloadManager.downloadLimitEnabled && downloadManager.maxConcurrentDownloads === 1) ? Theme.textPrimary : Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    font.weight: (downloadManager.downloadLimitEnabled && downloadManager.maxConcurrentDownloads === 1) ? Font.Bold : Font.Normal
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                        }

                        // Mode 3: Custom Concurrent Limit (with Stepper)
                        Rectangle {
                            property bool isLimitActive: downloadManager.downloadLimitEnabled && downloadManager.maxConcurrentDownloads > 1
                            height: 30
                            width: limitRow.width + 14
                            radius: Theme.radiusSm
                            color: isLimitActive ? (Theme.accent + "2D") : (limitHover.containsMouse ? Theme.bgCardHover : "transparent")
                            border.color: isLimitActive ? Theme.accent : "transparent"
                            border.width: 1

                            Behavior on color { ColorAnimation { duration: 150 } }

                            MouseArea {
                                id: limitHover
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    downloadManager.setDownloadLimitEnabled(true)
                                    if (downloadManager.maxConcurrentDownloads <= 1) {
                                        downloadManager.setMaxConcurrentDownloads(2)
                                    }
                                }
                            }

                            Row {
                                id: limitRow
                                anchors.centerIn: parent
                                spacing: 8

                                Text {
                                    text: I18n.t("downloadModeLimit")
                                    color: (downloadManager.downloadLimitEnabled && downloadManager.maxConcurrentDownloads > 1) ? Theme.textPrimary : Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    font.weight: (downloadManager.downloadLimitEnabled && downloadManager.maxConcurrentDownloads > 1) ? Font.Bold : Font.Normal
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                Row {
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: 4

                                    // Decrement Button [-]
                                    Rectangle {
                                        width: 18
                                        height: 18
                                        radius: 4
                                        color: minusMouse.containsMouse ? Theme.bgCardHover : Theme.bgElevated
                                        border.color: minusMouse.containsMouse ? Theme.accent : Theme.borderDim
                                        border.width: 1
                                        anchors.verticalCenter: parent.verticalCenter

                                        MouseArea {
                                            id: minusMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                downloadManager.setDownloadLimitEnabled(true)
                                                var cur = downloadManager.maxConcurrentDownloads
                                                if (cur > 2) {
                                                    downloadManager.setMaxConcurrentDownloads(cur - 1)
                                                } else {
                                                    downloadManager.setMaxConcurrentDownloads(2)
                                                }
                                            }
                                        }

                                        Text {
                                            anchors.centerIn: parent
                                            text: "−"
                                            color: Theme.textPrimary
                                            font.pixelSize: 11
                                            font.bold: true
                                        }
                                    }

                                    // Number value pill
                                    Rectangle {
                                        width: 22
                                        height: 18
                                        radius: 4
                                        color: (downloadManager.downloadLimitEnabled && downloadManager.maxConcurrentDownloads > 1) ? (Theme.accent + "33") : Theme.bgElevated
                                        anchors.verticalCenter: parent.verticalCenter

                                        Text {
                                            anchors.centerIn: parent
                                            text: downloadManager.maxConcurrentDownloads > 1 ? downloadManager.maxConcurrentDownloads : 2
                                            color: (downloadManager.downloadLimitEnabled && downloadManager.maxConcurrentDownloads > 1) ? Theme.accent : Theme.textSecondary
                                            font.pixelSize: 11
                                            font.family: Theme.fontFamily
                                            font.weight: Font.Bold
                                        }
                                    }

                                    // Increment Button [+]
                                    Rectangle {
                                        width: 18
                                        height: 18
                                        radius: 4
                                        color: plusMouse.containsMouse ? Theme.bgCardHover : Theme.bgElevated
                                        border.color: plusMouse.containsMouse ? Theme.accent : Theme.borderDim
                                        border.width: 1
                                        anchors.verticalCenter: parent.verticalCenter

                                        MouseArea {
                                            id: plusMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                downloadManager.setDownloadLimitEnabled(true)
                                                var cur = downloadManager.maxConcurrentDownloads
                                                if (cur < 2) {
                                                    downloadManager.setMaxConcurrentDownloads(2)
                                                } else if (cur < 10) {
                                                    downloadManager.setMaxConcurrentDownloads(cur + 1)
                                                }
                                            }
                                        }

                                        Text {
                                            anchors.centerIn: parent
                                            text: "+"
                                            color: Theme.textPrimary
                                            font.pixelSize: 11
                                            font.bold: true
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Clear Completed Button
                Rectangle {
                    visible: queueModel.totalDownloadCount > 0
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

                // Error Diagnostics Log Button
                Rectangle {
                    width: 110
                    height: 38
                    radius: Theme.radiusMd
                    color: logHeaderMouse.containsMouse ? Theme.bgCardHover : Theme.bgElevated
                    border.color: logHeaderMouse.containsMouse ? Theme.accentCyan : Theme.borderDim
                    border.width: 1

                    MouseArea {
                        id: logHeaderMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (typeof downloadManager !== "undefined" && downloadManager) {
                                downloadManager.openLogFile();
                            }
                        }
                    }

                    ToolTip.visible: logHeaderMouse.containsMouse
                    ToolTip.text: I18n.t("viewLogHint")
                    ToolTip.delay: 300

                    Row {
                        anchors.centerIn: parent
                        spacing: 6
                        Text {
                            text: "📄"
                            font.pixelSize: 12
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: I18n.t("openLogFile")
                            color: logHeaderMouse.containsMouse ? Theme.textPrimary : Theme.textSecondary
                            font.pixelSize: 12
                            font.family: Theme.fontFamily
                            font.weight: Font.Medium
                            anchors.verticalCenter: parent.verticalCenter
                        }
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
