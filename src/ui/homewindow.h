#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

class HomeWindow : public QWidget
{
    Q_OBJECT
public:
    explicit HomeWindow(QWidget* parent = nullptr);

signals:
    void openAutoClicker();
    void openAutoClickerPro();
    void openNormalClicker();

private slots:
    void onBrowseConfigDir();
    void onResetConfigDir();

private:
    void buildUI();

    QLineEdit* m_configDirEdit = nullptr;
};
