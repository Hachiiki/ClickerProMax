#include "appsettings.h"

AppSettings& AppSettings::instance()
{
    static AppSettings s;
    return s;
}

AppSettings::AppSettings()
    : m_settings(QSettings::IniFormat, QSettings::UserScope,
                 "AutoClicker", "AutoClicker")
{
}

QString AppSettings::defaultConfigDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString AppSettings::configDir() const
{
    QString dir = m_settings.value(kKeyConfigDir, defaultConfigDir()).toString();
    QDir().mkpath(dir);
    return dir;
}

void AppSettings::setConfigDir(const QString& dir)
{
    m_settings.setValue(kKeyConfigDir, dir);
    m_settings.sync();
}

QString AppSettings::configFilePath(const QString& filename) const
{
    const QString dir = configDir();      // already mkpath'd
    return dir + "/" + filename;
}
