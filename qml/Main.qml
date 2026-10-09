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
        playing: true
        targetFps: 60
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

                SettingsView {
                    id: settingsView
                }
            }
        }
    }
}
