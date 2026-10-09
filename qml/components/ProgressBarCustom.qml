import QtQuick
import MediaGrab

Item {
    id: root
    property real value: 0 // 0 to 100
    property color barColor: Theme.accent
    property color barColorEnd: Theme.accentBlue
    property color bgColor: Theme.bgInput
    property int barHeight: 8
    property bool showText: false

    implicitHeight: showText ? barHeight + 20 : barHeight
    implicitWidth: 200

    Rectangle {
        id: track
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: root.barHeight
        radius: height / 2
        color: root.bgColor

        Rectangle {
            id: fill
            height: parent.height
            radius: height / 2
            width: Math.max(0, Math.min(track.width, track.width * (root.value / 100.0)))

            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: root.barColor }
                GradientStop { position: 1.0; color: root.barColorEnd }
            }

            Behavior on width {
                NumberAnimation {
                    duration: 250
                    easing.type: Easing.OutQuad
                }
            }
        }
    }

    Text {
        visible: root.showText
        anchors.top: track.bottom
        anchors.topMargin: 4
        anchors.right: parent.right
        text: Math.round(root.value) + "%"
        color: Theme.textSecondary
        font.pixelSize: 11
        font.family: Theme.fontFamily
        font.weight: Font.Medium
    }
}
