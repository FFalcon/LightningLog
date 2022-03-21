#include "mainwindow.h"
#include "messagereceiver.h"

#include <SingleApplication>
#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    SingleApplication a(argc, argv, true, SingleApplication::Mode::ExcludeAppPath | SingleApplication::Mode::ExcludeAppVersion);

    a.setWindowIcon(QIcon(":/icons/icon.png"));

    if (QFontDatabase::addApplicationFont(":/fonts/CascadiaCode.ttf") == -1)
        std::cerr << "Couldn't load font";

    QCoreApplication::setApplicationName("Lightning Log");
    QCoreApplication::setApplicationVersion("0.1.0-Alpha");

    MainWindow w;

    MessageReceiver msgReceiver(&w);

    // If this is a secondary instance
    if (a.isSecondary()) {
        a.sendMessage(a.arguments().join(' ').toUtf8());
        qDebug() << "App already running.";
        qDebug() << "Primary instance PID: " << a.primaryPid();
        qDebug() << "Primary instance user: " << a.primaryUser();
        return 0;
    } else {
        QObject::connect(&a, &SingleApplication::receivedMessage, &msgReceiver, &MessageReceiver::receivedMessage);
    }

    for (auto i = 1; i < QCoreApplication::arguments().length(); i++) {
        auto filename = QCoreApplication::arguments().at(i);
        w.loadFile(filename);
    }

    w.show();
    return a.exec();
}
