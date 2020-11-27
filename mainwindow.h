#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "options.h"
#include <iostream>
#include <QDebug>
#include <QDropEvent>
#include <QLabel>
#include <QMainWindow>
#include <QMimeData>
#include <QProgressBar>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void loadFile(std::string filename);
  std::shared_ptr<Settings> getGlobalSettings() const {
    return global_settings;
  }
  void addTab(std::unique_ptr<QWidget> widget, std::string title,
              bool autoSwitch = true);

  void readSettings();
  void writeSettings();

protected:
  void dropEvent(QDropEvent *event) override;
  void dragEnterEvent(QDragEnterEvent *event) override;

private slots:
  void on_actionOpen_triggered();
  void on_fileViewTabWidget_tabCloseRequested(int index);
  void on_actionExit_triggered();
  void on_actionOptions_triggered();

  void on_actionAbout_triggered();

  signals:
  void settingsChanged(std::shared_ptr<Settings> global_settings);

private:
  Ui::MainWindow *ui;
  QProgressBar *progressBar;
  QLabel *label;
  std::shared_ptr<Settings> global_settings;
  std::unordered_map<unsigned int, std::unique_ptr<QWidget>> pages;
};
#endif // MAINWINDOW_H
