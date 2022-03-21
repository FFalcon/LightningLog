#include "tabcontent.h"
#include "contextwindow.h"
#include "filereadertask.h"
#include "mainwindow.h"
#include "ui_tabcontent.h"
#include <fstream>
#include <iostream>
#include <QThreadPool>

void TabContent::createConnections()
{
    connect(parentWindow, &MainWindow::settingsChanged, this, &TabContent::settingsChanged);

    connect(ui->logEntries->verticalScrollBar(), &QScrollBar::actionTriggered, [&]() {
        auto isAtBottom = ui->logEntries->verticalScrollBar()->sliderPosition() == ui->logEntries->verticalScrollBar()->maximum();
        if (!isAtBottom && scrollToBottom)
            on_scrollToBottom_toggled(false);
        if (isAtBottom && !scrollToBottom)
            on_scrollToBottom_toggled(true);
    });

    connect(this, &TabContent::scrollToBottomToggled, [&](bool state) {
        if (state != ui->scrollToBottom->isChecked()) {
            ui->scrollToBottom->setChecked(state);
        }
    });

    connect(this, &TabContent::isolateTaskDone, this, &TabContent::createIsolateTab);
    connect(ui->logEntries, &QTableView::doubleClicked, this, &TabContent::logEntryDoubleClicked);
    connect(this, &TabContent::contextTaskDone, this, &TabContent::createContextView);
}

TabContent::TabContent(QWidget *parent, std::shared_ptr<Settings> _tabSettings)
    : QWidget(parent), ui(new Ui::TabContent), cancellation_token(false), scrollToBottom(false), view_initialized(false), parentWindow((MainWindow *) parent),
      tabSettings(_tabSettings)
{
    ui->setupUi(this);
    model = std::make_unique<QStandardItemModel>(this);
    proxyModel = std::make_unique<QSortFilterProxyModel>(this);
    createConnections();
}

TabContent::TabContent(QWidget *parent, std::shared_ptr<Settings> _tabSettings, std::unique_ptr<QStandardItemModel> _model)
    : QWidget(parent), ui(new Ui::TabContent), model(std::move(_model)), cancellation_token(false), scrollToBottom(false), view_initialized(false),
      parentWindow((MainWindow *) parent), tabSettings(_tabSettings)
{
    ui->setupUi(this);
    proxyModel = std::make_unique<QSortFilterProxyModel>(this);
    initViewer();
    createConnections();
}

void TabContent::dropEvent(QDropEvent *event)
{
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
            parentWindow->loadFile(path);
        }
    }
}

void TabContent::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void TabContent::setContent(QString content)
{
    filename = content;
    auto file_reader_task = new FileReaderTask(content.toStdString(), tabSettings, std::ref(cancellation_token));

    connect(file_reader_task, &FileReaderTask::loadProgressed, this, &TabContent::loadProgressed);
    connect(file_reader_task, &FileReaderTask::initialLoadFinished, this, &TabContent::initViewer);
    connect(file_reader_task, &FileReaderTask::newItemCreated, [&](QStandardItem *item) { model->appendRow(item); });
    connect(file_reader_task, &FileReaderTask::fileWillReload, [&]() { model->clear(); });

    loader_thread = std::thread(&FileReaderTask::run, std::move(file_reader_task));
}

TabContent::~TabContent()
{
    cancellation_token = true;
    delete ui;
    if (loader_thread.joinable())
        loader_thread.join();
    if (scroll_task.joinable())
        scroll_task.join();
    for (auto &th : tasks) {
        if (th.joinable())
            th.join();
    }
    parentWindow = nullptr;
}

void TabContent::on_lineEdit_returnPressed()
{
    auto text = ui->lineEdit->text();
    std::cout << "Filter: " << text.toStdString() << std::endl;
    proxyModel->setFilterFixedString(text);
    parentWindow->addCompleterString(text);
}

void TabContent::on_isolateSelection_clicked()
{
    const auto isolate_func = [&]() {
        auto items = new QList<QStandardItem *>();
        for (auto i = 0; i < proxyModel->rowCount(); i++) {
            auto idx = proxyModel->index(i, 0);
            auto item = new QStandardItem();
            auto item_data = proxyModel->itemData(idx);

            for (auto it = item_data.begin(); it != item_data.end(); it++) {
                item->setData(it.value(), it.key());
            }

            //            auto text = proxyModel->data(idx).toString();
            //            auto font = proxyModel->data(idx, Qt::FontRole);
            //            auto foreground = proxyModel->data(idx, Qt::ForegroundRole);
            //            auto background = proxyModel->data(idx, Qt::BackgroundRole);

            //            item->setData(text, Qt::DisplayRole);
            //            if (font.canConvert<QFont>())
            //                item->setData(font.value<QFont>(), Qt::FontRole);
            //            if (foreground.canConvert<QBrush>())
            //                item->setForeground(foreground.value<QBrush>());
            //            if (background.canConvert<QBrush>())
            //                item->setBackground(background.value<QBrush>());
            items->append(item);
        }

        emit isolateTaskDone(items);
    };
    // start new one
    tasks.push_back(std::thread(isolate_func));
}

void TabContent::createIsolateTab(const QList<QStandardItem *> *items)
{
    auto model = std::make_unique<QStandardItemModel>();
    for (auto &item : *items)
        model->appendRow(item);
    parentWindow->addTab(std::make_unique<TabContent>(parentWindow, tabSettings, std::move(model)), filename.append(" Extract"));
}

void TabContent::on_scrollToBottom_toggled(bool checked)
{
    if (scrollToBottom != checked) {
        scrollToBottom = checked;
        emit scrollToBottomToggled(scrollToBottom);
    }
}

void TabContent::logEntryDoubleClicked(const QModelIndex &index)
{
    const auto context_isolate_func = [&](const int numberOfRows) {
        auto items = new QList<QStandardItem *>();
        auto sourceIndex = proxyModel->mapToSource(index);
        auto startIndex = sourceIndex.row() - numberOfRows / 2;
        if (startIndex < 0)
            startIndex = 0;
        for (auto i = startIndex; i <= startIndex + numberOfRows; ++i) {
            auto idx = model->index(i, 0);
            auto data = model->data(idx).toString();
            items->append(new QStandardItem(data));
        }
        emit contextTaskDone(items);
    };
    tasks.push_back(std::thread(context_isolate_func, 200));
}

void TabContent::createContextView(const QList<QStandardItem *> *items)
{
    auto context = std::make_unique<QStandardItemModel>();
    for (auto &item : *items)
        context->appendRow(item);
    auto context_view = new ContextWindow(nullptr, std::move(context));
    context_view->show();
}

void TabContent::settingsChanged(std::shared_ptr<Settings> settings)
{
    tabSettings = settings;
}

void TabContent::initViewer()
{
    if (view_initialized)
        return;
    proxyModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
    proxyModel->setSourceModel(model.get());
    ui->logEntries->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    ui->logEntries->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->logEntries->horizontalHeader()->setDefaultSectionSize(20000);
    ui->logEntries->verticalHeader()->hide();
    ui->logEntries->horizontalHeader()->hide();
    ui->logEntries->setShowGrid(false);
    ui->logEntries->setModel(proxyModel.get());
    ui->logEntries->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    ui->logEntries->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
    ui->logEntries->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //    ui->logEntries->setItemDelegate(new CustomDisplayDelegate);

    ui->lineEdit->setCompleter(parentWindow->getCompleter());

    connect(ui->logEntries->selectionModel(), &QItemSelectionModel::selectionChanged, [&](const QItemSelection &selected) {
        if (selected.isEmpty())
            return;
        auto last_range = selected.back();
        auto last_item = last_range.bottom();
        auto last_index = proxyModel->index(last_item, 0);
        auto item = proxyModel->data(last_index);
        ui->lineView->setPlainText(item.toString());
    });

    auto scroll_func = [](QAbstractItemView *listView, const bool &scrollToBottom, std::atomic_bool &cancellation_token) {
        while (!cancellation_token) {
            if (scrollToBottom)
                listView->scrollToBottom();
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    };
    scroll_task = std::thread(scroll_func, ui->logEntries, std::ref(scrollToBottom), std::ref(cancellation_token));
    view_initialized = true;
}
