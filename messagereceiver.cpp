#include "messagereceiver.h"
#include <QtDebug>

MessageReceiver::MessageReceiver(MainWindow *mainWindow, QObject *parent) : QObject(parent), _mainWindow(mainWindow) {}

void MessageReceiver::receivedMessage(int instanceId, QByteArray message)
{
    qDebug() << "Received message from instance: " << instanceId;
    qDebug() << "Message Text: " << message;
    auto arguments = QString::fromUtf8(message).split(" ");
    for (auto i = 1; i < arguments.length(); i++) {
        auto filename = arguments.at(i);
        _mainWindow->loadFile(filename);
    }
}
