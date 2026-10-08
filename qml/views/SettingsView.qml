import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import MediaGrab

Item {
    id: root

    FolderDialog {
        id: folderDialog
        title: "İndirme Klasörünü Seçin"
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
                    text: "Ayarlar ve Tercihler"
                    color: Theme.textPrimary
                    font.pixelSize: 22
                    font.family: Theme.fontFamily
                    font.weight: Font.Bold
                }
                Text {
                    text: "Depolama konumu, medya motoru ve ağ ayarlarını yapılandırın."
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
                        text: "📁 Varsayılan Kayıt Klasörü"
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
                                text: "Değiştir"
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
                            text: "FFmpeg Medya İşlemcisi"
                            color: Theme.textPrimary
                            font.pixelSize: 14
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }
                        Text {
                            text: ffmpegProcessor.isFFmpegAvailable ? "FFmpeg tespit edildi ve ses dönüştürmeye hazır." : "FFmpeg sistemde bulunamadı."
                            color: Theme.textMuted
                            font.pixelSize: 12
                            font.family: Theme.fontFamily
                        }
                    }

                    Rectangle {
                        height: 28
                        width: ffmpegBadgeText.width + 16
                        radius: 14
                        anchors.verticalCenter: parent.verticalCenter
                        color: ffmpegProcessor.isFFmpegAvailable ? Theme.success + "22" : Theme.warning + "22"
                        border.color: ffmpegProcessor.isFFmpegAvailable ? Theme.success : Theme.warning
                        border.width: 1

                        Text {
                            id: ffmpegBadgeText
                            anchors.centerIn: parent
                            text: ffmpegProcessor.isFFmpegAvailable ? "Aktif" : "Eksik"
                            color: ffmpegProcessor.isFFmpegAvailable ? Theme.success : Theme.warning
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }
                    }
                }
            }

            // Setting Card 3: About MediaGrab Architecture
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
                        text: "⚡ MediaGrab Hakkında"
                        color: Theme.textPrimary
                        font.pixelSize: 14
                        font.family: Theme.fontFamily
                        font.weight: Font.DemiBold
                    }

                    Text {
                        text: "MediaGrab; C++20, Qt 6 Quick / QML, FFmpeg ve libcurl teknolojileri kullanılarak geliştirilmiş modern bir Windows medya yönetim ve indirme uygulamasıdır."
                        color: Theme.textSecondary
                        font.pixelSize: 12
                        font.family: Theme.fontFamily
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }

                    Text {
                        text: "Sürüm 1.0.0 Pro • Donanım Hızlandırmalı QML Arayüzü"
                        color: Theme.accentHover
                        font.pixelSize: 11
                        font.family: Theme.fontFamily
                    }
                }
            }
        }
    }
}
