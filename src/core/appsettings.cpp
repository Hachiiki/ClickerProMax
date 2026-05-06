#include "appsettings.h"

/**
 * @brief Retrieves or creates the global AppSettings singleton instance
 * @details Uses a static local variable to ensure only one instance exists
 * and is thread-safe (since C++11)
 */
AppSettings& AppSettings::instance()
{
    static AppSettings s;
    return s;
}

/**
 * @brief Private constructor - initializes the QSettings object
 * @details Sets up Windows registry persistence with namespace "AutoClicker/AutoClicker"
 * Uses INI format for cross-platform compatibility
 */
AppSettings::AppSettings()
    : m_settings(QSettings::IniFormat, QSettings::UserScope,
                 "AutoClicker", "AutoClicker")
{
}

/**
 * @brief Gets the factory-default config directory
 * @details Returns the platform-standard application data location:
 *   - Windows: %APPDATA%/AutoClicker/AutoClicker
 *   - Linux: ~/.config/AutoClicker/AutoClicker (typically)
 *   - macOS: ~/Library/Preferences/AutoClicker/AutoClicker
 * @return Full path to the default config directory
 */
QString AppSettings::defaultConfigDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

/**
 * @brief Reads the config directory from persistent storage and ensures it exists
 * @details If the stored path is empty or invalid, falls back to defaultConfigDir().
 * Automatically creates the directory on the filesystem if needed via mkpath().
 * @return Absolute path to the configured directory
 */
QString AppSettings::configDir() const
{
    QString dir = m_settings.value(kKeyConfigDir, defaultConfigDir()).toString();
    QDir().mkpath(dir);
    return dir;
}

/**
 * @brief Updates the stored config directory and writes changes to registry
 * @details Persists the new directory path immediately via sync(). The directory
 * itself is not created here - that happens on-demand in configDir() and configFilePath().
 * @param dir Absolute path to the new config directory
 */
void AppSettings::setConfigDir(const QString& dir)
{
    m_settings.setValue(kKeyConfigDir, dir);
    m_settings.sync();
}

/**
 * @brief Constructs a full file path by joining config directory and filename
 * @details Creates the directory if it doesn't exist. Handles path separator
 * automatically. Used for loading/saving configuration files like JSON settings.
 * @param filename Name of the config file (e.g., "autoclicker_config.json")
 * @return Complete absolute path to the file
 */
QString AppSettings::configFilePath(const QString& filename) const
{
    const QString dir = configDir();      // already mkpath'd
    return dir + "/" + filename;
}
