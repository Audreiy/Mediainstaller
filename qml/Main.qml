import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MediaGrab

ApplicationWindow {
    id: window
    width: 1060
    height: 700
    minimumWidth: 840
    minimumHeight: 560
    visible: true
    title: I18n.t("appName") + " - " + I18n.t("appSubtitle")
    // Base dark fallback
    Rectangle {
        anchors.fill: parent
        color: "#0a0d13"
    }

    // Looping Video Background Layer (Direct Hardware Accelerated)
    VideoBackground {
        id: bgVideo
        anchors.fill: parent
        source: "assets/bg.mp4"
        playing: window.active && downloadManager.videoBackgroundEnabled
        targetFps: 45
    }

    // Frosted dark overlay for high contrast readability
    Rectangle {
        anchors.fill: parent
        color: "#070a12"
        opacity: 0.50
    }

    // Subtle ambient gradient background glow
    Rectangle {
        width: 420
        height: 420
        radius: 210
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: -120
        opacity: 0.08
        color: Theme.accentBlue
    }

    Rectangle {
        width: 380
        height: 380
        radius: 190
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: -100
        opacity: 0.06
        color: Theme.accent
    }

    Row {
        anchors.fill: parent

        // Sidebar
        Sidebar {
            id: sidebarNav
            height: parent.height
            width: 220
        }

        // Main View Stack
        Item {
            width: parent.width - sidebarNav.width
            height: parent.height

            StackLayout {
                anchors.fill: parent
                currentIndex: sidebarNav.activeTab

                DashboardView {
                    id: dashboardView
                    onNavigateToQueue: sidebarNav.activeTab = 1
                }

                QueueView {
                    id: queueView
                }

                UpdatesView {
                    id: updatesView
                }

                SettingsView {
                    id: settingsView
                }
            }
        }
    }

    // Floating Update Notification Banner
    Rectangle {
        id: updateBanner
        property bool dismissed: false
        visible: !dismissed && (typeof updateManager !== "undefined" && updateManager.updateAvailable && sidebarNav.activeTab !== 2)
        height: 48
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 20
        width: bannerRow.implicitWidth + 32
        radius: 24
        color: "#141c2b"
        border.color: Theme.accent
        border.width: 1

        Row {
            id: bannerRow
            anchors.centerIn: parent
            spacing: 12

            Text {
                text: (typeof updateManager !== "undefined" && updateManager.latestVersion) ? I18n.t("updateBannerTitle").arg(updateManager.latestVersion) : ""
                color: Theme.textPrimary
                font.pixelSize: 12
                font.family: Theme.fontFamily
                font.weight: Font.DemiBold
                anchors.verticalCenter: parent.verticalCenter
            }

            Rectangle {
                width: 76; height: 28; radius: 14
                color: updateBtnMouse.containsMouse ? Theme.accentHover : Theme.accent
                anchors.verticalCenter: parent.verticalCenter

                MouseArea {
                    id: updateBtnMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: sidebarNav.activeTab = 2
                }

                Text {
                    anchors.centerIn: parent
                    text: I18n.t("updateBannerAction")
                    color: "#07090e"
                    font.pixelSize: 11
                    font.family: Theme.fontFamily
                    font.weight: Font.Bold
                }
            }

            Text {
                text: "✕"
                color: closeBannerMouse.containsMouse ? Theme.textPrimary : Theme.textMuted
                font.pixelSize: 12
                font.weight: Font.Bold
                anchors.verticalCenter: parent.verticalCenter

                MouseArea {
                    id: closeBannerMouse
                    anchors.fill: parent
                    anchors.margins: -6
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: updateBanner.dismissed = true
                }
            }
        }
    }
}
