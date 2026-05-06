#pragma once
#include <QPoint>
#include <QJsonObject>
#include <QString>

// Represents a single target click point on screen.
// delayMs = how long to wait AFTER clicking this point before clicking the next one.
struct ClickPoint {
    QPoint  position;
    int     delayMs  = 300;
    QString label;   // optional user label

    QString actionKeys; // e.g. "A, B, Space"
    double  keyDelaySecs = 0.5;

    ClickPoint() = default;
    ClickPoint(QPoint pos, int delay = 300, const QString& lbl = {})
        : position(pos), delayMs(delay), label(lbl) {}

    // ---- JSON helpers ------------------------------------------------
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
