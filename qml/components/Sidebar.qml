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
        anchors.margins: 16
        spacing: 24

        // Logo & Branding
        Row {
            spacing: 12
            Rectangle {
                width: 42
                height: 42
                radius: 12
                color: Theme.bgElevated
                border.color: Theme.borderBright
                border.width: 1
                clip: true

                Image {
                    anchors.fill: parent
                    anchors.margins: 2
                    source: "qrc:/qt/qml/MediaGrab/assets/logo.png"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    mipmap: true
                }
            }

            Column {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 2
                Text {
                    text: I18n.t("appName")
                    color: Theme.textPrimary
                    font.pixelSize: 15
                    font.family: Theme.fontFamily
                    font.weight: Font.Bold
                }
                Text {
                    text: I18n.t("appVersion")
                    color: Theme.accent
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
            spacing: 6

            // Tab 0: Dashboard (Downloader)
            Rectangle {
                width: parent.width
                height: 42
                radius: Theme.radiusMd
                color: (root.activeTab === 0) ? Theme.activeTabBg : (tab0Mouse.containsMouse ? Theme.bgCardHover : "transparent")
                border.color: (root.activeTab === 0) ? Theme.activeTabBorder : "transparent"
                border.width: 1

                // Left vertical neon indicator line (like the screenshot)
                Rectangle {
                    width: 3
                    height: 18
                    radius: 2
                    color: Theme.activeTabIndicator
                    anchors.left: parent.left
                    anchors.leftMargin: 2
                    anchors.verticalCenter: parent.verticalCenter
                    visible: root.activeTab === 0
                }

                MouseArea {
                    id: tab0Mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.activeTab = 0
                }

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 12
                    spacing: 12

                    Item {
                        width: 18; height: 18
                        anchors.verticalCenter: parent.verticalCenter
                        Canvas {
                            id: tab0Canvas
                            anchors.fill: parent
                            onPaint: {
                                var ctx = getContext("2d");
                                ctx.reset();
                                ctx.strokeStyle = (root.activeTab === 0) ? Theme.accent : (tab0Mouse.containsMouse ? Theme.textPrimary : Theme.textSecondary);
                                ctx.fillStyle = ctx.strokeStyle;
                                ctx.lineWidth = 1.75;
                                ctx.lineCap = "round";
                                ctx.lineJoin = "round";

                                ctx.beginPath();
                                ctx.moveTo(9, 2);
                                ctx.lineTo(9, 11);
                                ctx.moveTo(5.5, 7.5);
                                ctx.lineTo(9, 11);
                                ctx.lineTo(12.5, 7.5);
                                ctx.stroke();

                                ctx.beginPath();
                                ctx.moveTo(3, 11);
                                ctx.lineTo(3, 15);
                                ctx.lineTo(15, 15);
                                ctx.lineTo(15, 11);
                                ctx.stroke();
                            }
                            Connections {
                                target: root
                                function onActiveTabChanged() { tab0Canvas.requestPaint(); }
                            }
                            Connections {
                                target: tab0Mouse
                                function onContainsMouseChanged() { tab0Canvas.requestPaint(); }
                            }
                        }
                    }

                    Text {
                        text: I18n.t("navDownloader")
                        color: (root.activeTab === 0) ? Theme.activeTabText : (tab0Mouse.containsMouse ? Theme.textPrimary : Theme.textSecondary)
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
                height: 42
                radius: Theme.radiusMd
                color: (root.activeTab === 1) ? Theme.activeTabBg : (tab1Mouse.containsMouse ? Theme.bgCardHover : "transparent")
                border.color: (root.activeTab === 1) ? Theme.activeTabBorder : "transparent"
                border.width: 1

                // Left vertical neon indicator line
                Rectangle {
                    width: 3
                    height: 18
                    radius: 2
                    color: Theme.activeTabIndicator
                    anchors.left: parent.left
                    anchors.leftMargin: 2
                    anchors.verticalCenter: parent.verticalCenter
                    visible: root.activeTab === 1
                }

                MouseArea {
                    id: tab1Mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.activeTab = 1
                }

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 12
                    spacing: 12

                    Item {
                        width: 18; height: 18
                        anchors.verticalCenter: parent.verticalCenter
                        Canvas {
                            id: tab1Canvas
                            anchors.fill: parent
                            onPaint: {
                                var ctx = getContext("2d");
                                ctx.reset();
                                ctx.strokeStyle = (root.activeTab === 1) ? Theme.accent : (tab1Mouse.containsMouse ? Theme.textPrimary : Theme.textSecondary);
                                ctx.fillStyle = ctx.strokeStyle;
                                ctx.lineWidth = 1.75;
                                ctx.lineCap = "round";

                                ctx.beginPath();
                                ctx.moveTo(7, 4);
                                ctx.lineTo(15, 4);
                                ctx.stroke();
                                ctx.beginPath();
                                ctx.arc(3.5, 4, 1.2, 0, Math.PI * 2);
                                ctx.fill();

                                ctx.beginPath();
                                ctx.moveTo(7, 9);
                                ctx.lineTo(15, 9);
                                ctx.stroke();
                                ctx.beginPath();
                                ctx.arc(3.5, 9, 1.2, 0, Math.PI * 2);
                                ctx.fill();

                                ctx.beginPath();
                                ctx.moveTo(7, 14);
                                ctx.lineTo(15, 14);
                                ctx.stroke();
                                ctx.beginPath();
                                ctx.arc(3.5, 14, 1.2, 0, Math.PI * 2);
                                ctx.fill();
                            }
                            Connections {
                                target: root
                                function onActiveTabChanged() { tab1Canvas.requestPaint(); }
                            }
                            Connections {
                                target: tab1Mouse
                                function onContainsMouseChanged() { tab1Canvas.requestPaint(); }
                            }
                        }
                    }

                    Text {
                        text: I18n.t("navQueue")
                        color: (root.activeTab === 1) ? Theme.activeTabText : (tab1Mouse.containsMouse ? Theme.textPrimary : Theme.textSecondary)
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
                            color: "#07090e"
                            font.pixelSize: 10
                            font.weight: Font.Bold
                        }
                    }
                }
            }

            // Tab 2: Updates
            Rectangle {
                width: parent.width
                height: 42
                radius: Theme.radiusMd
                color: (root.activeTab === 2) ? Theme.activeTabBg : (tab2Mouse.containsMouse ? Theme.bgCardHover : "transparent")
                border.color: (root.activeTab === 2) ? Theme.activeTabBorder : "transparent"
                border.width: 1

                // Left vertical neon indicator line
                Rectangle {
                    width: 3
                    height: 18
                    radius: 2
                    color: Theme.activeTabIndicator
                    anchors.left: parent.left
                    anchors.leftMargin: 2
                    anchors.verticalCenter: parent.verticalCenter
                    visible: root.activeTab === 2
                }

                MouseArea {
                    id: tab2Mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.activeTab = 2
                }

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 12
                    spacing: 12

                    Item {
                        width: 18; height: 18
                        anchors.verticalCenter: parent.verticalCenter
                        Canvas {
                            id: tab2Canvas
                            anchors.fill: parent
                            onPaint: {
                                var ctx = getContext("2d");
                                ctx.reset();
                                ctx.strokeStyle = (root.activeTab === 2) ? Theme.accent : (tab2Mouse.containsMouse ? Theme.textPrimary : Theme.textSecondary);
                                ctx.fillStyle = ctx.strokeStyle;
                                ctx.lineWidth = 1.75;
                                ctx.lineCap = "round";

                                // Arc reload / loop
                                ctx.beginPath();
                                ctx.arc(9, 9, 5.5, -Math.PI * 0.35, Math.PI * 1.15);
                                ctx.stroke();

                                // Arrow tip
                                ctx.beginPath();
                                ctx.moveTo(13.5, 4);
                                ctx.lineTo(15.5, 4);
                                ctx.lineTo(15.5, 6.5);
                                ctx.stroke();

                                // Center sparkle
                                ctx.beginPath();
                                ctx.arc(9, 9, 1.2, 0, Math.PI * 2);
                                ctx.fill();
                            }
                            Connections {
                                target: root
                                function onActiveTabChanged() { tab2Canvas.requestPaint(); }
                            }
                            Connections {
                                target: tab2Mouse
                                function onContainsMouseChanged() { tab2Canvas.requestPaint(); }
                            }
                        }
                    }

                    Text {
                        text: I18n.t("navUpdates")
                        color: (root.activeTab === 2) ? Theme.activeTabText : (tab2Mouse.containsMouse ? Theme.textPrimary : Theme.textSecondary)
                        font.pixelSize: 13
                        font.family: Theme.fontFamily
                        font.weight: (root.activeTab === 2) ? Font.DemiBold : Font.Normal
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    // Neon update notification badge with gentle breathing pulse
                    Item {
                        width: 14; height: 14
                        anchors.verticalCenter: parent.verticalCenter
                        visible: (typeof updateManager !== "undefined" && updateManager.updateAvailable)

                        Rectangle {
                            anchors.centerIn: parent
                            width: 12; height: 12; radius: 6
                            color: Theme.accent
                            opacity: 0.35

                            SequentialAnimation on scale {
                                running: (typeof updateManager !== "undefined" && updateManager.updateAvailable)
                                loops: Animation.Infinite
                                NumberAnimation { from: 0.8; to: 1.35; duration: 1100; easing.type: Easing.InOutQuad }
                                NumberAnimation { from: 1.35; to: 0.8; duration: 1100; easing.type: Easing.InOutQuad }
                            }
                        }

                        Rectangle {
                            anchors.centerIn: parent
                            width: 7; height: 7; radius: 3.5
                            color: Theme.accent
                        }
                    }
                }
            }

            // Tab 3: Settings
            Rectangle {
                width: parent.width
                height: 42
                radius: Theme.radiusMd
                color: (root.activeTab === 3) ? Theme.activeTabBg : (tab3Mouse.containsMouse ? Theme.bgCardHover : "transparent")
                border.color: (root.activeTab === 3) ? Theme.activeTabBorder : "transparent"
                border.width: 1

                // Left vertical neon indicator line
                Rectangle {
                    width: 3
                    height: 18
                    radius: 2
                    color: Theme.activeTabIndicator
                    anchors.left: parent.left
                    anchors.leftMargin: 2
                    anchors.verticalCenter: parent.verticalCenter
                    visible: root.activeTab === 3
                }

                MouseArea {
                    id: tab3Mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.activeTab = 3
                }

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 12
                    spacing: 12

                    Item {
                        width: 18; height: 18
                        anchors.verticalCenter: parent.verticalCenter
                        Canvas {
                            id: tab3Canvas
                            anchors.fill: parent
                            onPaint: {
                                var ctx = getContext("2d");
                                ctx.reset();
                                ctx.strokeStyle = (root.activeTab === 3) ? Theme.accent : (tab3Mouse.containsMouse ? Theme.textPrimary : Theme.textSecondary);
                                ctx.lineWidth = 1.75;
                                ctx.lineCap = "round";

                                ctx.beginPath();
                                ctx.arc(9, 9, 3.2, 0, Math.PI * 2);
                                ctx.stroke();

                                for (var i = 0; i < 6; i++) {
                                    var angle = i * Math.PI / 3;
                                    var x1 = 9 + Math.cos(angle) * 4.5;
                                    var y1 = 9 + Math.sin(angle) * 4.5;
                                    var x2 = 9 + Math.cos(angle) * 7.5;
                                    var y2 = 9 + Math.sin(angle) * 7.5;
                                    ctx.beginPath();
                                    ctx.moveTo(x1, y1);
                                    ctx.lineTo(x2, y2);
                                    ctx.stroke();
                                }
                            }
                            Connections {
                                target: root
                                function onActiveTabChanged() { tab3Canvas.requestPaint(); }
                            }
                            Connections {
                                target: tab3Mouse
                                function onContainsMouseChanged() { tab3Canvas.requestPaint(); }
                            }
                        }
                    }

                    Text {
                        text: I18n.t("navSettings")
                        color: (root.activeTab === 3) ? Theme.activeTabText : (tab3Mouse.containsMouse ? Theme.textPrimary : Theme.textSecondary)
                        font.pixelSize: 13
                        font.family: Theme.fontFamily
                        font.weight: (root.activeTab === 3) ? Font.DemiBold : Font.Normal
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }


    }
}
