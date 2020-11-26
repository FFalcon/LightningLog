#ifndef TABCONTENT_H
#define TABCONTENT_H

#include "mainwindow.h"
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QWidget>
#include <thread>

namespace Ui {
class TabContent;
}

class TabContent : public QWidget {
  Q_OBJECT

public:
  explicit TabContent(QWidget *parent, std::shared_ptr<Settings> _tabSettings);
  explicit TabContent(QWidget *parent, std::shared_ptr<Settings> _tabSettings,
                      std::unique_ptr<QStandardItemModel> _model);
  ~TabContent();

  void setContent(std::string content);
  void setScrollToBottom(bool val) { scrollToBottom = val; }

protected:
  void dropEvent(QDropEvent *event) override;
  void dragEnterEvent(QDragEnterEvent *event) override;
  void createConnections();

private slots:
  void on_lineEdit_returnPressed();
  void on_isolateSelection_clicked();
  void on_scrollToBottom_toggled(bool checked);
  void on_listView_doubleClicked(const QModelIndex &index);
  void createIsolateTab(const QList<QStandardItem *> items);
  void createContextView(const QList<QStandardItem *> items);
  void settingsChanged(std::shared_ptr<Settings> global_settings);
  void initViewer();

signals:
  void scrollToBottomToggled(bool state);
  void isolateTaskDone(const QList<QStandardItem *> items);
  void contextTaskDone(const QList<QStandardItem *> items);

private:
  Ui::TabContent *ui;
  std::unique_ptr<QStandardItemModel> model;
  std::unique_ptr<QSortFilterProxyModel> proxyModel;
  std::thread loader_thread;
  std::thread scroll_task;
  std::vector<std::thread> tasks;
  std::atomic_bool cancellation_token;
  bool scrollToBottom;
  MainWindow *parentWindow;
  std::shared_ptr<Settings> tabSettings;
  // items
  std::vector<QStandardItem *> items;
};

#endif // TABCONTENT_H
