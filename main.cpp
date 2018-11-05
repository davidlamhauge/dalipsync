
#include <QApplication>
#include "mainwindow.h"
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("dalipsync");

    // a TRANSLATOR for the QT strings
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
            QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);
    // a TRANSLATOR for the text in the program
    QTranslator appTranslator;
    appTranslator.load("translations/dalipsync_" + QLocale::system().name());
    a.installTranslator(&appTranslator);
    MainWindow w;
    w.showMaximized();

    return a.exec();
}
