import QtQuick
import MediaGrab

Rectangle {
    id: root
    implicitWidth: 220
    color: Theme.bgSidebar
    border.color: Theme.borderDim
    border.width: 1

    property int activeTab: 0 // 0: Dashboard, 1: Queue, 2: Settings

    Column {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 24

        // Logo & Branding
        Row {
            spacing: 12
            Rectangle {
                width: 38
                height: 38
                radius: Theme.radiusMd
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: Theme.accent }
                    GradientStop { position: 1.0; color: Theme.accentPurple }
                }

                Text {
                    anchors.centerIn: parent
                    text: "⚡"
                    font.pixelSize: 18
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 2
                Text {
                    text: "MediaGrab"
                    color: Theme.textPrimary
                    font.pixelSize: 17
                    font.family: Theme.fontFamily
                    font.weight: Font.Bold
                }
                Text {
                    text: "v1.0 Pro"
                    color: Theme.accentHover
                    font.pixelSize: 10
                    font.family: Theme.fontFamily
                    font.weight: Font.DemiBold
                }
            }
        }

        Rectangle {
            width: parent.width
            height: 1
            color: Theme.borderDim
        }

        // Navigation Menu
        Column {
            width: parent.width
            spacing: 8

            // Tab 0: Dashboard (İndirici)
            Rectangle {
                width: parent.width
                height: 44
                radius: Theme.radiusMd
                color: (root.activeTab === 0) ? (Theme.accent + "22") : (tab0Hover.hovered ? Theme.bgCardHover : "transparent")
                border.color: (root.activeTab === 0) ? (Theme.accent + "55") : "transparent"
                border.width: 1

                HoverHandler { id: tab0Hover }
                TapHandler { onTapped: root.activeTab = 0 }

                Row {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Text {
                        text: "📥"
                        font.pixelSize: 16
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "Downloader"
                        color: (root.activeTab === 0) ? Theme.textPrimary : Theme.textSecondary
                        font.pixelSize: 13
                        font.family: Theme.fontFamily
                        font.weight: (root.activeTab === 0) ? Font.DemiBold : Font.Normal
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            // Tab 1: Queue
            Rectangle {
                width: parent.width
                height: 44
                radius: Theme.radiusMd
                color: (root.activeTab === 1) ? (Theme.accent + "22") : (tab1Hover.hovered ? Theme.bgCardHover : "transparent")
                border.color: (root.activeTab === 1) ? (Theme.accent + "55") : "transparent"
                border.width: 1

                HoverHandler { id: tab1Hover }
                TapHandler { onTapped: root.activeTab = 1 }

                Row {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Text {
                        text: "📋"
                        font.pixelSize: 16
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "Queue"
                        color: (root.activeTab === 1) ? Theme.textPrimary : Theme.textSecondary
                        font.pixelSize: 13
                        font.family: Theme.fontFamily
                        font.weight: (root.activeTab === 1) ? Font.DemiBold : Font.Normal
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Item { width: 10; height: 1 }

                    // Badge for active downloads count
                    Rectangle {
                        visible: queueModel.activeDownloadCount > 0
                        width: 20
                        height: 20
                        radius: 10
                        color: Theme.accent
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            anchors.centerIn: parent
                            text: queueModel.activeDownloadCount
                            color: "#FFFFFF"
                            font.pixelSize: 10
                            font.weight: Font.Bold
                        }
                    }
                }
            }

            // Tab 2: Settings
            Rectangle {
                width: parent.width
                height: 44
                radius: Theme.radiusMd
                color: (root.activeTab === 2) ? (Theme.accent + "22") : (tab2Hover.hovered ? Theme.bgCardHover : "transparent")
                border.color: (root.activeTab === 2) ? (Theme.accent + "55") : "transparent"
                border.width: 1

                HoverHandler { id: tab2Hover }
                TapHandler { onTapped: root.activeTab = 2 }

                Row {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Text {
                        text: "⚙"
                        font.pixelSize: 16
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "Settings"
                        color: (root.activeTab === 2) ? Theme.textPrimary : Theme.textSecondary
                        font.pixelSize: 13
                        font.family: Theme.fontFamily
                        font.weight: (root.activeTab === 2) ? Font.DemiBold : Font.Normal
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }

        Item { height: parent.height - 320; width: 1 }

        // Bottom status badge: FFmpeg & C++ Core
        Rectangle {
            width: parent.width
            height: 52
            radius: Theme.radiusMd
            color: Theme.bgElevated
            border.color: Theme.borderDim
            border.width: 1

            Row {
                anchors.centerIn: parent
                spacing: 8

                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: (ffmpegProcessor.isFFmpegAvailable && ffmpegProcessor.isYtDlpAvailable) ? Theme.success : (ffmpegProcessor.isFFmpegAvailable ? Theme.accentHover : Theme.warning)
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: (ffmpegProcessor.isFFmpegAvailable && ffmpegProcessor.isYtDlpAvailable) ? "Core Engine Ready" : (ffmpegProcessor.isFFmpegAvailable ? "FFmpeg Ready" : "Engine Missing")
                    color: Theme.textSecondary
                    font.pixelSize: 11
                    font.family: Theme.fontFamily
                    font.weight: Font.Medium
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}
