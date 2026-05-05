#pragma once
#include <QObject>
#include <QTimer>
#include "core/configmanager.h"

class NormalClickerEngine : public QObject
{
    Q_OBJECT
public:
    explicit NormalClickerEngine(QObject* parent = nullptr);
    ~NormalClickerEngine() override;

    void setConfig(const NormalAppConfig& config);
    bool isRunning() const { return m_running; }

    void start();
    void stop();

signals:
    void started();
    void stopped(const QString& reason);
    void clicksUpdated(int totalClicks);
    void statusMessage(const QString& msg);

private slots:
    void tick();

private:
    void doClick();

    QTimer* m_timer;
    NormalAppConfig m_config;
    bool m_running = false;
    int m_clickCount = 0;
};
