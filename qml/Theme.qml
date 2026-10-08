pragma Singleton
import QtQuick

QtObject {
    // Backgrounds
    readonly property color bgApp: "#090D16"
    readonly property color bgSidebar: "#0D1322"
    readonly property color bgCard: "#131C2E"
    readonly property color bgCardHover: "#18243B"
    readonly property color bgInput: "#0F172A"
    readonly property color bgElevated: "#1E293B"

    // Accents & Gradients
    readonly property color accent: "#3B82F6"
    readonly property color accentHover: "#60A5FA"
    readonly property color accentPurple: "#8B5CF6"
    readonly property color accentSecondary: "#8B5CF6"
    readonly property color accentPink: "#EC4899"
    readonly property color accentCyan: "#06B6D4"

    readonly property color success: "#10B981"
    readonly property color warning: "#F59E0B"
    readonly property color error: "#EF4444"

    // Text
    readonly property color textPrimary: "#F8FAFC"
    readonly property color textSecondary: "#94A3B8"
    readonly property color textMuted: "#64748B"

    // Borders
    readonly property color borderDim: "#1E293B"
    readonly property color borderBright: "#334155"
    readonly property color borderHighlight: "#3B82F6"

    // Radii
    readonly property int radiusSm: 6
    readonly property int radiusMd: 10
    readonly property int radiusLg: 14
    readonly property int radiusXl: 20

    // Font Family
    readonly property string fontFamily: "Segoe UI, Inter, sans-serif"
}
