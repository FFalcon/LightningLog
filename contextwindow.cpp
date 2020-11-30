#include "contextwindow.h"
#include "ui_contextwindow.h"

ContextWindow::ContextWindow(QWidget *parent, std::unique_ptr<QStandardItemModel> _model) : QWidget(parent), ui(new Ui::ContextWindow), model(std::move(_model))
{
    ui->setupUi(this);
    ui->logEntries->setModel(model.get());
    ui->logEntries->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    ui->logEntries->setVerticalScrollMode(QListView::ScrollMode::ScrollPerItem);
    ui->logEntries->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

ContextWindow::~ContextWindow() { delete ui; }
