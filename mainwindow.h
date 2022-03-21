#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "options.h"
#include <iostream>
#include <QCompleter>
#include <QDebug>
#include <QDropEvent>
#include <QLabel>
#include <QMainWindow>
#include <QMimeData>
#include <QProgressBar>
#include <QSettings>
#include <QStringListModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class TabContent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadFile(QString filename);
    std::shared_ptr<Settings> getGlobalSettings() const { return global_settings; }
    QCompleter *getCompleter() { return completer; }
    void addCompleterString(QString filter);
    void addTab(std::unique_ptr<TabContent> widget, QString title, bool autoSwitch = true);

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
    std::unordered_map<unsigned int, std::unique_ptr<TabContent>> pages;
    QCompleter *completer;
    QStringListModel *listModel;
};
#endif // MAINWINDOW_H
