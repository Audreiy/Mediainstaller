import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import MediaGrab

Rectangle {
    id: root
    implicitHeight: contentColumn.height + 40
    implicitWidth: 600
    radius: Theme.radiusLg
    color: Theme.bgCard
    border.color: Theme.borderDim
    border.width: 1

    property var mediaInfo: null
    property string selectedFormatType: "video"
    property string selectedVideoQuality: "best_video"
    property string selectedAudioQuality: "audio_mp3_320"
    property string selectedOptionId: "best_video"
    property string customFolder: downloadManager.defaultDownloadPath
    property string customTitle: ""
    property bool isEditingTitle: false

    onMediaInfoChanged: {
        customTitle = mediaInfo ? mediaInfo.title : ""
        isEditingTitle = false
        selectedVideoQuality = "best_video"
        if (mediaInfo && mediaInfo.isAudioOnly) {
            selectedFormatType = "audio"
            selectedOptionId = selectedAudioQuality
        } else {
            selectedFormatType = "video"
            selectedOptionId = "best_video"
        }
    }

    function getVideoQualities() {
        var h = (root.mediaInfo && root.mediaInfo.maxHeight > 0) ? root.mediaInfo.maxHeight : 1080;
        var list = [];

        var bestDesc = (h >= 2160) ? "4K UHD" : ((h >= 1440) ? "2K QHD" : ((h >= 1080) ? "1080p FHD" : ((h >= 720) ? "720p HD" : (h + "p"))));
        list.push({ id: "best_video", label: I18n.t("qualityBest") + " (" + bestDesc + ")" });

        var heights = (root.mediaInfo && root.mediaInfo.availableHeights && root.mediaInfo.availableHeights.length > 0)
                      ? root.mediaInfo.availableHeights
                      : [];

        if (heights.length > 0) {
            for (var i = 0; i < heights.length; ++i) {
                var vh = heights[i];
                var label = "";
                var id = "video_" + vh + "p";
                if (vh >= 2160) { id = "video_4k"; label = "4K (2160p)"; }
                else if (vh >= 1440) { id = "video_1440p"; label = "2K (1440p)"; }
                else if (vh >= 1080) { id = "video_1080p"; label = "1080p Full HD"; }
                else if (vh >= 720) { id = "video_720p"; label = "720p HD"; }
                else if (vh >= 480) { id = "video_480p"; label = "480p SD"; }
                else if (vh >= 360) { id = "video_360p"; label = "360p"; }
                else if (vh >= 240) { id = "video_240p"; label = "240p"; }
                else if (vh >= 144) { id = "video_144p"; label = "144p"; }
                else { label = vh + "p"; }

                var exists = false;
                for (var j = 0; j < list.length; ++j) {
                    if (list[j].id === id) { exists = true; break; }
                }
                if (!exists) {
                    list.push({ id: id, label: label });
                }
            }
        } else {
            if (h >= 2160) list.push({ id: "video_4k", label: "4K (2160p)" });
            if (h >= 1440) list.push({ id: "video_1440p", label: "2K (1440p)" });
            if (h >= 1080) list.push({ id: "video_1080p", label: "1080p Full HD" });
            if (h >= 720) list.push({ id: "video_720p", label: "720p HD" });
            if (h >= 480) list.push({ id: "video_480p", label: "480p SD" });
            if (h >= 360) list.push({ id: "video_360p", label: "360p" });
            if (h < 360) list.push({ id: "video_" + h + "p", label: h + "p" });
        }

        return list;
    }

    signal downloadTriggered(string url, string title, string formatId, string folder)

    Column {
        id: contentColumn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        spacing: 16

        // Header: Thumbnail & Title & Metadata
        Row {
            width: parent.width
            spacing: 14

            // Media Thumbnail Preview
            Rectangle {
                width: 72
                height: 48
                radius: Theme.radiusMd
                color: Theme.bgElevated
                border.color: Theme.borderDim
                border.width: 1
                clip: true

                Image {
                    id: thumbImg
                    anchors.fill: parent
                    source: root.mediaInfo ? root.mediaInfo.thumbnailUrl : ""
                    fillMode: Image.PreserveAspectCrop
                    visible: status === Image.Ready
                }

                Rectangle {
                    anchors.centerIn: parent
                    visible: !thumbImg.visible
                    width: 36; height: 24; radius: 4
                    color: Theme.bgElevated
                    border.color: Theme.borderBright
                    border.width: 1
                    Text {
                        anchors.centerIn: parent
                        text: (root.selectedOptionId === "audio_mp3") ? "MP3" : "MP4"
                        color: Theme.accent
                        font.pixelSize: 11
                        font.family: Theme.fontFamily
                        font.weight: Font.Bold
                    }
                }
            }

            Column {
                width: parent.width - 86
                spacing: 4

                // Video Title (Click to Edit or Leave As Is)
                Item {
                    width: parent.width
                    height: root.isEditingTitle ? 32 : 24

                    // Display Mode (Click to Rename)
                    Rectangle {
                        id: titleDisplayArea
                        visible: !root.isEditingTitle
                        anchors.fill: parent
                        color: titleHover.hovered ? Theme.bgCardHover : "transparent"
                        radius: Theme.radiusSm
                        border.color: titleHover.hovered ? Theme.borderBright : "transparent"
                        border.width: 1

                        HoverHandler { id: titleHover; cursorShape: Qt.PointingHandCursor }
                        TapHandler {
                            onTapped: {
                                root.isEditingTitle = true
                                titleInput.text = (root.customTitle.length > 0) ? root.customTitle : (root.mediaInfo ? root.mediaInfo.title : "")
                                titleInput.forceActiveFocus()
                                titleInput.selectAll()
                            }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 4
                            anchors.rightMargin: 4
                            spacing: 8

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width - (titleHover.hovered ? 68 : 24)
                                text: (root.customTitle.length > 0) ? root.customTitle : (root.mediaInfo ? root.mediaInfo.title : "Media Item")
                                color: Theme.textPrimary
                                font.pixelSize: 14
                                font.family: Theme.fontFamily
                                font.weight: Font.DemiBold
                                elide: Text.ElideMiddle
                            }

                            // Edit Badge
                            Rectangle {
                                anchors.verticalCenter: parent.verticalCenter
                                width: titleHover.hovered ? 60 : 18
                                height: 18
                                radius: 4
                                color: titleHover.hovered ? (Theme.accent + "33") : "transparent"
                                border.color: titleHover.hovered ? Theme.accent : "transparent"
                                border.width: 1

                                Row {
                                    anchors.centerIn: parent
                                    spacing: 3
                                    Text {
                                        text: "✎"
                                        color: titleHover.hovered ? Theme.accent : Theme.textMuted
                                        font.pixelSize: 11
                                    }
                                    Text {
                                        visible: titleHover.hovered
                                        text: I18n.t("editTitleSave") === "Kaydet" ? "Düzenle" : "Edit"
                                        color: Theme.accent
                                        font.pixelSize: 10
                                        font.family: Theme.fontFamily
                                        font.weight: Font.Medium
                                    }
                                }
                            }
                        }
                    }

                    // Edit Mode (Inline Input with Save / Cancel)
                    Rectangle {
                        id: titleEditArea
                        visible: root.isEditingTitle
                        anchors.fill: parent
                        color: Theme.bgInput
                        radius: Theme.radiusSm
                        border.color: Theme.accent
                        border.width: 1.5

                        Row {
                            anchors.fill: parent
                            anchors.margins: 4
                            spacing: 6

                            TextField {
                                id: titleInput
                                width: parent.width - 64
                                height: parent.height
                                text: root.customTitle
                                font.pixelSize: 13
                                font.family: Theme.fontFamily
                                color: Theme.textPrimary
                                verticalAlignment: TextInput.AlignVCenter
                                placeholderText: I18n.t("editTitlePlaceholder")
                                selectByMouse: true
                                background: Rectangle { color: "transparent" }

                                Keys.onReturnPressed: {
                                    if (titleInput.text.trim().length > 0) {
                                        root.customTitle = titleInput.text.trim()
                                    }
                                    root.isEditingTitle = false
                                }
                                Keys.onEscapePressed: {
                                    root.isEditingTitle = false
                                }
                            }

                            // Confirm Button (✓)
                            Rectangle {
                                width: 24; height: 24
                                radius: 4
                                anchors.verticalCenter: parent.verticalCenter
                                color: confirmHover.hovered ? Theme.accentHover : Theme.accent
                                HoverHandler { id: confirmHover; cursorShape: Qt.PointingHandCursor }
                                TapHandler {
                                    onTapped: {
                                        if (titleInput.text.trim().length > 0) {
                                            root.customTitle = titleInput.text.trim()
                                        }
                                        root.isEditingTitle = false
                                    }
                                }
                                Text {
                                    anchors.centerIn: parent
                                    text: "✓"
                                    color: "#FFFFFF"
                                    font.pixelSize: 12
                                    font.bold: true
                                }
                            }

                            // Cancel Button (✕)
                            Rectangle {
                                width: 24; height: 24
                                radius: 4
                                anchors.verticalCenter: parent.verticalCenter
                                color: cancelHover.hovered ? Theme.bgCardHover : Theme.bgElevated
                                border.color: Theme.borderDim
                                border.width: 1
                                HoverHandler { id: cancelHover; cursorShape: Qt.PointingHandCursor }
                                TapHandler {
                                    onTapped: {
                                        root.isEditingTitle = false
                                    }
                                }
                                Text {
                                    anchors.centerIn: parent
                                    text: "✕"
                                    color: Theme.textMuted
                                    font.pixelSize: 11
                                }
                            }
                        }
                    }
                }

                Row {
                    spacing: 8
                    Rectangle {
                        height: 20
                        width: badgeText.width + 12
                        radius: 4
                        color: Theme.accent + "22"
                        border.color: Theme.accent + "66"
                        border.width: 1

                        Text {
                            id: badgeText
                            anchors.centerIn: parent
                            text: (root.mediaInfo && root.mediaInfo.isDirectFile) ? I18n.t("directFileBadge") : "Online Stream"
                            color: Theme.accentHover
                            font.pixelSize: 10
                            font.family: Theme.fontFamily
                            font.weight: Font.Medium
                        }
                    }

                    Text {
                        visible: root.mediaInfo && root.mediaInfo.author.length > 0
                        text: root.mediaInfo ? root.mediaInfo.author : ""
                        color: Theme.accentCyan
                        font.pixelSize: 11
                        font.family: Theme.fontFamily
                        font.weight: Font.Medium
                    }

                    Text {
                        text: root.mediaInfo ? root.mediaInfo.url : ""
                        color: Theme.textMuted
                        font.pixelSize: 11
                        font.family: Theme.fontFamily
                        elide: Text.ElideRight
                        width: 260
                    }
                }
            }
        }

        Rectangle {
            width: parent.width
            height: 1
            color: Theme.borderDim
        }

        // Section: Format / Quality Options
        Text {
            text: I18n.t("chooseQuality")
            color: Theme.textSecondary
            font.pixelSize: 12
            font.family: Theme.fontFamily
            font.weight: Font.DemiBold
        }

        Row {
            width: parent.width
            spacing: 12

            // Option 1: Video (MP4)
            Rectangle {
                id: videoCard
                width: (parent.width - 12) / 2
                height: 70
                radius: Theme.radiusMd
                color: (root.selectedFormatType === "video") ? Theme.activeTabBg : (videoHover.hovered ? Theme.bgCardHover : Theme.bgElevated)
                border.color: (root.selectedFormatType === "video") ? Theme.accent : Theme.borderDim
                border.width: (root.selectedFormatType === "video") ? 2 : 1

                HoverHandler { id: videoHover }
                TapHandler {
                    onTapped: {
                        root.selectedFormatType = "video"
                        root.selectedOptionId = root.selectedVideoQuality
                    }
                }

                Row {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Rectangle {
                        width: 38; height: 26; radius: 6
                        color: (root.selectedFormatType === "video") ? Theme.accent : Theme.bgElevated
                        border.color: (root.selectedFormatType === "video") ? Theme.accent : Theme.borderDim
                        anchors.verticalCenter: parent.verticalCenter
                        Text {
                            anchors.centerIn: parent
                            text: "MP4"
                            color: (root.selectedFormatType === "video") ? "#07090e" : Theme.textPrimary
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            font.weight: Font.Bold
                        }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: I18n.t("videoAudio")
                            color: (root.selectedFormatType === "video") ? Theme.accent : Theme.textPrimary
                            font.pixelSize: 13
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: I18n.t("videoAudioDesc")
                            color: Theme.textMuted
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                        }
                    }
                }
            }

            // Option 2: Audio Only (MP3)
            Rectangle {
                id: audioCard
                width: (parent.width - 12) / 2
                height: 70
                radius: Theme.radiusMd
                color: (root.selectedFormatType === "audio") ? Theme.activeTabBg : (audioHover.hovered ? Theme.bgCardHover : Theme.bgElevated)
                border.color: (root.selectedFormatType === "audio") ? Theme.accent : Theme.borderDim
                border.width: (root.selectedFormatType === "audio") ? 2 : 1

                HoverHandler { id: audioHover }
                TapHandler {
                    onTapped: {
                        root.selectedFormatType = "audio"
                        root.selectedOptionId = root.selectedAudioQuality
                    }
                }

                Row {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Rectangle {
                        width: 38; height: 26; radius: 6
                        color: (root.selectedFormatType === "audio") ? Theme.accent : Theme.bgElevated
                        border.color: (root.selectedFormatType === "audio") ? Theme.accent : Theme.borderDim
                        anchors.verticalCenter: parent.verticalCenter
                        Text {
                            anchors.centerIn: parent
                            text: "MP3"
                            color: (root.selectedFormatType === "audio") ? "#07090e" : Theme.textPrimary
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            font.weight: Font.Bold
                        }
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: I18n.t("audioMp3")
                            color: (root.selectedFormatType === "audio") ? Theme.accent : Theme.textPrimary
                            font.pixelSize: 13
                            font.family: Theme.fontFamily
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: I18n.t("audioMp3Desc")
                            color: Theme.textMuted
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                        }
                    }
                }
            }
        }

        // Sub-Options: Resolution (if MP4) or Bitrate (if MP3)
        Column {
            width: parent.width
            spacing: 8

            Row {
                spacing: 8
                Rectangle {
                    width: 4; height: 14; radius: 2
                    color: Theme.accent
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: root.selectedFormatType === "video" ? I18n.t("selectResolution") : I18n.t("selectBitrate")
                    color: Theme.textSecondary
                    font.pixelSize: 12
                    font.family: Theme.fontFamily
                    font.weight: Font.DemiBold
                }
            }

            // Video Qualities Pills (Dynamic by Video Source & Resolution)
            Flow {
                width: parent.width
                spacing: 8
                visible: root.selectedFormatType === "video"

                Repeater {
                    model: root.getVideoQualities()

                    delegate: Rectangle {
                        required property var modelData
                        width: Math.max(86, pillText.implicitWidth + 24)
                        height: 34
                        radius: Theme.radiusSm
                        color: root.selectedVideoQuality === modelData.id ? Theme.activeTabBg : (vPillHover.hovered ? Theme.bgCardHover : Theme.bgElevated)
                        border.color: root.selectedVideoQuality === modelData.id ? Theme.accent : Theme.borderDim
                        border.width: root.selectedVideoQuality === modelData.id ? 1.5 : 1

                        HoverHandler { id: vPillHover }
                        TapHandler {
                            onTapped: {
                                root.selectedVideoQuality = modelData.id
                                root.selectedOptionId = modelData.id
                            }
                        }

                        Text {
                            id: pillText
                            anchors.centerIn: parent
                            text: modelData.label
                            color: root.selectedVideoQuality === modelData.id ? Theme.accent : Theme.textPrimary
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            font.weight: root.selectedVideoQuality === modelData.id ? Font.Bold : Font.Normal
                        }
                    }
                }
            }

            // Audio Bitrates Pills
            Row {
                width: parent.width
                spacing: 8
                visible: root.selectedFormatType === "audio"

                Repeater {
                    model: [
                        { id: "audio_mp3_320", label: I18n.t("bitrateStudio") },
                        { id: "audio_mp3_256", label: I18n.t("bitrateHigh") },
                        { id: "audio_mp3_192", label: I18n.t("bitrateStandard") },
                        { id: "audio_mp3_128", label: I18n.t("bitrateLight") }
                    ]

                    delegate: Rectangle {
                        required property var modelData
                        width: (parent.width - 24) / 4
                        height: 34
                        radius: Theme.radiusSm
                        color: root.selectedAudioQuality === modelData.id ? Theme.activeTabBg : (aPillHover.hovered ? Theme.bgCardHover : Theme.bgElevated)
                        border.color: root.selectedAudioQuality === modelData.id ? Theme.accent : Theme.borderDim
                        border.width: root.selectedAudioQuality === modelData.id ? 1.5 : 1

                        HoverHandler { id: aPillHover }
                        TapHandler {
                            onTapped: {
                                root.selectedAudioQuality = modelData.id
                                root.selectedOptionId = modelData.id
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: modelData.label
                            color: root.selectedAudioQuality === modelData.id ? Theme.accent : Theme.textPrimary
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            font.weight: root.selectedAudioQuality === modelData.id ? Font.Bold : Font.Normal
                        }
                    }
                }
            }
        }

        // Section: Save Folder & Start Download
        Row {
            width: parent.width
            spacing: 12

            // Folder path
            Rectangle {
                width: parent.width - 160
                height: 44
                radius: Theme.radiusMd
                color: Theme.bgInput
                border.color: Theme.borderDim
                border.width: 1

                Row {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8

                    Rectangle {
                        width: 6; height: 6; radius: 3
                        color: Theme.accent
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: root.customFolder
                        color: Theme.textSecondary
                        font.pixelSize: 12
                        font.family: Theme.fontFamily
                        elide: Text.ElideMiddle
                        width: parent.width - 30
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            // Start Download Button
            Rectangle {
                id: downloadBtn
                width: 148
                height: 44
                radius: Theme.radiusMd

                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: dlHover.hovered ? Theme.accentHover : Theme.accent }
                    GradientStop { position: 1.0; color: dlHover.hovered ? Theme.accentBlueHover : Theme.accentBlue }
                }

                HoverHandler { id: dlHover }

                Row {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        text: "▷"
                        color: "#FFFFFF"
                        font.pixelSize: 14
                        font.bold: true
                    }

                    Text {
                        text: I18n.t("startDownload")
                        color: "#FFFFFF"
                        font.pixelSize: 13
                        font.family: Theme.fontFamily
                        font.weight: Font.Bold
                    }
                }

                TapHandler {
                    onTapped: {
                        if (root.mediaInfo) {
                            if (root.isEditingTitle && titleInput.text.trim().length > 0) {
                                root.customTitle = titleInput.text.trim()
                                root.isEditingTitle = false
                            }
                            var titleToUse = (root.customTitle && root.customTitle.trim().length > 0) ? root.customTitle.trim() : root.mediaInfo.title
                            root.downloadTriggered(
                                root.mediaInfo.url,
                                titleToUse,
                                root.selectedOptionId,
                                root.customFolder
                            )
                        }
                    }
                }
            }
        }
    }
}
