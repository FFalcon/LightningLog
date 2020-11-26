#include "contextwindow.h"
#include "filereadertask.h"
#include "mainwindow.h"
#include "tabcontent.h"
#include "ui_tabcontent.h"
#include <QThreadPool>
#include <fstream>
#include <iostream>

void TabContent::createConnections() {
  connect(parentWindow, &MainWindow::settingsChanged, this,
          &TabContent::settingsChanged);

  connect(ui->logEntries->verticalScrollBar(), &QScrollBar::actionTriggered,
          [=]() {
            if (scrollToBottom)
              this->on_scrollToBottom_toggled(false);
          });

  connect(this, &TabContent::scrollToBottomToggled, [=](bool state) {
    if (state != ui->scrollToBottom->isChecked()) {
      ui->scrollToBottom->toggle();
    }
  });

  connect(this, &TabContent::isolateTaskDone, this,
          &TabContent::createIsolateTab);
  connect(ui->logEntries, &QTableView::doubleClicked, this,
          &TabContent::on_listView_doubleClicked);
  connect(this, &TabContent::contextTaskDone, this,
          &TabContent::createContextView);
}

TabContent::TabContent(QWidget *parent, std::shared_ptr<Settings> _tabSettings)
    : QWidget(parent), ui(new Ui::TabContent), cancellation_token(false),
      scrollToBottom(false), parentWindow((MainWindow *)parent),
      tabSettings(_tabSettings) {
  ui->setupUi(this);
  model = std::make_unique<QStandardItemModel>(this);
  proxyModel = std::make_unique<QSortFilterProxyModel>(this);
  createConnections();
}

TabContent::TabContent(QWidget *parent, std::shared_ptr<Settings> _tabSettings,
                       std::unique_ptr<QStandardItemModel> _model)
    : QWidget(parent), ui(new Ui::TabContent), model(std::move(_model)),
      cancellation_token(false), scrollToBottom(false),
      parentWindow((MainWindow *)parent), tabSettings(_tabSettings) {
  ui->setupUi(this);
  proxyModel = std::make_unique<QSortFilterProxyModel>(this);
  proxyModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  proxyModel->setSourceModel(model.get());
  ui->logEntries->setModel(proxyModel.get());
  ui->logEntries->setSelectionMode(
      QAbstractItemView::SelectionMode::ExtendedSelection);
  ui->logEntries->setVerticalScrollMode(
      QAbstractItemView::ScrollMode::ScrollPerItem);
  ui->logEntries->setEditTriggers(QAbstractItemView::NoEditTriggers);
  createConnections();
}

void TabContent::dropEvent(QDropEvent *event) {
  const QMimeData *mimeData = event->mimeData();

  // check for our needed mime type, here a file or a list of files
  if (mimeData->hasUrls()) {
    QStringList pathList;
    QList<QUrl> urlList = mimeData->urls();

    for (int i = 0; i < urlList.size() && i < 32; ++i) {
      pathList.append(urlList.at(i).toLocalFile());
    }

    // call a function to open the files
    for (const auto &path : pathList) {
      parentWindow->loadFile(path.toStdString());
    }
  }
}

void TabContent::dragEnterEvent(QDragEnterEvent *event) {
  event->acceptProposedAction();
}

void TabContent::setContent(std::string content) {
  auto file_reader_task = std::make_unique<FileReaderTask>(
      content, tabSettings, std::ref(cancellation_token));
  connect(file_reader_task.get(), &FileReaderTask::initialLoadFinished, this,
          &TabContent::initViewer);
  connect(file_reader_task.get(), &FileReaderTask::newItemCreated,
          [&](QStandardItem *item) {
            items.push_back(item);
            model->appendRow(item);
          });
  connect(file_reader_task.get(), &FileReaderTask::fileWillReload, this,
          [&]() { model->clear(); });
  loader_thread =
      std::thread(&FileReaderTask::run, std::move(file_reader_task));

  auto scroll_func = [](QAbstractItemView *listView, const bool &scrollToBottom,
                        std::atomic_bool &cancellation_token) {
    while (!cancellation_token) {
      if (scrollToBottom)
        listView->scrollToBottom();
      std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
  };
  scroll_task =
      std::thread(scroll_func, ui->logEntries, std::ref(scrollToBottom),
                  std::ref(cancellation_token));
}

TabContent::~TabContent() {
  cancellation_token = true;
  if (loader_thread.joinable())
    loader_thread.join();
  if (scroll_task.joinable())
    scroll_task.join();
  for (auto &th : tasks) {
    if (th.joinable())
      th.join();
  }
  tasks.clear();
  delete ui;
  for (auto &item : items) {
    delete item;
    item = nullptr;
  }
  items.erase(std::remove(items.begin(), items.end(), nullptr), items.end());
  parentWindow = nullptr;
  proxyModel->deleteLater();
  model->deleteLater();
}

void TabContent::on_lineEdit_returnPressed() {
  auto text = ui->lineEdit->text();
  std::cout << "Filter: " << text.toStdString() << std::endl;
  if (text.isEmpty()) {
    proxyModel->setFilterFixedString(text);
    return;
  }
  proxyModel->setFilterFixedString(text);
}

void TabContent::on_isolateSelection_clicked() {
  const auto isolate_func = [&]() {
    auto items = QList<QStandardItem *>();
    for (auto i = 0; i < proxyModel->rowCount(); i++) {
      auto idx = proxyModel->index(i, 0);
      auto item = new QStandardItem();
      item->setData(proxyModel->data(idx).toString(), Qt::DisplayRole);
      item->setData(proxyModel->data(idx, Qt::FontRole).value<QFont>(),
                    Qt::FontRole);
      item->setForeground(
          proxyModel->data(idx, Qt::ForegroundRole).value<QBrush>());
      item->setBackground(
          proxyModel->data(idx, Qt::BackgroundRole).value<QBrush>());
      items.append(item);
    }

    emit isolateTaskDone(std::move(items));
  };
  // start new one
  tasks.push_back(std::thread(isolate_func));
}

void TabContent::createIsolateTab(QList<QStandardItem *> items) {
  auto model = std::make_unique<QStandardItemModel>(this);
  model->appendRow(items);
  parentWindow->addTab(
      std::make_unique<TabContent>(parentWindow, tabSettings, std::move(model)),
      "Extract");
}

void TabContent::on_scrollToBottom_toggled(bool checked) {
  if (scrollToBottom != checked) {
    scrollToBottom = checked;
    emit scrollToBottomToggled(scrollToBottom);
  }
}

void TabContent::on_listView_doubleClicked(const QModelIndex &index) {
  const auto context_isolate_func = [&](const int numberOfRows) {
    auto items = QList<QStandardItem *>();
    auto sourceIndex = proxyModel->mapToSource(index);
    auto startIndex = sourceIndex.row() - numberOfRows / 2;
    if (startIndex < 0)
      startIndex = 0;
    for (auto i = startIndex; i <= startIndex + numberOfRows; ++i) {
      auto idx = model->index(i, 0);
      auto data = model->data(idx).toString();
      items.append(new QStandardItem(data));
    }
    emit contextTaskDone(std::move(items));
  };
  tasks.push_back(std::thread(context_isolate_func, 10));
}

void TabContent::createContextView(const QList<QStandardItem *> items) {
  auto context = std::make_unique<QStandardItemModel>(this);
  context->appendRow(items);
  auto context_view =
      std::make_unique<ContextWindow>(parentWindow, std::move(context));
  context_view->show();
}

void TabContent::settingsChanged(std::shared_ptr<Settings> settings) {
  tabSettings = settings;
}

void TabContent::initViewer() {
  proxyModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  proxyModel->setSourceModel(model.get());
  ui->logEntries->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->logEntries->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  ui->logEntries->verticalHeader()->hide();
  ui->logEntries->horizontalHeader()->hide();
  ui->logEntries->setShowGrid(false);
  ui->logEntries->setModel(proxyModel.get());
  ui->logEntries->setSelectionMode(
      QAbstractItemView::SelectionMode::ExtendedSelection);
  ui->logEntries->setVerticalScrollMode(
      QAbstractItemView::ScrollMode::ScrollPerItem);
  ui->logEntries->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
