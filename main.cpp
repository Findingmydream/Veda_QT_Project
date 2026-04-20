#include <QApplication>
#include <QPalette>

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("ShootingGameGoo");
    app.setOrganizationName("QtGame");
    app.setStyle("Fusion");

    QPalette dark;
    dark.setColor(QPalette::Window,          QColor(26, 26, 46));
    dark.setColor(QPalette::WindowText,      Qt::white);
    dark.setColor(QPalette::Base,            QColor(15, 15, 30));
    dark.setColor(QPalette::AlternateBase,   QColor(30, 30, 55));
    dark.setColor(QPalette::Text,            Qt::white);
    dark.setColor(QPalette::Button,          QColor(15, 52, 96));
    dark.setColor(QPalette::ButtonText,      Qt::white);
    dark.setColor(QPalette::Highlight,       QColor(233, 69, 96));
    dark.setColor(QPalette::HighlightedText, Qt::white);
    app.setPalette(dark);

    MainWindow w;
    w.show();
    return app.exec();
}
