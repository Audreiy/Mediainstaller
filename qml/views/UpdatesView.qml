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

            // Architecture & Integrity Information Card
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
                    spacing: 10

                    Row {
                        spacing: 8
                        Rectangle {
                            width: 6; height: 6; radius: 3
                            color: Theme.accent
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: "Update Architecture & Security"
                            color: Theme.textPrimary
                            font.pixelSize: 13
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }
                    }

                    Text {
                        width: parent.width
                        text: "YuYuInstaller connects securely to official GitHub Releases via HTTPS. Updates are verified, unpacked in the background, and seamlessly applied without interrupting your active configuration."
                        color: Theme.textSecondary
                        font.pixelSize: 12
                        font.family: Theme.fontFamily
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }
    }
}
