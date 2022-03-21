#include "mainwindow.h"

#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icons/icon.png"));

    if (QFontDatabase::addApplicationFont(":/fonts/CascadiaCode.ttf") == -1)
        std::cerr << "Couldn't load font";

    QCoreApplication::setApplicationName("Lightning Log");
    QCoreApplication::setApplicationVersion("0.1.0-Alpha");

    MainWindow w;

    for (auto i = 1; i < QCoreApplication::arguments().length(); i++) {
        auto filename = QCoreApplication::arguments().at(i);
        w.loadFile(filename);
    }

    w.show();
    return a.exec();
}
