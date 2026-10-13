import QtQuick
import QtQuick.Controls
import MediaGrab

Rectangle {
    id: root
    implicitHeight: 96
    implicitWidth: 600
    radius: Theme.radiusMd
    color: cardHover.hovered ? Theme.bgCardHover : Theme.bgCard
    border.color: Theme.borderDim
    border.width: 1

    Behavior on color { ColorAnimation { duration: 150 } }
    HoverHandler { id: cardHover }

    Row {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 14

        // Icon Box
        Rectangle {
            width: 44
            height: 44
            radius: Theme.radiusSm
            color: Theme.bgElevated
            anchors.verticalCenter: parent.verticalCenter

            Text {
                anchors.centerIn: parent
                text: model.isAudioOnly ? "MP3" : "MP4"
                color: Theme.accent
                font.pixelSize: 11
                font.family: Theme.fontFamily
                font.weight: Font.Bold
            }
        }

        // Center Content: Info & Progress
        Column {
            width: parent.width - 44 - 14 - actionButtons.width - 14
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6

            // Top Line: Title & Status Badge
            Row {
                width: parent.width
                spacing: 8

                Text {
                    text: model.title
                    color: Theme.textPrimary
                    font.pixelSize: 13
                    font.family: Theme.fontFamily
                    font.weight: Font.DemiBold
                    elide: Text.ElideMiddle
                    width: Math.min(implicitWidth, parent.width - statusBadge.width - 10)
                }

                Rectangle {
                    id: statusBadge
                    height: 18
                    width: statusText.width + 10
                    radius: 4
                    color: {
                        if (model.status === "Completed") return Theme.success + "22"
                        if (model.status === "Downloading") return Theme.accent + "22"
                        if (model.status === "Converting") return Theme.accentPurple + "22"
                        if (model.status === "Paused") return Theme.warning + "22"
                        if (model.status === "Queued") return Theme.accentCyan + "22"
                        if (model.status === "Failed") return Theme.error + "22"
                        if (model.status === "Cancelled") return Theme.error + "22"
                        return Theme.bgElevated
                    }

                    Text {
                        id: statusText
                        anchors.centerIn: parent
                        text: {
                            if (model.status === "Completed") return I18n.t("statusCompleted")
                            if (model.status === "Downloading") return I18n.t("statusDownloading")
                            if (model.status === "Converting") return I18n.t("statusConverting")
                            if (model.status === "Paused") return I18n.t("statusPaused")
                            if (model.status === "Queued") return I18n.t("statusQueued")
                            if (model.status === "Failed") return I18n.t("statusFailed")
                            if (model.status === "Cancelled") return I18n.t("statusCancelled")
                            return model.status
                        }
                        color: {
                            if (model.status === "Completed") return Theme.success
                            if (model.status === "Downloading") return Theme.accentHover
                            if (model.status === "Converting") return Theme.accentPurple
                            if (model.status === "Paused") return Theme.warning
                            if (model.status === "Queued") return Theme.accentCyan
                            if (model.status === "Failed") return Theme.error
                            if (model.status === "Cancelled") return Theme.error
                            return Theme.textSecondary
                        }
                        font.pixelSize: 10
                        font.family: Theme.fontFamily
                        font.weight: Font.Medium
                    }
                }
            }

            // Progress Bar
            ProgressBarCustom {
                width: parent.width
                value: model.progress
                barHeight: 5
                barColor: (model.status === "Completed") ? Theme.success : ((model.status === "Converting") ? Theme.accentPurple : ((model.status === "Queued") ? Theme.accentCyan : Theme.accent))
                barColorEnd: (model.status === "Completed") ? "#34D399" : ((model.status === "Converting") ? Theme.accentPink : ((model.status === "Queued") ? Theme.accent : Theme.accentPurple))
            }

            // Stats Line (Downloaded, Speed, ETA, Percentage)
            Row {
                width: parent.width
                spacing: 12

                Text {
                    text: {
                        if (model.status === "Completed") {
                            return model.totalFormatted || model.downloadedFormatted || ""
                        }
                        var dl = model.downloadedFormatted || ""
                        var tot = model.totalFormatted || ""
                        if (!dl && !tot) return ""
                        var base = dl + (tot ? (" / " + tot) : "")
                        if (model.status === "Downloading" && model.remainingFormatted && model.remainingFormatted.length > 0) {
                            return base + " (" + I18n.t("remainingPrefix") + model.remainingFormatted + ")"
                        }
                        return base
                    }
                    color: Theme.textMuted
                    font.pixelSize: 11
                    font.family: Theme.fontFamily
                }

                Text {
                    visible: model.status === "Downloading" && model.speed.length > 0
                    text: {
                        var sp = model.speed
                        if (sp === "Connecting..." || sp === "Bağlanıyor..." || sp === "Connecting") return "•  " + I18n.t("statusConnecting")
                        if (sp === "Resuming..." || sp === "Sürdürülüyor..." || sp === "Resuming") return "•  " + I18n.t("statusResuming")
                        if (sp === "Starting..." || sp === "Başlatılıyor..." || sp === "Starting") return "•  " + I18n.t("statusStarting")
                        if (sp === "Encoding 320k MP3..." || sp === "320k MP3 Kodlanıyor...") return "•  " + I18n.t("statusEncodingMp3")
                        if (sp === "Merging Audio & Video..." || sp === "Ses ve Video Birleştiriliyor...") return "•  " + I18n.t("statusMerging")
                        return "•  " + sp
                    }
                    color: Theme.accentCyan
                    font.pixelSize: 11
                    font.family: Theme.fontFamily
                }

                Text {
                    visible: model.status === "Queued"
                    text: "•  " + I18n.t("waitingInQueue")
                    color: Theme.accentCyan
                    font.pixelSize: 11
                    font.family: Theme.fontFamily
                }

                Text {
                    visible: model.status === "Failed" && model.speed.length > 0
                    text: "•  " + model.speed
                    color: Theme.error
                    font.pixelSize: 11
                    font.family: Theme.fontFamily
                    elide: Text.ElideRight
                    width: Math.min(implicitWidth, 260)
                }

                Text {
                    visible: model.status === "Downloading" && model.eta.length > 0
                    text: "•  ETA: " + model.eta
                    color: Theme.textSecondary
                    font.pixelSize: 11
                    font.family: Theme.fontFamily
                }

                Item { width: 10 }

                Text {
                    text: Math.round(model.progress) + "%"
                    color: Theme.textSecondary
                    font.pixelSize: 11
                    font.family: Theme.fontFamily
                    font.weight: Font.DemiBold
                }
            }
        }

        // Action Buttons (Up / Down / Pause / Resume / Cancel / Show in Explorer)
        Row {
            id: actionButtons
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6

            // Move Up (▲)
            Rectangle {
                visible: queueModel.totalDownloadCount > 1
                opacity: index > 0 ? 1.0 : 0.25
                width: 34
                height: 34
                radius: Theme.radiusSm
                color: (upMouse.containsMouse && index > 0) ? Theme.bgCardHover : Theme.bgElevated
                border.color: (upMouse.containsMouse && index > 0) ? Theme.accent : Theme.borderDim
                border.width: 1

                MouseArea {
                    id: upMouse
                    anchors.fill: parent
                    hoverEnabled: index > 0
                    cursorShape: index > 0 ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: {
                        if (index > 0) queueModel.moveItemUp(model.id)
                    }
                }

                Text {
                    anchors.centerIn: parent
                    text: "▲"
                    color: (upMouse.containsMouse && index > 0) ? Theme.textPrimary : Theme.textSecondary
                    font.pixelSize: 10
                }
            }

            // Move Down (▼)
            Rectangle {
                visible: queueModel.totalDownloadCount > 1
                opacity: index < (queueModel.totalDownloadCount - 1) ? 1.0 : 0.25
                width: 34
                height: 34
                radius: Theme.radiusSm
                color: (downMouse.containsMouse && index < (queueModel.totalDownloadCount - 1)) ? Theme.bgCardHover : Theme.bgElevated
                border.color: (downMouse.containsMouse && index < (queueModel.totalDownloadCount - 1)) ? Theme.accent : Theme.borderDim
                border.width: 1

                MouseArea {
                    id: downMouse
                    anchors.fill: parent
                    hoverEnabled: index < (queueModel.totalDownloadCount - 1)
                    cursorShape: index < (queueModel.totalDownloadCount - 1) ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: {
                        if (index < (queueModel.totalDownloadCount - 1)) queueModel.moveItemDown(model.id)
                    }
                }

                Text {
                    anchors.centerIn: parent
                    text: "▼"
                    color: (downMouse.containsMouse && index < (queueModel.totalDownloadCount - 1)) ? Theme.textPrimary : Theme.textSecondary
                    font.pixelSize: 10
                }
            }

            // Open Folder (When completed)
            Rectangle {
                visible: model.status === "Completed"
                width: 34
                height: 34
                radius: Theme.radiusSm
                color: folderMouse.containsMouse ? Theme.bgCardHover : Theme.bgElevated
                border.color: folderMouse.containsMouse ? Theme.accent : Theme.borderDim
                border.width: 1

                MouseArea {
                    id: folderMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: queueModel.openFileFolder(model.id)
                }

                Canvas {
                    id: folderCanvas
                    anchors.centerIn: parent
                    width: 14; height: 14
                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.reset();
                        ctx.strokeStyle = folderMouse.containsMouse ? Theme.textPrimary : Theme.textSecondary;
                        ctx.lineWidth = 1.4;
                        ctx.lineCap = "round";
                        ctx.lineJoin = "round";
                        ctx.beginPath();
                        ctx.moveTo(1, 4);
                        ctx.lineTo(1, 12);
                        ctx.lineTo(13, 12);
                        ctx.lineTo(13, 4);
                        ctx.lineTo(7, 4);
                        ctx.lineTo(5, 2);
                        ctx.lineTo(1, 2);
                        ctx.closePath();
                        ctx.stroke();
                    }
                    Connections {
                        target: folderMouse
                        function onContainsMouseChanged() { folderCanvas.requestPaint(); }
                    }
                }
            }

            // View Error Log (When failed)
            Rectangle {
                visible: model.status === "Failed"
                width: 34
                height: 34
                radius: Theme.radiusSm
                color: logMouse.containsMouse ? Theme.bgCardHover : Theme.bgElevated
                border.color: logMouse.containsMouse ? Theme.accentCyan : Theme.borderDim
                border.width: 1

                MouseArea {
                    id: logMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (typeof downloadManager !== "undefined" && downloadManager) {
                            downloadManager.openLogFile()
                        }
                    }
                }

                ToolTip.visible: logMouse.containsMouse
                ToolTip.text: I18n.t("viewLogHint")
                ToolTip.delay: 300

                Item {
                    anchors.centerIn: parent
                    width: 14; height: 14
                    Canvas {
                        anchors.fill: parent
                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.reset();
                            ctx.strokeStyle = logMouse.containsMouse ? Theme.textPrimary : Theme.textSecondary;
                            ctx.lineWidth = 1.2;
                            ctx.strokeRect(1.5, 1.5, 11, 11);
                            ctx.beginPath();
                            ctx.moveTo(4, 5); ctx.lineTo(10, 5);
                            ctx.moveTo(4, 8); ctx.lineTo(8, 8);
                            ctx.stroke();
                        }
                    }
                }
            }

            // Pause / Resume / Retry (When downloading, paused, queued, or failed)
            Rectangle {
                visible: model.status === "Downloading" || model.status === "Paused" || model.status === "Queued" || model.status === "Failed"
                width: 34
                height: 34
                radius: Theme.radiusSm
                color: pauseMouse.containsMouse ? Theme.bgCardHover : Theme.bgElevated
                border.color: pauseMouse.containsMouse ? Theme.accent : Theme.borderDim
                border.width: 1

                MouseArea {
                    id: pauseMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (model.status === "Downloading") {
                            queueModel.pauseDownload(model.id)
                        } else {
                            queueModel.resumeDownload(model.id)
                        }
                    }
                }

                ToolTip.visible: pauseMouse.containsMouse && model.status === "Failed"
                ToolTip.text: I18n.t("retryDownload")
                ToolTip.delay: 300

                Text {
                    anchors.centerIn: parent
                    text: (model.status === "Downloading") ? "⏸" : "▶"
                    color: Theme.textPrimary
                    font.pixelSize: 12
                }
            }

            // Cancel / Remove
            Rectangle {
                width: 34
                height: 34
                radius: Theme.radiusSm
                color: cancelMouse.containsMouse ? Theme.error + "22" : Theme.bgElevated
                border.color: cancelMouse.containsMouse ? Theme.error + "66" : Theme.borderDim
                border.width: 1

                MouseArea {
                    id: cancelMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: queueModel.cancelDownload(model.id)
                }

                Text {
                    anchors.centerIn: parent
                    text: "✕"
                    color: cancelMouse.containsMouse ? Theme.error : Theme.textSecondary
                    font.pixelSize: 13
                    font.weight: Font.Bold
                }
            }
        }
    }
}
