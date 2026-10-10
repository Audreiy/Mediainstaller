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

                // 1. Download Mode Segmented Pill (All at Once | Sequential)
                Rectangle {
                    height: 36
                    width: modeRow.width + 8
                    radius: Theme.radiusMd
                    color: Theme.bgElevated
                    border.color: Theme.borderDim
                    border.width: 1

                    Row {
                        id: modeRow
                        anchors.centerIn: parent
                        spacing: 4

                        // Option 1: All at Once
                        Rectangle {
                            height: 28
                            width: concurrentContent.width + 20
                            radius: Theme.radiusSm
                            color: !downloadManager.sequentialMode ? (Theme.accent + "26") : (concurrentHover.containsMouse ? Theme.bgCardHover : "transparent")
                            border.color: !downloadManager.sequentialMode ? Theme.accent : "transparent"
                            border.width: 1

                            Behavior on color { ColorAnimation { duration: 150 } }
                            Behavior on border.color { ColorAnimation { duration: 150 } }

                            MouseArea {
                                id: concurrentHover
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: downloadManager.setSequentialMode(false)
                            }

                            Row {
                                id: concurrentContent
                                anchors.centerIn: parent

                                Text {
                                    text: I18n.t("downloadModeConcurrent")
                                    color: !downloadManager.sequentialMode ? Theme.textPrimary : Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    font.weight: !downloadManager.sequentialMode ? Font.Bold : Font.DemiBold
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                        }

                        // Option 2: Sequential
                        Rectangle {
                            height: 28
                            width: seqContent.width + 20
                            radius: Theme.radiusSm
                            color: downloadManager.sequentialMode ? (Theme.accent + "26") : (seqHover.containsMouse ? Theme.bgCardHover : "transparent")
                            border.color: downloadManager.sequentialMode ? Theme.accent : "transparent"
                            border.width: 1

                            Behavior on color { ColorAnimation { duration: 150 } }
                            Behavior on border.color { ColorAnimation { duration: 150 } }

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
                                    color: downloadManager.sequentialMode ? Theme.textPrimary : Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    font.weight: downloadManager.sequentialMode ? Font.Bold : Font.DemiBold
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                        }
                    }
                }

                // 2. Dedicated Speed Limit Controller (HUD Glow Card with Toggle & Stepper)
                Rectangle {
                    id: limitCard
                    property bool isActive: downloadManager.speedLimitEnabled
                    height: 36
                    width: limitMainRow.width + 16
                    radius: Theme.radiusMd
                    color: isActive ? "#162332" : Theme.bgElevated
                    border.color: isActive ? Theme.accent : (limitCardMouse.containsMouse ? Theme.borderBright : Theme.borderDim)
                    border.width: 1

                    Behavior on color { ColorAnimation { duration: 180 } }
                    Behavior on border.color { ColorAnimation { duration: 180 } }

                    Row {
                        id: limitMainRow
                        anchors.centerIn: parent
                        spacing: 8

                        // Clickable Toggle Section: [Pulse Dot] Speed Limit [ON/OFF]
                        Item {
                            id: toggleAreaItem
                            width: toggleContentRow.width
                            height: 28
                            anchors.verticalCenter: parent.verticalCenter

                            MouseArea {
                                id: limitCardMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: downloadManager.setSpeedLimitEnabled(!downloadManager.speedLimitEnabled)
                            }

                            Row {
                                id: toggleContentRow
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 7

                                // Pulsing / Status LED Dot
                                Item {
                                    width: 10
                                    height: 10
                                    anchors.verticalCenter: parent.verticalCenter

                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 10; height: 10; radius: 5
                                        color: Theme.accent
                                        opacity: 0.35
                                        visible: limitCard.isActive
                                        SequentialAnimation on scale {
                                            running: limitCard.isActive
                                            loops: Animation.Infinite
                                            NumberAnimation { from: 0.8; to: 1.4; duration: 1000; easing.type: Easing.InOutQuad }
                                            NumberAnimation { from: 1.4; to: 0.8; duration: 1000; easing.type: Easing.InOutQuad }
                                        }
                                    }

                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 6; height: 6; radius: 3
                                        color: limitCard.isActive ? Theme.accent : Theme.textMuted
                                    }
                                }

                                Text {
                                    text: I18n.t("downloadModeLimit")
                                    color: limitCard.isActive ? Theme.textPrimary : Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    font.weight: limitCard.isActive ? Font.Bold : Font.DemiBold
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                // Mini Status Badge (ON / OFF)
                                Rectangle {
                                    width: 24
                                    height: 16
                                    radius: 4
                                    color: limitCard.isActive ? (Theme.accent + "33") : Theme.borderDim
                                    border.color: limitCard.isActive ? (Theme.accent + "66") : "transparent"
                                    border.width: 1
                                    anchors.verticalCenter: parent.verticalCenter

                                    Text {
                                        anchors.centerIn: parent
                                        text: limitCard.isActive ? "ON" : "OFF"
                                        color: limitCard.isActive ? Theme.accent : Theme.textMuted
                                        font.pixelSize: 8
                                        font.weight: Font.Bold
                                        font.family: Theme.fontFamily
                                    }
                                }
                            }
                        }

                        // Divider
                        Rectangle {
                            width: 1
                            height: 18
                            color: Theme.borderDim
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        // Stepper Controls [-] [ N MB/s ] [+]
                        Row {
                            id: speedStepperRow
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 4

                            readonly property var speedSteps: [1, 2, 3, 5, 8, 10, 15, 20, 30, 50]

                            function getNextSpeed(cur) {
                                for (var i = 0; i < speedSteps.length; i++) {
                                    if (speedSteps[i] > cur) return speedSteps[i];
                                }
                                return 50;
                            }

                            function getPrevSpeed(cur) {
                                for (var i = speedSteps.length - 1; i >= 0; i--) {
                                    if (speedSteps[i] < cur) return speedSteps[i];
                                }
                                return 1;
                            }

                            // Decrement Button [-]
                            Rectangle {
                                width: 22
                                height: 22
                                radius: 5
                                color: minusMouse.containsMouse ? Theme.bgCardHover : (limitCard.isActive ? "#1d2e40" : Theme.bgElevated)
                                border.color: minusMouse.containsMouse ? Theme.accent : Theme.borderDim
                                border.width: 1
                                anchors.verticalCenter: parent.verticalCenter

                                MouseArea {
                                    id: minusMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if (!downloadManager.speedLimitEnabled) {
                                            downloadManager.setSpeedLimitEnabled(true)
                                        }
                                        var cur = downloadManager.speedLimitMBps
                                        downloadManager.setSpeedLimitMBps(speedStepperRow.getPrevSpeed(cur))
                                    }
                                }

                                Text {
                                    anchors.centerIn: parent
                                    text: "−"
                                    color: limitCard.isActive ? Theme.textPrimary : Theme.textSecondary
                                    font.pixelSize: 12
                                    font.bold: true
                                }
                            }

                            // Stepper Value Display Pill: e.g. "2 MB/s"
                            Rectangle {
                                width: speedValText.implicitWidth + 14
                                height: 22
                                radius: 5
                                color: limitCard.isActive ? (Theme.accent + "2D") : Theme.bgElevated
                                border.color: limitCard.isActive ? (Theme.accent + "55") : Theme.borderDim
                                border.width: 1
                                anchors.verticalCenter: parent.verticalCenter

                                Text {
                                    id: speedValText
                                    anchors.centerIn: parent
                                    text: downloadManager.speedLimitMBps + " MB/s"
                                    color: limitCard.isActive ? Theme.accent : Theme.textSecondary
                                    font.pixelSize: 10
                                    font.family: Theme.fontFamily
                                    font.weight: Font.Bold
                                }
                            }

                            // Increment Button [+]
                            Rectangle {
                                width: 22
                                height: 22
                                radius: 5
                                color: plusMouse.containsMouse ? Theme.bgCardHover : (limitCard.isActive ? "#1d2e40" : Theme.bgElevated)
                                border.color: plusMouse.containsMouse ? Theme.accent : Theme.borderDim
                                border.width: 1
                                anchors.verticalCenter: parent.verticalCenter

                                MouseArea {
                                    id: plusMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if (!downloadManager.speedLimitEnabled) {
                                            downloadManager.setSpeedLimitEnabled(true)
                                        }
                                        var cur = downloadManager.speedLimitMBps
                                        downloadManager.setSpeedLimitMBps(speedStepperRow.getNextSpeed(cur))
                                    }
                                }

                                Text {
                                    anchors.centerIn: parent
                                    text: "+"
                                    color: limitCard.isActive ? Theme.textPrimary : Theme.textSecondary
                                    font.pixelSize: 12
                                    font.bold: true
                                }
                            }
                        }
                    }
                }

                // Clear Completed Button
                Rectangle {
                    visible: queueModel.totalDownloadCount > 0
                    width: 136
                    height: 36
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
                        Item {
                            width: 13; height: 13
                            anchors.verticalCenter: parent.verticalCenter
                            Canvas {
                                anchors.fill: parent
                                onPaint: {
                                    var ctx = getContext("2d");
                                    ctx.reset();
                                    ctx.strokeStyle = logHeaderMouse.containsMouse ? Theme.textPrimary : Theme.textSecondary;
                                    ctx.lineWidth = 1.2;
                                    ctx.strokeRect(1.5, 1.5, 10, 10);
                                    ctx.beginPath();
                                    ctx.moveTo(4, 5); ctx.lineTo(9, 5);
                                    ctx.moveTo(4, 8); ctx.lineTo(7, 8);
                                    ctx.stroke();
                                }
                            }
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
