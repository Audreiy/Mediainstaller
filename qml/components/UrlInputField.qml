import QtQuick
import QtQuick.Controls
import MediaGrab

Item {
    id: root
    implicitHeight: 70
    implicitWidth: 600

    property alias text: textInput.text
    signal analyzeRequested(string url)

    function clear() {
        textInput.text = ""
    }

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

            // Link Vector Icon
            Item {
                width: 20; height: 20
                anchors.verticalCenter: parent.verticalCenter
                Canvas {
                    anchors.fill: parent
                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.reset();
                        ctx.strokeStyle = Theme.accent;
                        ctx.lineWidth = 1.8;
                        ctx.lineCap = "round";

                        // Left link loop
                        ctx.beginPath();
                        ctx.arc(7.5, 10, 3.8, Math.PI * 0.5, Math.PI * 1.5);
                        ctx.stroke();

                        // Right link loop
                        ctx.beginPath();
                        ctx.arc(12.5, 10, 3.8, -Math.PI * 0.5, Math.PI * 0.5);
                        ctx.stroke();

                        // Center connecting bar
                        ctx.beginPath();
                        ctx.moveTo(7, 10);
                        ctx.lineTo(13, 10);
                        ctx.stroke();
                    }
                }
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
                    text: I18n.t("urlPlaceholder")
                    color: Theme.textMuted
                    font.pixelSize: 13
                    font.family: Theme.fontFamily
                    elide: Text.ElideRight
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
                    text: I18n.t("pasteBtn")
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
                    GradientStop { position: 1.0; color: analyzeHover.hovered ? Theme.accentBlueHover : Theme.accentBlue }
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
                        text: urlAnalyzer.isAnalyzing ? "..." : I18n.t("analyzeBtn")
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
