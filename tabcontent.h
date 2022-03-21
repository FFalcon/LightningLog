#ifndef TABCONTENT_H
#define TABCONTENT_H

#include "customdisplaydelegate.h"
#include "mainwindow.h"
#include <thread>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QWidget>

namespace Ui {
class MainWindow;
class TabContent;
}

class TabContent : public QWidget
{
    Q_OBJECT

public:
    explicit TabContent(QWidget *parent, std::shared_ptr<Settings> _tabSettings);
    explicit TabContent(QWidget *parent, std::shared_ptr<Settings> _tabSettings, std::unique_ptr<QStandardItemModel> _model);
    ~TabContent();

    void setContent(QString content);
    void setScrollToBottom(bool val) { scrollToBottom = val; }
    QString getFilename() { return filename; }

protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void createConnections();

private slots:
    void on_lineEdit_returnPressed();
    void on_isolateSelection_clicked();
    void on_scrollToBottom_toggled(bool checked);
    void logEntryDoubleClicked(const QModelIndex &index);
    void createIsolateTab(const QList<QStandardItem *> *items);
    void createContextView(const QList<QStandardItem *> *items);
    void settingsChanged(std::shared_ptr<Settings> global_settings);
    void initViewer();

signals:
    void scrollToBottomToggled(bool state);
    void isolateTaskDone(const QList<QStandardItem *> *items);
    void contextTaskDone(const QList<QStandardItem *> *items);
    void loadProgressed(QString filename, int signal);

private:
    Ui::TabContent *ui;
    std::unique_ptr<QStandardItemModel> model;
    std::unique_ptr<QSortFilterProxyModel> proxyModel;
    QString filename;
    std::thread loader_thread;
    std::thread scroll_task;
    std::vector<std::thread> tasks;
    std::atomic_bool cancellation_token;
    bool scrollToBottom;
    bool view_initialized;
    MainWindow *parentWindow;
    std::shared_ptr<Settings> tabSettings;
};

#endif // TABCONTENT_H
