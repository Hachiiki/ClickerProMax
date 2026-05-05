#pragma once
#include <QString>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

// ── Singleton that stores the user-chosen config directory ──────────────────
// The directory preference lives in QSettings (Windows registry) so it can
// be read *before* any JSON config file is loaded.
class AppSettings
{
public:
    static AppSettings& instance();

    // Returns the current config directory (creates it if needed).
    QString configDir() const;

    // Persist a new config directory.
    void setConfigDir(const QString& dir);

    // Convenience: configDir() + "/" + filename  (creates dir if needed).
    QString configFilePath(const QString& filename) const;

    // The factory-default directory (%APPDATA%/AutoClicker/AutoClicker).
    static QString defaultConfigDir();

private:
    AppSettings();
    QSettings m_settings;

    static constexpr const char* kKeyConfigDir = "configDir";
};
