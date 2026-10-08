import QtQuick
import QtQuick.Controls
import MediaGrab

Item {
    id: root

    property string currentErrorMessage: ""

    signal navigateToQueue()

    Connections {
        target: urlAnalyzer
        function onAnalysisError(msg) {
            root.currentErrorMessage = msg
        }
        function onAnalysisSuccess(info) {
            root.currentErrorMessage = ""
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

            // Welcome & Title
            Column {
                spacing: 6
                Text {
                    text: "Media Downloader & Converter"
                    color: Theme.textPrimary
                    font.pixelSize: 24
                    font.family: Theme.fontFamily
                    font.weight: Font.Bold
                }
                Text {
                    text: "Paste any video or audio URL, select your desired quality, and download at maximum speed."
                    color: Theme.textSecondary
                    font.pixelSize: 13
                    font.family: Theme.fontFamily
                }
            }

            // URL Input Area
            UrlInputField {
                width: parent.width
                onAnalyzeRequested: function(rawUrl) {
                    root.currentErrorMessage = ""
                    urlAnalyzer.analyzeUrl(rawUrl)
                }
            }

            // Error banner (if any)
            Rectangle {
                visible: root.currentErrorMessage.length > 0
                width: parent.width
                height: 44
                radius: Theme.radiusMd
                color: Theme.error + "18"
                border.color: Theme.error + "55"
                border.width: 1

                Row {
                    anchors.centerIn: parent
                    spacing: 10
                    Text { text: "⚠️"; font.pixelSize: 14 }
                    Text {
                        text: root.currentErrorMessage
                        color: Theme.error
                        font.pixelSize: 12
                        font.family: Theme.fontFamily
                        font.weight: Font.Medium
                    }
                }
            }

            // Quality & Format Selector (visible when url is analyzed)
            QualitySelector {
                id: qualityBox
                width: parent.width
                visible: urlAnalyzer.currentMediaInfo !== null
                mediaInfo: urlAnalyzer.currentMediaInfo

                onDownloadTriggered: function(url, title, formatId, folder) {
                    downloadManager.addDownload(url, title, formatId, folder)
                    root.navigateToQueue()
                }
            }

            // Quick Info Cards (Supported Formats & FFmpeg)
            Row {
                width: parent.width
                spacing: 16

                Rectangle {
                    width: (parent.width - 16) / 2
                    height: 110
                    radius: Theme.radiusLg
                    color: Theme.bgCard
                    border.color: Theme.borderDim
                    border.width: 1

                    Column {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 6

                        Row {
                            spacing: 8
                            Text { text: "⚡"; font.pixelSize: 16 }
                            Text {
                                text: "High-Speed Stream Engine"
                                color: Theme.textPrimary
                                font.pixelSize: 13
                                font.family: Theme.fontFamily
                                font.weight: Font.DemiBold
                            }
                        }
                        Text {
                            text: "Direct-to-disk streaming architecture ensures blazing fast speeds with zero memory overhead."
                            color: Theme.textMuted
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            wrapMode: Text.WordWrap
                            width: parent.width
                        }
                    }
                }

                Rectangle {
                    width: (parent.width - 16) / 2
                    height: 110
                    radius: Theme.radiusLg
                    color: Theme.bgCard
                    border.color: Theme.borderDim
                    border.width: 1

                    Column {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 6

                        Row {
                            spacing: 8
                            Text { text: "🎧"; font.pixelSize: 16 }
                            Text {
                                text: "320 kbps MP3 Converter"
                                color: Theme.textPrimary
                                font.pixelSize: 13
                                font.family: Theme.fontFamily
                                font.weight: Font.DemiBold
                            }
                        }
                        Text {
                            text: "Integrated FFmpeg processor seamlessly extracts studio-quality MP3 and AAC audio directly from videos."
                            color: Theme.textMuted
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            wrapMode: Text.WordWrap
                            width: parent.width
                        }
                    }
                }
            }
        }
    }
}
