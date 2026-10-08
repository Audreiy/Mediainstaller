import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import MediaGrab

Rectangle {
    id: root
    implicitHeight: contentColumn.height + 40
    implicitWidth: 600
    radius: Theme.radiusLg
    color: Theme.bgCard
    border.color: Theme.borderDim
    border.width: 1

    property var mediaInfo: null
    property string selectedOptionId: "best_video"
    property string customFolder: downloadManager.defaultDownloadPath

    signal downloadTriggered(string url, string title, string formatId, string folder)

    Column {
        id: contentColumn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        spacing: 16

        // Header: Thumbnail & Title & Metadata
        Row {
            width: parent.width
            spacing: 14

            // Media Thumbnail Preview
            Rectangle {
                width: 72
                height: 48
                radius: Theme.radiusMd
                color: Theme.bgElevated
                border.color: Theme.borderDim
                border.width: 1
                clip: true

                Image {
                    id: thumbImg
                    anchors.fill: parent
                    source: root.mediaInfo ? root.mediaInfo.thumbnailUrl : ""
                    fillMode: Image.PreserveAspectCrop
                    visible: status === Image.Ready
                }

                Text {
                    anchors.centerIn: parent
                    visible: !thumbImg.visible
                    text: (root.selectedOptionId === "audio_mp3") ? "🎵" : "🎬"
                    font.pixelSize: 22
                }
            }

            Column {
                width: parent.width - 86
                spacing: 4

                Text {
                    width: parent.width
                    text: root.mediaInfo ? root.mediaInfo.title : "Media Item"
                    color: Theme.textPrimary
                    font.pixelSize: 14
                    font.family: Theme.fontFamily
                    font.weight: Font.DemiBold
                    elide: Text.ElideMiddle
                }

                Row {
                    spacing: 8
                    Rectangle {
                        height: 20
                        width: badgeText.width + 12
                        radius: 4
                        color: Theme.accent + "22"
                        border.color: Theme.accent + "66"
                        border.width: 1

                        Text {
                            id: badgeText
                            anchors.centerIn: parent
                            text: (root.mediaInfo && root.mediaInfo.isDirectFile) ? "Direct Media" : "Online Stream"
                            color: Theme.accentHover
                            font.pixelSize: 10
                            font.family: Theme.fontFamily
                            font.weight: Font.Medium
                        }
                    }

                    Text {
                        visible: root.mediaInfo && root.mediaInfo.author.length > 0
                        text: root.mediaInfo ? root.mediaInfo.author : ""
                        color: Theme.accentCyan
                        font.pixelSize: 11
                        font.family: Theme.fontFamily
                        font.weight: Font.Medium
                    }

                    Text {
                        text: root.mediaInfo ? root.mediaInfo.url : ""
                        color: Theme.textMuted
                        font.pixelSize: 11
                        font.family: Theme.fontFamily
                        elide: Text.ElideRight
                        width: 260
                    }
                }
            }
        }

        Rectangle {
            width: parent.width
            height: 1
            color: Theme.borderDim
        }

        // Section: Format / Quality Options
        Text {
            text: "Format & Quality Selection"
            color: Theme.textSecondary
            font.pixelSize: 12
            font.family: Theme.fontFamily
            font.weight: Font.DemiBold
        }

        Row {
            width: parent.width
            spacing: 12

            // Option 1: Video (Best Quality)
            Rectangle {
                id: videoCard
                width: (parent.width - 12) / 2
                height: 70
                radius: Theme.radiusMd
                color: (root.selectedOptionId !== "audio_mp3") ? (Theme.accent + "18") : (videoHover.hovered ? Theme.bgCardHover : Theme.bgInput)
                border.color: (root.selectedOptionId !== "audio_mp3") ? Theme.accent : Theme.borderDim
                border.width: (root.selectedOptionId !== "audio_mp3") ? 2 : 1

                HoverHandler { id: videoHover }
                TapHandler { onTapped: root.selectedOptionId = "best_video" }

                Row {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Text {
                        text: "📹"
                        font.pixelSize: 22
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: "Video (MP4 / Best Quality)"
                            color: Theme.textPrimary
                            font.pixelSize: 13
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: "Highest resolution video + merged audio"
                            color: Theme.textMuted
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                        }
                    }
                }
            }

            // Option 2: Audio Only (MP3)
            Rectangle {
                id: audioCard
                width: (parent.width - 12) / 2
                height: 70
                radius: Theme.radiusMd
                color: (root.selectedOptionId === "audio_mp3") ? (Theme.accentPurple + "18") : (audioHover.hovered ? Theme.bgCardHover : Theme.bgInput)
                border.color: (root.selectedOptionId === "audio_mp3") ? Theme.accentPurple : Theme.borderDim
                border.width: (root.selectedOptionId === "audio_mp3") ? 2 : 1

                HoverHandler { id: audioHover }
                TapHandler { onTapped: root.selectedOptionId = "audio_mp3" }

                Row {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Text {
                        text: "🎧"
                        font.pixelSize: 22
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: "Audio Only (MP3)"
                            color: Theme.textPrimary
                            font.pixelSize: 13
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: "Converted to 320 kbps HQ MP3"
                            color: Theme.textMuted
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                        }
                    }
                }
            }
        }

        // Section: Save Folder & Start Download
        Row {
            width: parent.width
            spacing: 12

            // Folder path
            Rectangle {
                width: parent.width - 160
                height: 44
                radius: Theme.radiusMd
                color: Theme.bgInput
                border.color: Theme.borderDim
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8

                    Text {
                        text: "📁"
                        font.pixelSize: 14
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: root.customFolder
                        color: Theme.textSecondary
                        font.pixelSize: 12
                        font.family: Theme.fontFamily
                        elide: Text.ElideMiddle
                        width: parent.width - 30
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            // Start Download Button
            Rectangle {
                id: downloadBtn
                width: 148
                height: 44
                radius: Theme.radiusMd

                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: dlHover.hovered ? Theme.success : "#059669" }
                    GradientStop { position: 1.0; color: dlHover.hovered ? "#34D399" : Theme.success }
                }

                HoverHandler { id: dlHover }

                Row {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        text: "⬇"
                        color: "#FFFFFF"
                        font.pixelSize: 15
                    }

                    Text {
                        text: "Start Download"
                        color: "#FFFFFF"
                        font.pixelSize: 13
                        font.family: Theme.fontFamily
                        font.weight: Font.Bold
                    }
                }

                TapHandler {
                    onTapped: {
                        if (root.mediaInfo) {
                            root.downloadTriggered(
                                root.mediaInfo.url,
                                root.mediaInfo.title,
                                root.selectedOptionId,
                                root.customFolder
                            )
                        }
                    }
                }
            }
        }
    }
}
