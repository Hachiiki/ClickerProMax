#pragma once
#include <QPoint>
#include <QJsonObject>
#include <QString>

/**
 * @struct ClickPoint
 * @brief Represents a single target click point in the automation sequence
 *
 * Encapsulates all data for one position in the click sequence, including:
 * - Screen coordinates of the target
 * - Delay to apply after clicking
 * - Optional user-provided label for UI display
 * - Optional keyboard actions to perform at this point
 *
 * Includes JSON serialization/deserialization for saving/loading click sequences.
 */
struct ClickPoint {
    QPoint  position;       ///< Screen coordinates (in pixels) where to click
    int     delayMs  = 300; ///< Milliseconds to wait AFTER clicking before proceeding to next point
    QString label;          ///< Optional user-friendly name for this point (displayed in UI)

    QString actionKeys;     ///< Optional keyboard keys to press at this point (e.g., "A, B, Space")
    double  keyDelaySecs = 0.5;  ///< Delay (in seconds) between each key press in actionKeys sequence

    ClickPoint() = default;
    /**
     * @brief Constructs a ClickPoint with position and optional delay/label
     * @param pos Screen coordinates for the click target
     * @param delay Milliseconds to wait after clicking (default: 300 ms)
     * @param lbl Optional display name for this point
     */
    ClickPoint(QPoint pos, int delay = 300, const QString& lbl = {})
        : position(pos), delayMs(delay), label(lbl) {}

    /**
     * @brief Serializes this ClickPoint to JSON format
     * @return QJsonObject containing all point data
     * @see fromJson() for deserialization
     */
    QJsonObject toJson() const {
        QJsonObject o;
        o["x"]     = position.x();
        o["y"]     = position.y();
        o["delay"] = delayMs;
        o["label"] = label;
        o["actionKeys"] = actionKeys;
        o["keyDelaySecs"] = keyDelaySecs;
        return o;
    }

    /**
     * @brief Deserializes a ClickPoint from JSON format
     * @param o QJsonObject containing click point data
     * @return Reconstructed ClickPoint with default values for any missing fields
     * @see toJson() for serialization
     */
    static ClickPoint fromJson(const QJsonObject& o) {
        ClickPoint p;
        p.position = QPoint(o["x"].toInt(0), o["y"].toInt(0));
        p.delayMs  = o["delay"].toInt(300);
        p.label    = o["label"].toString();
        p.actionKeys = o["actionKeys"].toString("");
        p.keyDelaySecs = o["keyDelaySecs"].toDouble(0.5);
        return p;
    }
};
