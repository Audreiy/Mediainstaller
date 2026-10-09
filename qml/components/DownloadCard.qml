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
                text: model.isAudioOnly ? "🎵" : "🎬"
                font.pixelSize: 20
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
                        if (model.status === "Failed") return Theme.error + "22"
                        if (model.status === "Cancelled") return Theme.error + "22"
                        return Theme.bgElevated
                    }

                    Text {
                        id: statusText
                        anchors.centerIn: parent
                        text: {
                            if (model.status === "Completed") return "Completed"
                            if (model.status === "Downloading") return "Downloading"
                            if (model.status === "Converting") return "Converting (MP3)"
                            if (model.status === "Paused") return "Paused"
                            if (model.status === "Failed") return "Failed"
                            if (model.status === "Cancelled") return "Cancelled"
                            return model.status
                        }
                        color: {
                            if (model.status === "Completed") return Theme.success
                            if (model.status === "Downloading") return Theme.accentHover
                            if (model.status === "Converting") return Theme.accentPurple
                            if (model.status === "Paused") return Theme.warning
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
                barColor: (model.status === "Completed") ? Theme.success : ((model.status === "Converting") ? Theme.accentPurple : Theme.accent)
                barColorEnd: (model.status === "Completed") ? "#34D399" : ((model.status === "Converting") ? Theme.accentPink : Theme.accentPurple)
            }

            // Stats Line (Downloaded, Speed, ETA, Percentage)
            Row {
                width: parent.width
                spacing: 12

                Text {
                    text: model.downloadedFormatted + (model.totalFormatted ? (" / " + model.totalFormatted) : "")
                    color: Theme.textMuted
                    font.pixelSize: 11
                    font.family: Theme.fontFamily
                }

                Text {
                    visible: model.status === "Downloading" && model.speed.length > 0
                    text: "•  " + model.speed
                    color: Theme.accentCyan
                    font.pixelSize: 11
                    font.family: Theme.fontFamily
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

        // Action Buttons (Pause / Resume / Cancel / Show in Explorer)
        Row {
            id: actionButtons
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6

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

                Text {
                    anchors.centerIn: parent
                    text: "📂"
                    font.pixelSize: 14
                }
            }

            // Pause / Resume (When downloading or paused)
            Rectangle {
                visible: model.status === "Downloading" || model.status === "Paused"
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
