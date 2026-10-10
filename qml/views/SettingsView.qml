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

            // Setting Card 0: Language Selection (English / Turkish / Ukrainian)
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
                        Rectangle {
                            width: 6; height: 6; radius: 3
                            color: Theme.accent
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
                            color: (I18n.language === "en") ? Theme.activeTabBg : (enMouse.containsMouse ? Theme.bgCardHover : Theme.bgElevated)
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
                                spacing: 10
                                Rectangle {
                                    width: 28; height: 20; radius: 4
                                    color: (I18n.language === "en") ? Theme.accent : Theme.borderBright
                                    anchors.verticalCenter: parent.verticalCenter
                                    Text {
                                        anchors.centerIn: parent
                                        text: "EN"
                                        color: (I18n.language === "en") ? "#07090e" : Theme.textPrimary
                                        font.pixelSize: 11
                                        font.family: Theme.fontFamily
                                        font.weight: Font.Bold
                                    }
                                }
                                Text {
                                    text: I18n.t("langEn")
                                    color: (I18n.language === "en") ? Theme.activeTabText : Theme.textPrimary
                                    font.pixelSize: 12
                                    font.family: Theme.fontFamily
                                    font.weight: (I18n.language === "en") ? Font.Bold : Font.Medium
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                        }

                        // Option 2: Türkçe
                        Rectangle {
                            width: (parent.width - 24) / 3
                            height: 42
                            radius: Theme.radiusMd
                            color: (I18n.language === "tr") ? Theme.activeTabBg : (trMouse.containsMouse ? Theme.bgCardHover : Theme.bgElevated)
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
                                spacing: 10
                                Rectangle {
                                    width: 28; height: 20; radius: 4
                                    color: (I18n.language === "tr") ? Theme.accent : Theme.borderBright
                                    anchors.verticalCenter: parent.verticalCenter
                                    Text {
                                        anchors.centerIn: parent
                                        text: "TR"
                                        color: (I18n.language === "tr") ? "#07090e" : Theme.textPrimary
                                        font.pixelSize: 11
                                        font.family: Theme.fontFamily
                                        font.weight: Font.Bold
                                    }
                                }
                                Text {
                                    text: I18n.t("langTr")
                                    color: (I18n.language === "tr") ? Theme.activeTabText : Theme.textPrimary
                                    font.pixelSize: 12
                                    font.family: Theme.fontFamily
                                    font.weight: (I18n.language === "tr") ? Font.Bold : Font.Medium
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                        }

                        // Option 3: Українська
                        Rectangle {
                            width: (parent.width - 24) / 3
                            height: 42
                            radius: Theme.radiusMd
                            color: (I18n.language === "uk") ? Theme.activeTabBg : (ukMouse.containsMouse ? Theme.bgCardHover : Theme.bgElevated)
                            border.color: (I18n.language === "uk") ? Theme.accent : Theme.borderDim
                            border.width: (I18n.language === "uk") ? 2 : 1

                            MouseArea {
                                id: ukMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: I18n.setLanguage("uk")
                            }

                            Row {
                                anchors.centerIn: parent
                                spacing: 10
                                Rectangle {
                                    width: 28; height: 20; radius: 4
                                    color: (I18n.language === "uk") ? Theme.accent : Theme.borderBright
                                    anchors.verticalCenter: parent.verticalCenter
                                    Text {
                                        anchors.centerIn: parent
                                        text: "UA"
                                        color: (I18n.language === "uk") ? "#07090e" : Theme.textPrimary
                                        font.pixelSize: 11
                                        font.family: Theme.fontFamily
                                        font.weight: Font.Bold
                                    }
                                }
                                Text {
                                    text: I18n.t("langUk")
                                    color: (I18n.language === "uk") ? Theme.activeTabText : Theme.textPrimary
                                    font.pixelSize: 12
                                    font.family: Theme.fontFamily
                                    font.weight: (I18n.language === "uk") ? Font.Bold : Font.Medium
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                        }
                    }
                }
            }

            // Setting Card: Video Background Toggle
            Rectangle {
                width: parent.width
                implicitHeight: 70
                radius: Theme.radiusLg
                color: Theme.bgCard
                border.color: Theme.borderDim
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    Item {
                        width: parent.width - toggleSwitch.width - 16
                        height: parent.height
                        anchors.verticalCenter: parent.verticalCenter

                        Row {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 8
                            Rectangle {
                                width: 6; height: 6; radius: 3
                                color: Theme.accent
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: I18n.t("videoBgSection")
                                color: Theme.textPrimary
                                font.pixelSize: 14
                                font.family: Theme.fontFamily
                                font.weight: Font.DemiBold
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: "• " + (downloadManager.videoBackgroundEnabled ? I18n.t("videoBgEnabled") : I18n.t("videoBgDisabled"))
                                color: downloadManager.videoBackgroundEnabled ? Theme.accent : Theme.textMuted
                                font.pixelSize: 11
                                font.family: Theme.fontFamily
                                font.weight: Font.Medium
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }

                    // Sleek Modern Toggle Switch
                    Rectangle {
                        id: toggleSwitch
                        width: 54
                        height: 30
                        radius: 15
                        color: downloadManager.videoBackgroundEnabled ? Theme.accent : Theme.bgElevated
                        border.color: downloadManager.videoBackgroundEnabled ? Theme.accent : Theme.borderBright
                        border.width: 1
                        anchors.verticalCenter: parent.verticalCenter

                        Behavior on color { ColorAnimation { duration: 180 } }
                        Behavior on border.color { ColorAnimation { duration: 180 } }

                        // Toggle Knob
                        Rectangle {
                            width: 22
                            height: 22
                            radius: 11
                            color: downloadManager.videoBackgroundEnabled ? "#07090e" : Theme.textSecondary
                            anchors.verticalCenter: parent.verticalCenter
                            x: downloadManager.videoBackgroundEnabled ? (parent.width - width - 4) : 4

                            Behavior on x {
                                NumberAnimation { duration: 180; easing.type: Easing.OutQuad }
                            }
                            Behavior on color { ColorAnimation { duration: 180 } }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                downloadManager.setVideoBackgroundEnabled(!downloadManager.videoBackgroundEnabled)
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

                    Row {
                        spacing: 8
                        Rectangle {
                            width: 6; height: 6; radius: 3
                            color: Theme.accent
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: I18n.t("storageSection")
                            color: Theme.textPrimary
                            font.pixelSize: 14
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                            anchors.verticalCenter: parent.verticalCenter
                        }
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
                        border.color: Theme.borderDim
                        border.width: 1
                        anchors.verticalCenter: parent.verticalCenter
                        Image {
                            anchors.centerIn: parent
                            width: 28; height: 28
                            source: "qrc:/qt/qml/MediaGrab/assets/ffmpeg.png"
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            mipmap: true
                        }
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
                        border.color: Theme.borderDim
                        border.width: 1
                        anchors.verticalCenter: parent.verticalCenter
                        Image {
                            anchors.centerIn: parent
                            width: 28; height: 28
                            source: "qrc:/qt/qml/MediaGrab/assets/ytdlp.png"
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            mipmap: true
                        }
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

            // Setting Card 4: C++20 Core Engine Status
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
                        border.color: Theme.borderDim
                        border.width: 1
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            anchors.centerIn: parent
                            text: "⚡"
                            font.pixelSize: 22
                        }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 4
                        width: parent.width - 170

                        Text {
                            text: I18n.t("coreEngineTitle")
                            color: Theme.textPrimary
                            font.pixelSize: 14
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }
                        Text {
                            text: I18n.t("coreEngineDesc")
                            color: Theme.textMuted
                            font.pixelSize: 12
                            font.family: Theme.fontFamily
                            wrapMode: Text.WordWrap
                            width: parent.width
                        }
                    }

                    Rectangle {
                        height: 28
                        width: coreEngineBadgeText.width + 18
                        radius: 14
                        anchors.verticalCenter: parent.verticalCenter
                        color: Theme.success + "22"
                        border.color: Theme.success
                        border.width: 1

                        Text {
                            id: coreEngineBadgeText
                            anchors.centerIn: parent
                            text: I18n.t("readyStatus")
                            color: Theme.success
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }
                    }
                }
            }


            // Setting Card 4: Diagnostics & Error Log
            Rectangle {
                width: parent.width
                implicitHeight: 88
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
                        border.color: Theme.borderDim
                        border.width: 1
                        anchors.verticalCenter: parent.verticalCenter

                        Item {
                            anchors.centerIn: parent
                            width: 22; height: 22
                            Canvas {
                                anchors.fill: parent
                                onPaint: {
                                    var ctx = getContext("2d");
                                    ctx.reset();
                                    ctx.strokeStyle = Theme.accent;
                                    ctx.lineWidth = 1.6;
                                    ctx.strokeRect(3, 2, 16, 18);
                                    ctx.beginPath();
                                    ctx.moveTo(6, 7); ctx.lineTo(16, 7);
                                    ctx.moveTo(6, 11); ctx.lineTo(16, 11);
                                    ctx.moveTo(6, 15); ctx.lineTo(12, 15);
                                    ctx.stroke();
                                }
                            }
                        }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 4
                        width: parent.width - 44 - 16 - 220

                        Text {
                            text: I18n.t("diagnosticsSection")
                            color: Theme.textPrimary
                            font.pixelSize: 14
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }
                        Text {
                            text: I18n.t("diagnosticsSubtitle")
                            color: Theme.textMuted
                            font.pixelSize: 12
                            font.family: Theme.fontFamily
                            wrapMode: Text.WordWrap
                            width: parent.width
                        }
                    }

                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8

                        // Open Log File Button
                        Rectangle {
                            width: 110
                            height: 36
                            radius: Theme.radiusMd
                            color: openLogBtnMouse.containsMouse ? Theme.bgCardHover : Theme.bgElevated
                            border.color: openLogBtnMouse.containsMouse ? Theme.accentCyan : Theme.borderDim
                            border.width: 1

                            MouseArea {
                                id: openLogBtnMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (typeof downloadManager !== "undefined" && downloadManager) {
                                        downloadManager.openLogFile()
                                    }
                                }
                            }

                            Text {
                                anchors.centerIn: parent
                                text: I18n.t("openLogBtn")
                                color: openLogBtnMouse.containsMouse ? Theme.textPrimary : Theme.textSecondary
                                font.pixelSize: 11
                                font.family: Theme.fontFamily
                                font.weight: Font.Medium
                            }
                        }

                        // Clear Log Button
                        Rectangle {
                            width: 90
                            height: 36
                            radius: Theme.radiusMd
                            color: clearLogBtnMouse.containsMouse ? Theme.error + "22" : Theme.bgElevated
                            border.color: clearLogBtnMouse.containsMouse ? Theme.error + "66" : Theme.borderDim
                            border.width: 1

                            MouseArea {
                                id: clearLogBtnMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (typeof downloadManager !== "undefined" && downloadManager) {
                                        downloadManager.clearLogFile()
                                    }
                                }
                            }

                            Text {
                                anchors.centerIn: parent
                                text: I18n.t("clearLogBtn")
                                color: clearLogBtnMouse.containsMouse ? Theme.error : Theme.textSecondary
                                font.pixelSize: 11
                                font.family: Theme.fontFamily
                                font.weight: Font.Medium
                            }
                        }
                    }
                }
            }

            // Setting Card 5: About YuYuİnstaller Architecture
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

                    Row {
                        spacing: 8
                        Rectangle {
                            width: 6; height: 6; radius: 3
                            color: Theme.accent
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: I18n.t("aboutTitle")
                            color: Theme.textPrimary
                            font.pixelSize: 14
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                            anchors.verticalCenter: parent.verticalCenter
                        }
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
