import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import MediaGrab

Item {
    id: root

    FolderDialog {
        id: folderDialog
        title: I18n.t("storageSection")
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
                    text: I18n.t("settingsTitle")
                    color: Theme.textPrimary
                    font.pixelSize: 22
                    font.family: Theme.fontFamily
                    font.weight: Font.Bold
                }
                Text {
                    text: I18n.t("settingsSubtitle")
                    color: Theme.textSecondary
                    font.pixelSize: 12
                    font.family: Theme.fontFamily
                }
            }

            // Setting Card 0: Language Selection (English / Turkish / Russian)
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

                    Row {
                        spacing: 8
                        Text {
                            text: "🌐"
                            font.pixelSize: 15
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: I18n.t("langSection")
                            color: Theme.textPrimary
                            font.pixelSize: 14
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: "• " + I18n.t("langSubtitle")
                            color: Theme.textMuted
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Row {
                        width: parent.width
                        spacing: 12

                        // Option 1: English
                        Rectangle {
                            width: (parent.width - 24) / 3
                            height: 42
                            radius: Theme.radiusMd
                            color: (I18n.language === "en") ? (Theme.accent + "22") : (enMouse.containsMouse ? Theme.bgCardHover : Theme.bgElevated)
                            border.color: (I18n.language === "en") ? Theme.accent : Theme.borderDim
                            border.width: (I18n.language === "en") ? 2 : 1

                            MouseArea {
                                id: enMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: I18n.setLanguage("en")
                            }

                            Row {
                                anchors.centerIn: parent
                                spacing: 8
                                Text { text: "🇬🇧"; font.pixelSize: 16 }
                                Text {
                                    text: I18n.t("langEn")
                                    color: (I18n.language === "en") ? Theme.accentHover : Theme.textPrimary
                                    font.pixelSize: 12
                                    font.family: Theme.fontFamily
                                    font.weight: (I18n.language === "en") ? Font.Bold : Font.Medium
                                }
                            }
                        }

                        // Option 2: Türkçe
                        Rectangle {
                            width: (parent.width - 24) / 3
                            height: 42
                            radius: Theme.radiusMd
                            color: (I18n.language === "tr") ? (Theme.accent + "22") : (trMouse.containsMouse ? Theme.bgCardHover : Theme.bgElevated)
                            border.color: (I18n.language === "tr") ? Theme.accent : Theme.borderDim
                            border.width: (I18n.language === "tr") ? 2 : 1

                            MouseArea {
                                id: trMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: I18n.setLanguage("tr")
                            }

                            Row {
                                anchors.centerIn: parent
                                spacing: 8
                                Text { text: "🇹🇷"; font.pixelSize: 16 }
                                Text {
                                    text: I18n.t("langTr")
                                    color: (I18n.language === "tr") ? Theme.accentHover : Theme.textPrimary
                                    font.pixelSize: 12
                                    font.family: Theme.fontFamily
                                    font.weight: (I18n.language === "tr") ? Font.Bold : Font.Medium
                                }
                            }
                        }

                        // Option 3: Русский
                        Rectangle {
                            width: (parent.width - 24) / 3
                            height: 42
                            radius: Theme.radiusMd
                            color: (I18n.language === "ru") ? (Theme.accent + "22") : (ruMouse.containsMouse ? Theme.bgCardHover : Theme.bgElevated)
                            border.color: (I18n.language === "ru") ? Theme.accent : Theme.borderDim
                            border.width: (I18n.language === "ru") ? 2 : 1

                            MouseArea {
                                id: ruMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: I18n.setLanguage("ru")
                            }

                            Row {
                                anchors.centerIn: parent
                                spacing: 8
                                Text { text: "🇷🇺"; font.pixelSize: 16 }
                                Text {
                                    text: I18n.t("langRu")
                                    color: (I18n.language === "ru") ? Theme.accentHover : Theme.textPrimary
                                    font.pixelSize: 12
                                    font.family: Theme.fontFamily
                                    font.weight: (I18n.language === "ru") ? Font.Bold : Font.Medium
                                }
                            }
                        }
                    }
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
                        text: "📁 " + I18n.t("storageSection")
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
                            color: dirMouse.containsMouse ? Theme.bgCardHover : Theme.bgElevated
                            border.color: dirMouse.containsMouse ? Theme.accent : Theme.borderDim
                            border.width: 1

                            MouseArea {
                                id: dirMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: folderDialog.open()
                            }

                            Text {
                                anchors.centerIn: parent
                                text: I18n.t("browse")
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
                            text: I18n.t("ffmpegTitle")
                            color: Theme.textPrimary
                            font.pixelSize: 14
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }
                        Text {
                            text: ffmpegProcessor.isFFmpegAvailable ? I18n.t("ffmpegAvailable") : I18n.t("ffmpegNotFound")
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
                            text: ffmpegProcessor.isFFmpegAvailable ? I18n.t("readyStatus") : I18n.t("missingStatus")
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
                            text: I18n.t("ytdlpTitle")
                            color: Theme.textPrimary
                            font.pixelSize: 14
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }
                        Text {
                            text: ffmpegProcessor.isYtDlpAvailable ? I18n.t("ytdlpAvailable") : I18n.t("ytdlpNotFound")
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
                            text: ffmpegProcessor.isYtDlpAvailable ? I18n.t("readyStatus") : I18n.t("missingStatus")
                            color: ffmpegProcessor.isYtDlpAvailable ? Theme.success : Theme.warning
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }
                    }
                }
            }

            // Setting Card 4: About YuYuİnstaller Architecture
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
                        text: "ℹ️ " + I18n.t("aboutTitle")
                        color: Theme.textPrimary
                        font.pixelSize: 14
                        font.family: Theme.fontFamily
                        font.weight: Font.DemiBold
                    }

                    Text {
                        text: I18n.t("aboutDesc")
                        color: Theme.textSecondary
                        font.pixelSize: 12
                        font.family: Theme.fontFamily
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }

                    Text {
                        text: I18n.t("aboutVersion")
                        color: Theme.accentHover
                        font.pixelSize: 11
                        font.family: Theme.fontFamily
                    }
                }
            }
        }
    }
}
