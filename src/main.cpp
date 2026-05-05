#include <QApplication>
#include <QIcon>
#include "ui/mainwindow.h"
#include "ui/homewindow.h"
#include "ui/normalclickwindow.h"

int main(int argc, char* argv[])
{
    // High-DPI support (Qt5 style – harmless on Qt6)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    app.setApplicationName("AutoClicker");
    app.setOrganizationName("AutoClicker");
    app.setWindowIcon(QIcon("src/logo/ClickerProMax_256.png"));

    HomeWindow home;
    MainWindow mw;
    NormalClickWindow nw;

    QObject::connect(&home, &HomeWindow::openAutoClicker, [&]() {
        home.hide();
        mw.show();
    });

    QObject::connect(&home, &HomeWindow::openNormalClicker, [&]() {
        home.hide();
        nw.show();
    });

    QObject::connect(&mw, &MainWindow::backRequested, [&]() {
        mw.hide();
        home.show();
    });

    QObject::connect(&nw, &NormalClickWindow::backRequested, [&]() {
        nw.hide();
        home.show();
    });

    home.show();

    return app.exec();
}
