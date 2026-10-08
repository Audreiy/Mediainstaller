import QtQuick
import QtQuick.Controls
import MediaGrab

Item {
    id: root

    Column {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 20

        // Header Line
        Row {
            width: parent.width

            Column {
                spacing: 4
                Text {
                    text: "İndirme Kuyruğu"
                    color: Theme.textPrimary
                    font.pixelSize: 22
                    font.family: Theme.fontFamily
                    font.weight: Font.Bold
                }
                Text {
                    text: queueModel.activeDownloadCount + " aktif işlem • Toplam " + queueModel.totalDownloadCount + " dosya"
                    color: Theme.textSecondary
                    font.pixelSize: 12
                    font.family: Theme.fontFamily
                }
            }

            Item { width: Math.max(10, parent.width - 320) }

            // Clear Completed Button
            Rectangle {
                visible: queueModel.totalDownloadCount > 0
                width: 150
                height: 38
                radius: Theme.radiusMd
                color: clearHover.hovered ? Theme.bgCardHover : Theme.bgElevated
                border.color: Theme.borderDim
                border.width: 1

                HoverHandler { id: clearHover }
                TapHandler { onTapped: queueModel.clearCompleted() }

                Row {
                    anchors.centerIn: parent
                    spacing: 6
                    Text { text: "🧹"; font.pixelSize: 12 }
                    Text {
                        text: "Bitmişleri Temizle"
                        color: clearHover.hovered ? Theme.textPrimary : Theme.textSecondary
                        font.pixelSize: 12
                        font.family: Theme.fontFamily
                        font.weight: Font.Medium
                    }
                }
            }
        }

        // Empty state
        Rectangle {
            visible: queueModel.totalDownloadCount === 0
            width: parent.width
            height: 280
            radius: Theme.radiusLg
            color: Theme.bgCard
            border.color: Theme.borderDim
            border.width: 1

            Column {
                anchors.centerIn: parent
                spacing: 12

                Rectangle {
                    width: 56
                    height: 56
                    radius: 28
                    color: Theme.bgElevated
                    anchors.horizontalCenter: parent.horizontalCenter

                    Text {
                        anchors.centerIn: parent
                        text: "📥"
                        font.pixelSize: 26
                    }
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "İndirme Kuyruğu Boş"
                    color: Theme.textPrimary
                    font.pixelSize: 15
                    font.family: Theme.fontFamily
                    font.weight: Font.DemiBold
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "İndirmek istediğiniz bir bağlantıyı 'İndirici' sekmesinden ekleyin."
                    color: Theme.textMuted
                    font.pixelSize: 12
                    font.family: Theme.fontFamily
                }
            }
        }

        // Active List
        ListView {
            visible: queueModel.totalDownloadCount > 0
            width: parent.width
            height: parent.height - 80
            model: queueModel
            spacing: 10
            clip: true

            delegate: DownloadCard {
                width: parent.width
            }
        }
    }
}
