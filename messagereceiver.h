#ifndef MESSAGERECEIVER_H
#define MESSAGERECEIVER_H

#include "mainwindow.h"

#include <QObject>

class MessageReceiver : public QObject
{
    Q_OBJECT
public:
    explicit MessageReceiver(MainWindow *mainWindow, QObject *parent = nullptr);

public slots:
    void receivedMessage(int instanceId, QByteArray message);

private:
    MainWindow *_mainWindow;
};

#endif // MESSAGERECEIVER_H
