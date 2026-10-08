import QtQuick
import QtQuick.Controls
import MediaGrab

Item {
    id: root
    implicitHeight: 70
    implicitWidth: 600

    property alias text: textInput.text
    signal analyzeRequested(string url)

    Rectangle {
        id: container
        anchors.fill: parent
        radius: Theme.radiusLg
        color: Theme.bgInput
        border.color: textInput.activeFocus ? Theme.borderHighlight : (containerHover.hovered ? Theme.borderBright : Theme.borderDim)
        border.width: textInput.activeFocus ? 2 : 1

        Behavior on border.color {
            ColorAnimation { duration: 150 }
        }

        HoverHandler { id: containerHover }

        Row {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 10
            anchors.topMargin: 8
            anchors.bottomMargin: 8
            spacing: 12

            // Link Icon
            Text {
                text: "🔗"
                font.pixelSize: 20
                anchors.verticalCenter: parent.verticalCenter
                opacity: 0.7
            }

            // Input Field
            TextInput {
                id: textInput
                width: parent.width - 240
                anchors.verticalCenter: parent.verticalCenter
                color: Theme.textPrimary
                font.pixelSize: 14
                font.family: Theme.fontFamily
                selectByMouse: true
                selectionColor: Theme.accent
                selectedTextColor: "#FFFFFF"
                clip: true

                Text {
                    anchors.fill: parent
                    verticalAlignment: Text.AlignVCenter
                    text: "Paste video or audio link here (https://...)"
                    color: Theme.textMuted
                    font.pixelSize: 14
                    font.family: Theme.fontFamily
                    visible: !textInput.text && !textInput.activeFocus
                }

                Keys.onReturnPressed: {
                    if (textInput.text.trim().length > 0 && !urlAnalyzer.isAnalyzing) {
                        root.analyzeRequested(textInput.text.trim())
                    }
                }
            }

            // Paste Button
            Rectangle {
                id: pasteBtn
                width: 70
                height: 38
                anchors.verticalCenter: parent.verticalCenter
                radius: Theme.radiusMd
                color: pasteHover.hovered ? Theme.bgCardHover : Theme.bgElevated
                border.color: Theme.borderDim
                border.width: 1

                HoverHandler { id: pasteHover }

                Text {
                    anchors.centerIn: parent
                    text: "Paste"
                    color: pasteHover.hovered ? Theme.textPrimary : Theme.textSecondary
                    font.pixelSize: 12
                    font.family: Theme.fontFamily
                    font.weight: Font.Medium
                }

                TapHandler {
                    onTapped: {
                        textInput.paste()
                        if (textInput.text.trim().length > 0) {
                            root.analyzeRequested(textInput.text.trim())
                        }
                    }
                }
            }

            // Analyze Button
            Rectangle {
                id: analyzeBtn
                width: 110
                height: 42
                anchors.verticalCenter: parent.verticalCenter
                radius: Theme.radiusMd
                enabled: !urlAnalyzer.isAnalyzing && textInput.text.trim().length > 0
                opacity: enabled ? 1.0 : 0.6

                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: analyzeHover.hovered ? Theme.accentHover : Theme.accent }
                    GradientStop { position: 1.0; color: analyzeHover.hovered ? Theme.accentPurple : Theme.accentSecondary }
                }

                HoverHandler { id: analyzeHover }

                Row {
                    anchors.centerIn: parent
                    spacing: 6

                    Text {
                        id: spinner
                        text: "⟳"
                        color: "#FFFFFF"
                        font.pixelSize: 16
                        visible: urlAnalyzer.isAnalyzing

                        NumberAnimation on rotation {
                            from: 0; to: 360
                            duration: 900
                            loops: Animation.Infinite
                            running: urlAnalyzer.isAnalyzing
                        }
                    }

                    Text {
                        text: urlAnalyzer.isAnalyzing ? "Analyzing..." : "Analyze"
                        color: "#FFFFFF"
                        font.pixelSize: 13
                        font.family: Theme.fontFamily
                        font.weight: Font.DemiBold
                    }
                }

                TapHandler {
                    onTapped: {
                        if (textInput.text.trim().length > 0 && !urlAnalyzer.isAnalyzing) {
                            root.analyzeRequested(textInput.text.trim())
                        }
                    }
                }
            }
        }
    }
}
