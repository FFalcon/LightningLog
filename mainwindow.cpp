#include "QFileDialog"
#include "mainwindow.h"
#include "tabcontent.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      global_settings(std::make_shared<Settings>()) {
  ui->setupUi(this);
  ui->fileViewTabWidget->clear();
  ui->fileViewTabWidget->setTabsClosable(true);
  connect(ui->fileViewTabWidget, &QTabWidget::tabCloseRequested,
          [&](int index) {
            qDebug() << ui->fileViewTabWidget->count();
            auto deletedWidget = ui->fileViewTabWidget->widget(index);
            if (deletedWidget != nullptr) {
              delete deletedWidget;
              deletedWidget = nullptr;
            }
            if (pages.find(index) != pages.end()) {
              auto ptr = std::move(pages[index]);
              pages.erase(index);
            }
          });
  readSettings();
}

void MainWindow::loadFile(std::string filename) {
  qDebug() << "Loading file: " << QString::fromStdString(filename);
  auto tabContent = std::make_unique<TabContent>(this, global_settings);
  tabContent->setContent(filename);
  addTab(std::move(tabContent), filename);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::dropEvent(QDropEvent *event) {
  const QMimeData *mimeData = event->mimeData();

  // check for our needed mime type, here a file or a list of files
  if (mimeData->hasUrls()) {
    QStringList pathList;
    QList<QUrl> urlList = mimeData->urls();

    for (int i = 0; i < urlList.size() && i < 32; ++i) {
      pathList.append(urlList.at(i).toLocalFile());
    }

    // call a function to open the files
    for (auto path : pathList) {
      loadFile(path.toStdString());
    }
  }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  event->acceptProposedAction();
}

void MainWindow::readSettings() {
  QSettings settings(QCoreApplication::applicationDirPath() +
                         "loglightning.conf",
                     QSettings::IniFormat);

  settings.beginGroup("NormalMode");
  QMap<QString, FilterOptions> *normalModeFilterMap =
      new QMap<QString, FilterOptions>();
  auto nitems = settings.value("nitems").value<int>();
  for (auto i = 0; i < nitems; i++) {
    QString raw_entry =
        settings.value(QString("item") + QString(i)).value<QString>();
    auto entry_splits = raw_entry.split("|");
    if (entry_splits.size() != 6)
      continue;
    auto filter_kv = entry_splits.at(0).split("=");
    auto front_kv = entry_splits.at(1).split("=");
    auto back_kv = entry_splits.at(2).split("=");
    auto bold_kv = entry_splits.at(3).split("=");
    auto italic_kv = entry_splits.at(4).split("=");
    auto caseSensitive_kv = entry_splits.at(5).split("=");
    FilterOptions filterOptions{
        .frontColor = QColor(front_kv.at(1)),
        .backColor = QColor(back_kv.at(1)),
        .isBold = bold_kv.at(1) == "true",
        .isItalic = italic_kv.at(1) == "true",
        .isCaseInsensitive = caseSensitive_kv.at(1) == "false",
    };
    normalModeFilterMap->insert(filter_kv.at(1), filterOptions);
  }
  settings.endGroup();
  global_settings->setNormalModeFilters(normalModeFilterMap);
  emit settingsChanged(global_settings);
}

void MainWindow::addTab(std::unique_ptr<QWidget> widget, std::string title,
                        bool autoSwitch) {
  if (widget) {
    unsigned index = ui->fileViewTabWidget->addTab(
        widget.get(), QString::fromStdString(title));
    pages.insert(std::make_pair(index, std::move(widget)));
    if (autoSwitch)
      ui->fileViewTabWidget->setCurrentIndex(index);
  }
}

void MainWindow::writeSettings() {
  QSettings settings(QCoreApplication::applicationDirPath() +
                         "loglightning.conf",
                     QSettings::IniFormat);
  settings.beginGroup("NormalMode");

  settings.endGroup();
}

void MainWindow::on_actionOpen_triggered() {
  QString filename = QFileDialog::getOpenFileName(this, tr("Choose file"), "/",
                                                  tr("All files (*.*)"));
  if (!filename.isEmpty())
    loadFile(filename.toStdString());
}

void MainWindow::on_fileViewTabWidget_tabCloseRequested(int index) {
  ui->fileViewTabWidget->removeTab(index);
}

void MainWindow::on_actionExit_triggered() { qApp->exit(); }

void MainWindow::on_actionOptions_triggered() {
  auto optionsWindow = new Options(this);
  auto result = optionsWindow->exec();
  if (result == QDialog::Accepted)
    readSettings();
  else
    qDebug() << "Setting edition cancelled";
}
