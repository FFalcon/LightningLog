#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  QCoreApplication::setApplicationName("Log Lightning");
  QCoreApplication::setApplicationVersion("0.1.0-Alpha");

  MainWindow w;

  if (QCoreApplication::arguments().length() > 1) {
    auto filename = QCoreApplication::arguments().at(1);
    w.loadFile(filename.toStdString());
  }

  w.show();
  return a.exec();
}
