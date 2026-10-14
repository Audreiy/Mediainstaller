import QtQuick
import QtQuick.Controls
import MediaGrab

Item {
    id: root

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

            // Header
            Column {
                spacing: 4
                Text {
                    text: I18n.t("updateSection")
                    color: Theme.textPrimary
                    font.pixelSize: 22
                    font.family: Theme.fontFamily
                    font.weight: Font.Bold
                }
                Text {
                    text: I18n.t("updateSubtitle")
                    color: Theme.textSecondary
                    font.pixelSize: 12
                    font.family: Theme.fontFamily
                }
            }

            // Main Application Updates Card (User Screenshot)
            Rectangle {
                width: parent.width
                implicitHeight: updateCardCol.implicitHeight + 40
                radius: Theme.radiusLg
                color: Theme.bgCard
                border.color: (typeof updateManager !== "undefined" && updateManager.updateAvailable) ? Theme.accent : Theme.borderDim
                border.width: (typeof updateManager !== "undefined" && updateManager.updateAvailable) ? 2 : 1

                Column {
                    id: updateCardCol
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 14

                    Row {
                        spacing: 8

                        // Animated green indicator dot with breathing pulse
                        Item {
                            width: 12; height: 12
                            anchors.verticalCenter: parent.verticalCenter

                            Rectangle {
                                anchors.centerIn: parent
                                width: 12; height: 12; radius: 6
                                color: Theme.accent
                                opacity: 0.35
                                visible: (typeof updateManager !== "undefined" && updateManager.updateAvailable)
                                SequentialAnimation on scale {
                                    running: (typeof updateManager !== "undefined" && updateManager.updateAvailable)
                                    loops: Animation.Infinite
                                    NumberAnimation { from: 0.8; to: 1.4; duration: 1000; easing.type: Easing.InOutQuad }
                                    NumberAnimation { from: 1.4; to: 0.8; duration: 1000; easing.type: Easing.InOutQuad }
                                }
                            }

                            Rectangle {
                                anchors.centerIn: parent
                                width: 6; height: 6; radius: 3
                                color: Theme.accent
                            }
                        }

                        Text {
                            text: I18n.t("updateSection")
                            color: Theme.textPrimary
                            font.pixelSize: 14
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: "• " + I18n.t("updateSubtitle")
                            color: Theme.textMuted
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Row {
                        width: parent.width
                        spacing: 12

                        Rectangle {
                            height: 42
                            width: parent.width - 150
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Row {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 10
                                anchors.verticalCenter: parent.verticalCenter

                                Text {
                                    text: I18n.t("currentVersionLabel") + ": v" + ((typeof updateManager !== "undefined") ? updateManager.currentVersion : "1.0.0")
                                    color: Theme.textPrimary
                                    font.pixelSize: 12
                                    font.family: Theme.fontFamily
                                    font.weight: Font.DemiBold
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                Rectangle {
                                    width: 1; height: 16
                                    color: Theme.borderDim
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                Text {
                                    text: (typeof updateManager !== "undefined") ? updateManager.statusMessage : ""
                                    color: (typeof updateManager !== "undefined" && updateManager.updateAvailable) ? Theme.accent : Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    anchors.verticalCenter: parent.verticalCenter
                                    elide: Text.ElideRight
                                    width: parent.width - 180
                                }
                            }
                        }

                        Rectangle {
                            width: 138
                            height: 42
                            radius: Theme.radiusMd
                            color: {
                                if (typeof updateManager === "undefined") return Theme.bgElevated
                                if (updateManager.downloading) return Theme.bgElevated
                                if (updateManager.updateAvailable) return Theme.accent
                                if (checkBtnMouse.containsMouse) return Theme.bgCardHover
                                return Theme.bgElevated
                            }
                            border.color: {
                                if (typeof updateManager === "undefined") return Theme.borderDim
                                if (updateManager.updateAvailable) return Theme.accentHover
                                if (checkBtnMouse.containsMouse) return Theme.accent
                                return Theme.borderDim
                            }
                            border.width: 1

                            MouseArea {
                                id: checkBtnMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                enabled: (typeof updateManager !== "undefined" && !updateManager.checking && !updateManager.downloading)
                                onClicked: {
                                    if (updateManager.updateAvailable) {
                                        updateManager.startDownloadAndInstall()
                                    } else {
                                        updateManager.checkForUpdates(false)
                                    }
                                }
                            }

                            Text {
                                anchors.centerIn: parent
                                text: {
                                    if (typeof updateManager === "undefined") return I18n.t("checkUpdatesBtn")
                                    if (updateManager.checking) return I18n.t("checkingUpdates")
                                    if (updateManager.downloading) return updateManager.downloadProgress + "%"
                                    if (updateManager.updateAvailable) return I18n.t("downloadAndInstallBtn")
                                    return I18n.t("checkUpdatesBtn")
                                }
                                color: (typeof updateManager !== "undefined" && updateManager.updateAvailable && !updateManager.downloading) ? "#07090e" : Theme.textPrimary
                                font.pixelSize: 11
                                font.family: Theme.fontFamily
                                font.weight: Font.Bold
                            }
                        }
                    }

                    // Progress Bar during download
                    Column {
                        width: parent.width
                        spacing: 6
                        visible: (typeof updateManager !== "undefined" && updateManager.downloading)

                        Row {
                            width: parent.width
                            Text {
                                text: I18n.t("downloadingUpdate")
                                color: Theme.accent
                                font.pixelSize: 11
                                font.family: Theme.fontFamily
                            }
                            Item { width: parent.width - 200; height: 1 }
                            Text {
                                text: (typeof updateManager !== "undefined") ? (updateManager.downloadProgress + "%") : "0%"
                                color: Theme.textPrimary
                                font.pixelSize: 11
                                font.family: Theme.fontFamily
                                font.weight: Font.Bold
                            }
                        }

                        Rectangle {
                            width: parent.width
                            height: 6
                            radius: 3
                            color: Theme.bgInput
                            clip: true

                            Rectangle {
                                height: parent.height
                                width: (typeof updateManager !== "undefined") ? (parent.width * (updateManager.downloadProgress / 100.0)) : 0
                                radius: 3
                                color: Theme.accent
                            }
                        }
                    }

                    // Changelog / Release Notes
                    Rectangle {
                        width: parent.width
                        implicitHeight: Math.min(notesText.implicitHeight + 16, 120)
                        radius: Theme.radiusMd
                        color: Theme.bgInput
                        border.color: Theme.borderDim
                        border.width: 1
                        clip: true
                        visible: (typeof updateManager !== "undefined" && updateManager.updateAvailable && updateManager.releaseNotes.length > 0)

                        Text {
                            id: notesText
                            anchors.fill: parent
                            anchors.margins: 10
                            text: (typeof updateManager !== "undefined") ? updateManager.releaseNotes : ""
                            color: Theme.textSecondary
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            wrapMode: Text.WordWrap
                            elide: Text.ElideRight
                        }
                    }
                }
            }

            // Update Logs & Changelog Card
            Rectangle {
                width: parent.width
                implicitHeight: changelogCol.implicitHeight + 40
                radius: Theme.radiusLg
                color: Theme.bgCard
                border.color: Theme.borderDim
                border.width: 1

                Column {
                    id: changelogCol
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 14

                    Row {
                        spacing: 8
                        Rectangle {
                            width: 6; height: 6; radius: 3
                            color: Theme.accent
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: I18n.t("updateLogTitle")
                            color: Theme.textPrimary
                            font.pixelSize: 14
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: "• " + I18n.t("updateLogSubtitle")
                            color: Theme.textMuted
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    // GitHub Release Notes (if body is supplied by release author)
                    Rectangle {
                        width: parent.width
                        implicitHeight: remoteNotesCol.implicitHeight + 20
                        radius: Theme.radiusMd
                        color: Theme.bgInput
                        border.color: Theme.accent
                        border.width: 1
                        visible: (typeof updateManager !== "undefined" && updateManager.releaseNotes.length > 0)

                        Column {
                            id: remoteNotesCol
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 6

                            Row {
                                spacing: 6
                                Rectangle {
                                    width: 46; height: 18; radius: 4
                                    color: Theme.accent
                                    Text {
                                        anchors.centerIn: parent
                                        text: (typeof updateManager !== "undefined" && updateManager.latestVersion.length > 0) ? updateManager.latestVersion : "LATEST"
                                        color: "#07090e"
                                        font.pixelSize: 10
                                        font.weight: Font.Bold
                                    }
                                }
                                Text {
                                    text: "GitHub Notes"
                                    color: Theme.textPrimary
                                    font.pixelSize: 11
                                    font.weight: Font.Bold
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            Text {
                                width: parent.width
                                text: (typeof updateManager !== "undefined") ? updateManager.releaseNotes : ""
                                color: Theme.textSecondary
                                font.pixelSize: 11
                                font.family: Theme.fontFamily
                                wrapMode: Text.WordWrap
                            }
                        }
                    }

                    // Version Timeline / Logs
                    Column {
                        width: parent.width
                        spacing: 10

                        // v1.1.7 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log117Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log117Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.accent
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.1.7"
                                            color: "#07090e"
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                    Rectangle {
                                        width: 54; height: 18; radius: 9
                                        color: "#162338"
                                        border.color: Theme.accent
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "CURRENT"
                                            color: Theme.accent
                                            font.pixelSize: 9
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log117_1") + "\n• " + I18n.t("log117_2") + "\n• " + I18n.t("log117_3") + "\n• " + I18n.t("log117_4")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.1.6 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log116Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log116Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.1.6"
                                            color: Theme.textPrimary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log116_1") + "\n• " + I18n.t("log116_2") + "\n• " + I18n.t("log116_3") + "\n• " + I18n.t("log116_4")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.1.5 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log115Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log115Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.1.5"
                                            color: Theme.textPrimary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log115_1") + "\n• " + I18n.t("log115_2") + "\n• " + I18n.t("log115_3") + "\n• " + I18n.t("log115_4")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.1.4 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log114Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log114Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.1.4"
                                            color: Theme.textPrimary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log114_1") + "\n• " + I18n.t("log114_2") + "\n• " + I18n.t("log114_3") + "\n• " + I18n.t("log114_4")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.1.3 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log113Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log113Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.1.3"
                                            color: Theme.textPrimary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log113_1") + "\n• " + I18n.t("log113_2") + "\n• " + I18n.t("log113_3") + "\n• " + I18n.t("log113_4")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.1.2 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log112Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log112Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.1.2"
                                            color: Theme.textPrimary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log112_1") + "\n• " + I18n.t("log112_2") + "\n• " + I18n.t("log112_3") + "\n• " + I18n.t("log112_4")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.1.1 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log111Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log111Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.1.1"
                                            color: Theme.textPrimary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log111_1") + "\n• " + I18n.t("log111_2") + "\n• " + I18n.t("log111_3") + "\n• " + I18n.t("log111_4")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.1.0 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log110Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log110Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.1.0"
                                            color: Theme.textSecondary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log110_1") + "\n• " + I18n.t("log110_2") + "\n• " + I18n.t("log110_3") + "\n• " + I18n.t("log110_4")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.0.9 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log109Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log109Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.0.9"
                                            color: Theme.textSecondary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log109_1") + "\n• " + I18n.t("log109_2") + "\n• " + I18n.t("log109_3") + "\n• " + I18n.t("log109_4")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.0.8 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log108Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log108Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.0.8"
                                            color: Theme.textSecondary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log108_1") + "\n• " + I18n.t("log108_2") + "\n• " + I18n.t("log108_3") + "\n• " + I18n.t("log108_4")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.0.7 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log107Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log107Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.0.7"
                                            color: Theme.textSecondary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log107_1") + "\n• " + I18n.t("log107_2") + "\n• " + I18n.t("log107_3")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.0.6 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log106Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log106Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.0.6"
                                            color: Theme.textSecondary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log106_1") + "\n• " + I18n.t("log106_2")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.0.5 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log105Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log105Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.0.5"
                                            color: Theme.textPrimary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log105_1") + "\n• " + I18n.t("log105_2")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.0.4 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log104Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log104Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.0.4"
                                            color: Theme.textPrimary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log104_1") + "\n• " + I18n.t("log104_2") + "\n• " + I18n.t("log104_3")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.0.3 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log103Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log103Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.0.3"
                                            color: Theme.textPrimary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log103_1")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.0.2 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log102Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log102Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.0.2"
                                            color: Theme.textPrimary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log102_1") + "\n• " + I18n.t("log102_2")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.0.1 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log101Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log101Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.0.1"
                                            color: Theme.textPrimary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log101_1") + "\n• " + I18n.t("log101_2")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        // v1.0.0 Pro
                        Rectangle {
                            width: parent.width
                            implicitHeight: log100Col.implicitHeight + 20
                            radius: Theme.radiusMd
                            color: Theme.bgInput
                            border.color: Theme.borderDim
                            border.width: 1

                            Column {
                                id: log100Col
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 8

                                Row {
                                    spacing: 8
                                    Rectangle {
                                        width: 58; height: 20; radius: 4
                                        color: Theme.bgElevated
                                        border.color: Theme.borderDim
                                        border.width: 1
                                        Text {
                                            anchors.centerIn: parent
                                            text: "v1.0.0"
                                            color: Theme.textPrimary
                                            font.pixelSize: 11
                                            font.weight: Font.Bold
                                        }
                                    }
                                }

                                Text {
                                    width: parent.width
                                    text: "• " + I18n.t("log100_1")
                                    color: Theme.textSecondary
                                    font.pixelSize: 11
                                    font.family: Theme.fontFamily
                                    lineHeight: 1.45
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
