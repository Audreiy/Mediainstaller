import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
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
                    text: I18n.t("dashTitle")
                    color: Theme.textPrimary
                    font.pixelSize: 24
                    font.family: Theme.fontFamily
                    font.weight: Font.Bold
                }
                Text {
                    text: I18n.t("dashSubtitle")
                    color: Theme.textSecondary
                    font.pixelSize: 13
                    font.family: Theme.fontFamily
                }
            }

            // URL Input Area
            UrlInputField {
                id: urlInputField
                width: parent.width
                onAnalyzeRequested: function(rawUrl) {
                    root.currentErrorMessage = ""
                    urlAnalyzer.analyzeUrl(rawUrl)
                }
            }

            // Error banner (if any)
            Rectangle {
                id: errorBanner
                visible: root.currentErrorMessage.length > 0
                width: parent.width
                implicitHeight: visible ? (errorRow.implicitHeight + 28) : 0
                height: implicitHeight
                radius: Theme.radiusMd
                color: Theme.error + "18"
                border.color: Theme.error + "55"
                border.width: 1

                RowLayout {
                    id: errorRow
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 14
                    spacing: 12

                    Rectangle {
                        width: 22
                        height: 22
                        radius: 11
                        color: Theme.error
                        Layout.alignment: Qt.AlignTop

                        Text {
                            anchors.centerIn: parent
                            text: "!"
                            color: "#ffffff"
                            font.pixelSize: 12
                            font.weight: Font.Bold
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: root.currentErrorMessage
                        color: Theme.error
                        font.pixelSize: 12
                        font.family: Theme.fontFamily
                        font.weight: Font.Medium
                        wrapMode: Text.WordWrap
                        lineHeight: 1.25
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
                    urlInputField.clear()
                    urlAnalyzer.clear()
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
                            Rectangle {
                                width: 6; height: 6; radius: 3
                                color: Theme.accent
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: I18n.t("feature1Title")
                                color: Theme.textPrimary
                                font.pixelSize: 13
                                font.family: Theme.fontFamily
                                font.weight: Font.DemiBold
                            }
                        }
                        Text {
                            text: I18n.t("feature1Desc")
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
                            Rectangle {
                                width: 6; height: 6; radius: 3
                                color: Theme.accentBlue
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: I18n.t("feature2Title")
                                color: Theme.textPrimary
                                font.pixelSize: 13
                                font.family: Theme.fontFamily
                                font.weight: Font.DemiBold
                            }
                        }
                        Text {
                            text: I18n.t("feature2Desc")
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
