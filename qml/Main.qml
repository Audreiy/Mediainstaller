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
    title: "MediaGrab - Media Downloader & Converter"
    color: Theme.bgApp

    // Subtle ambient gradient background glow
    Rectangle {
        width: 400
        height: 400
        radius: 200
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: -100
        opacity: 0.10
        color: Theme.accentPurple
    }

    Rectangle {
        width: 450
        height: 450
        radius: 225
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: -120
        opacity: 0.08
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
