#include "configmanager.h"
#include "appsettings.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QString ConfigManager::defaultConfigPath()
{
    return AppSettings::instance().configFilePath("autoclicker_config.json");
}

QString ConfigManager::defaultProConfigPath()
{
    return AppSettings::instance().configFilePath("autoclicker_pro_config.json");
}

bool ConfigManager::save(const AppConfig& cfg, const QString& filePath)
{
    const QString path = filePath.isEmpty() ? defaultConfigPath() : filePath;

    QJsonObject root;

    // Points
    QJsonArray pts;
    for (const auto& p : cfg.points)
        pts.append(p.toJson());
    root["points"] = pts;

    // Stop condition
    root["stopCondition"] = static_cast<int>(cfg.stopCondition);
    root["timeLimitSecs"] = cfg.timeLimitSecs;
    root["maxCycles"]     = cfg.maxCycles;

    // Hotkey
    QJsonObject hk;
    hk["key"]       = cfg.toggleHotkey.key;
    hk["modifiers"] = cfg.toggleHotkey.modifiers;
    root["toggleHotkey"] = hk;
    root["hideOverlayWhenRunning"] = cfg.hideOverlayWhenRunning;

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool ConfigManager::load(AppConfig& cfg, const QString& filePath)
{
    const QString path = filePath.isEmpty() ? defaultConfigPath() : filePath;

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError)
        return false;

    const QJsonObject root = doc.object();

    // Points
    cfg.points.clear();
    for (const auto& v : root["points"].toArray())
        cfg.points.append(ClickPoint::fromJson(v.toObject()));

    // Stop condition
    cfg.stopCondition = static_cast<StopCondition>(root["stopCondition"].toInt(0));
    cfg.timeLimitSecs = root["timeLimitSecs"].toInt(60);
    cfg.maxCycles     = root["maxCycles"].toInt(10);

    // Hotkey
    if (root.contains("toggleHotkey")) {
        const QJsonObject hk = root["toggleHotkey"].toObject();
        cfg.toggleHotkey.key       = hk["key"].toInt(Qt::Key_F6);
        cfg.toggleHotkey.modifiers = hk["modifiers"].toInt(0);
    }

    cfg.hideOverlayWhenRunning = root["hideOverlayWhenRunning"].toBool(false);

    return true;
}

QString ConfigManager::defaultNormalConfigPath()
{
    return AppSettings::instance().configFilePath("autoclicker_normal_config.json");
}

bool ConfigManager::saveNormal(const NormalAppConfig& cfg, const QString& filePath)
{
    const QString path = filePath.isEmpty() ? defaultNormalConfigPath() : filePath;

    QJsonObject root;
    root["hours"] = cfg.hours;
    root["mins"]  = cfg.mins;
    root["secs"]  = cfg.secs;
    root["ms"]    = cfg.ms;

    root["buttonType"] = static_cast<int>(cfg.buttonType);
    root["clickType"]  = static_cast<int>(cfg.clickType);

    root["repeatUntilStopped"] = cfg.repeatUntilStopped;
    root["repeatTimes"]        = cfg.repeatTimes;

    root["positionType"] = static_cast<int>(cfg.positionType);
    root["pickedX"]      = cfg.pickedX;
    root["pickedY"]      = cfg.pickedY;

    QJsonObject hk;
    hk["key"]       = cfg.toggleHotkey.key;
    hk["modifiers"] = cfg.toggleHotkey.modifiers;
    root["toggleHotkey"] = hk;

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;

    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool ConfigManager::loadNormal(NormalAppConfig& cfg, const QString& filePath)
{
    const QString path = filePath.isEmpty() ? defaultNormalConfigPath() : filePath;

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError)
        return false;

    const QJsonObject root = doc.object();

    cfg.hours = root["hours"].toInt(0);
    cfg.mins  = root["mins"].toInt(0);
    cfg.secs  = root["secs"].toInt(0);
    cfg.ms    = root["ms"].toInt(100);

    cfg.buttonType = static_cast<MouseButtonType>(root["buttonType"].toInt(static_cast<int>(MouseButtonType::Left)));
    cfg.clickType  = static_cast<ClickType>(root["clickType"].toInt(static_cast<int>(ClickType::Single)));

    cfg.repeatUntilStopped = root["repeatUntilStopped"].toBool(true);
    cfg.repeatTimes        = root["repeatTimes"].toInt(1);

    cfg.positionType = static_cast<CursorPositionType>(root["positionType"].toInt(static_cast<int>(CursorPositionType::Current)));
    cfg.pickedX      = root["pickedX"].toInt(0);
    cfg.pickedY      = root["pickedY"].toInt(0);

    if (root.contains("toggleHotkey")) {
        const QJsonObject hk = root["toggleHotkey"].toObject();
        cfg.toggleHotkey.key       = hk["key"].toInt(Qt::Key_F6);
        cfg.toggleHotkey.modifiers = hk["modifiers"].toInt(0);
    }

    return true;
}
