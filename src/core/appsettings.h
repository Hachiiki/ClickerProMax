#pragma once
#include <QString>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

/**
 * @class AppSettings
 * @brief Application-wide settings manager (Singleton)
 *
 * Manages persistent application configuration stored in the Windows registry/INI files.
 * Provides a centralized interface for retrieving and setting the application's config
 * directory, which determines where user settings and click point configurations are saved.
 *
 * The config directory defaults to %APPDATA%/AutoClicker/AutoClicker but can be
 * customized by the user. All changes are automatically persisted via QSettings.
 *
 * @usage
 *     QString configDir = AppSettings::instance().configDir();
 *     QString settingsFile = AppSettings::instance().configFilePath("settings.json");
 */
class AppSettings
{
public:
    /**
     * @brief Retrieves the singleton instance of AppSettings
     * @return Reference to the global AppSettings instance
     */
    static AppSettings& instance();

    /**
     * @brief Gets the current config directory path
     * @details If the directory doesn't exist on disk, it is created automatically.
     * @return Absolute path to the config directory
     */
    QString configDir() const;

    /**
     * @brief Updates the config directory and persists the change
     * @param dir Absolute path to the new config directory
     * @note The directory is not automatically created here; it's created when needed
     */
    void setConfigDir(const QString& dir);

    /**
     * @brief Constructs a full config file path
     * @param filename Name of the config file (e.g., "settings.json")
     * @return Absolute path combining configDir() and filename
     * @note Creates the directory if it doesn't exist
     */
    QString configFilePath(const QString& filename) const;

    /**
     * @brief Gets the default factory config directory
     * @return Path to %APPDATA%/AutoClicker/AutoClicker
     */
    static QString defaultConfigDir();

private:
    /**
     * @brief Private constructor (Singleton)
     * @details Initializes QSettings with AutoClicker/AutoClicker namespace
     */
    AppSettings();

    QSettings m_settings;  ///< Qt settings object for registry/INI persistence
    static constexpr const char* kKeyConfigDir = "configDir";  ///< Registry key for config directory
};
