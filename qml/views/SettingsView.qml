import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import MediaGrab

Item {
    id: root

    FolderDialog {
        id: folderDialog
        title: "Select Download Folder"
        currentFolder: "file:///" + downloadManager.defaultDownloadPath
        onAccepted: {
            var path = selectedFolder.toString().replace("file:///", "")
            downloadManager.setDefaultDownloadPath(path)
        }
    }

    Flickable {
        anchors.fill: parent
        anchors.margins: 28
        contentHeight: contentCol.height + 40
        clip: true

        Column {
            id: contentCol
            width: Math.min(parent.width, 760)
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 24

            Column {
                spacing: 4
                Text {
                    text: "Settings & Preferences"
                    color: Theme.textPrimary
                    font.pixelSize: 22
                    font.family: Theme.fontFamily
                    font.weight: Font.Bold
                }
                Text {
                    text: "Configure download directory, media processing engine, and system tools."
                    color: Theme.textSecondary
                    font.pixelSize: 12
                    font.family: Theme.fontFamily
                }
            }

            // Setting Card 1: Default Download Directory
            Rectangle {
                width: parent.width
                implicitHeight: 120
                radius: Theme.radiusLg
                color: Theme.bgCard
                border.color: Theme.borderDim
                border.width: 1

                Column {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 12

                    Text {
                        text: "📁 Default Download Folder"
                        color: Theme.textPrimary
                        font.pixelSize: 14
                        font.family: Theme.fontFamily
                        font.weight: Font.DemiBold
                    }

                    Row {
                        width: parent.width
                        spacing: 12

                        Rectangle {
                            width: parent.width - 120
                            height: 40
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Text {
                                anchors.fill: parent
                                anchors.margins: 10
                                text: downloadManager.defaultDownloadPath
                                color: Theme.textSecondary
                                font.pixelSize: 12
                                font.family: Theme.fontFamily
                                elide: Text.ElideMiddle
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        Rectangle {
                            width: 108
                            height: 40
                            radius: Theme.radiusMd
                            color: dirHover.hovered ? Theme.bgCardHover : Theme.bgElevated
                            border.color: Theme.borderDim
                            border.width: 1

                            HoverHandler { id: dirHover }
                            TapHandler { onTapped: folderDialog.open() }

                            Text {
                                anchors.centerIn: parent
                                text: "Browse"
                                color: Theme.textPrimary
                                font.pixelSize: 12
                                font.family: Theme.fontFamily
                                font.weight: Font.Medium
                            }
                        }
                    }
                }
            }

            // Setting Card 2: FFmpeg Status
            Rectangle {
                width: parent.width
                implicitHeight: 100
                radius: Theme.radiusLg
                color: Theme.bgCard
                border.color: Theme.borderDim
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    Rectangle {
                        width: 44
                        height: 44
                        radius: Theme.radiusMd
                        color: Theme.bgElevated
                        anchors.verticalCenter: parent.verticalCenter
                        Text { anchors.centerIn: parent; text: "🎬"; font.pixelSize: 22 }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 4
                        width: parent.width - 170

                        Text {
                            text: "FFmpeg Media Processor"
                            color: Theme.textPrimary
                            font.pixelSize: 14
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }
                        Text {
                            text: ffmpegProcessor.isFFmpegAvailable ? "FFmpeg is installed and ready for media merging & audio conversion." : "FFmpeg was not detected on this system."
                            color: Theme.textMuted
                            font.pixelSize: 12
                            font.family: Theme.fontFamily
                            wrapMode: Text.WordWrap
                            width: parent.width
                        }
                    }

                    Rectangle {
                        height: 28
                        width: ffmpegBadgeText.width + 18
                        radius: 14
                        anchors.verticalCenter: parent.verticalCenter
                        color: ffmpegProcessor.isFFmpegAvailable ? Theme.success + "22" : Theme.warning + "22"
                        border.color: ffmpegProcessor.isFFmpegAvailable ? Theme.success : Theme.warning
                        border.width: 1

                        Text {
                            id: ffmpegBadgeText
                            anchors.centerIn: parent
                            text: ffmpegProcessor.isFFmpegAvailable ? "Ready" : "Missing"
                            color: ffmpegProcessor.isFFmpegAvailable ? Theme.success : Theme.warning
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }
                    }
                }
            }

            // Setting Card 3: yt-dlp Status
            Rectangle {
                width: parent.width
                implicitHeight: 100
                radius: Theme.radiusLg
                color: Theme.bgCard
                border.color: Theme.borderDim
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    Rectangle {
                        width: 44
                        height: 44
                        radius: Theme.radiusMd
                        color: Theme.bgElevated
                        anchors.verticalCenter: parent.verticalCenter
                        Text { anchors.centerIn: parent; text: "⚡"; font.pixelSize: 22 }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 4
                        width: parent.width - 170

                        Text {
                            text: "yt-dlp Video Engine"
                            color: Theme.textPrimary
                            font.pixelSize: 14
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }
                        Text {
                            text: ffmpegProcessor.isYtDlpAvailable ? "yt-dlp is installed and ready for YouTube & online video downloads." : "yt-dlp was not detected on this system."
                            color: Theme.textMuted
                            font.pixelSize: 12
                            font.family: Theme.fontFamily
                            wrapMode: Text.WordWrap
                            width: parent.width
                        }
                    }

                    Rectangle {
                        height: 28
                        width: ytdlpBadgeText.width + 18
                        radius: 14
                        anchors.verticalCenter: parent.verticalCenter
                        color: ffmpegProcessor.isYtDlpAvailable ? Theme.success + "22" : Theme.warning + "22"
                        border.color: ffmpegProcessor.isYtDlpAvailable ? Theme.success : Theme.warning
                        border.width: 1

                        Text {
                            id: ytdlpBadgeText
                            anchors.centerIn: parent
                            text: ffmpegProcessor.isYtDlpAvailable ? "Ready" : "Missing"
                            color: ffmpegProcessor.isYtDlpAvailable ? Theme.success : Theme.warning
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }
                    }
                }
            }

            // Setting Card 4: About MediaGrab Architecture
            Rectangle {
                width: parent.width
                implicitHeight: 130
                radius: Theme.radiusLg
                color: Theme.bgCard
                border.color: Theme.borderDim
                border.width: 1

                Column {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 8

                    Text {
                        text: "ℹ️ About MediaGrab"
                        color: Theme.textPrimary
                        font.pixelSize: 14
                        font.family: Theme.fontFamily
                        font.weight: Font.DemiBold
                    }

                    Text {
                        text: "MediaGrab is a modern Windows media downloader and converter built with C++20, Qt 6 Quick / QML, FFmpeg, and yt-dlp for high-performance direct streaming and 320 kbps audio extraction."
                        color: Theme.textSecondary
                        font.pixelSize: 12
                        font.family: Theme.fontFamily
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }

                    Text {
                        text: "Version 1.0.0 Pro • Hardware-Accelerated QML Interface"
                        color: Theme.accentHover
                        font.pixelSize: 11
                        font.family: Theme.fontFamily
                    }
                }
            }
        }
    }
}
