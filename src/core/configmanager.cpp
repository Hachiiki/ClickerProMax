#include "configmanager.h"
#include "appsettings.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/**
 * @brief Gets the default config file path for multi-point automation mode
 * @return Absolute path to "autoclicker_config.json" in the config directory
 */
QString ConfigManager::defaultConfigPath()
{
    return AppSettings::instance().configFilePath("autoclicker_config.json");
}

/**
 * @brief Gets the default config file path for Pro/Advanced automation mode
 * @return Absolute path to "autoclicker_pro_config.json" in the config directory
 */
QString ConfigManager::defaultProConfigPath()
{
    return AppSettings::instance().configFilePath("autoclicker_pro_config.json");
}

/**
 * @brief Serializes an AppConfig to JSON and writes it to disk
 * @details Converts the configuration snapshot (points, stop conditions, hotkey)
 * to indented JSON format for human readability and debugging.
 * @param cfg The AppConfig to save
 * @param filePath Optional custom file path; uses defaultConfigPath() if empty
 * @return True if write succeeded, false if file operations failed
 */
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

/**
 * @brief Loads an AppConfig from a JSON file on disk
 * @details Reads and parses JSON, reconstructs all ClickPoints, stop conditions,
 * and hotkey settings. Gracefully handles missing fields with sensible defaults.
 * @param cfg Output parameter; populated with loaded configuration
 * @param filePath Optional custom file path; uses defaultConfigPath() if empty
 * @return True if load succeeded, false if file doesn't exist or JSON is malformed
 */
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

/**
 * @brief Gets the default config file path for simple/normal automation mode
 * @details Normal mode is simpler than multi-point mode (single location, timed clicks)
 * @return Absolute path to "autoclicker_normal_config.json" in the config directory
 */
QString ConfigManager::defaultNormalConfigPath()
{
    return AppSettings::instance().configFilePath("autoclicker_normal_config.json");
}

/**
 * @brief Saves a NormalAppConfig (simple mode configuration) to JSON
 * @details Serializes timing, button type, click type, position, and hotkey
 * to disk in indented JSON format.
 * @param cfg The NormalAppConfig to save
 * @param filePath Optional custom file path; uses defaultNormalConfigPath() if empty
 * @return True if write succeeded, false on file operation failure
 */
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

/**
 * @brief Loads a NormalAppConfig (simple mode configuration) from a JSON file
 * @details Reconstructs all timing parameters, button/click types, position info,
 * and hotkey. Provides sensible defaults for any missing fields.
 * @param cfg Output parameter; populated with loaded normal mode config
 * @param filePath Optional custom file path; uses defaultNormalConfigPath() if empty
 * @return True if load succeeded, false if file doesn't exist or JSON is invalid
 */
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
