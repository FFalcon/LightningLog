#include "options.h"
#include "ui_options.h"

Options::Options(QWidget *parent)
    : QDialog(parent), ui(new Ui::Options),
      currentFilterOptions(FilterOptions{.frontColor = Qt::black,
                                         .backColor = Qt::white,
                                         .isBold = false,
                                         .isItalic = false,
                                         .isCaseInsensitive = true}) {
  ui->setupUi(this);
  normalModeFilterMap = new QMap<QString, FilterOptions>();
  loadSettings();
}

void Options::loadSettings()
{
    auto path = QCoreApplication::applicationDirPath().append("\\").append(SETTINGS_FILENAME);
    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup("NormalMode");
    auto nitems = settings.value("nitems").value<int>();
    for (auto i = 0; i < nitems; i++) {
        QString raw_entry = settings.value(QString("item") + QString::number(i)).value<QString>();
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

        auto listItem = new QListWidgetItem();
        listItem->setData(Qt::DisplayRole, filter_kv.at(1));
        listItem->setData(Qt::FontRole, QFont("Cascadia Code", -1, filterOptions.isBold ? QFont::Bold : QFont::Normal, filterOptions.isItalic));
        listItem->setForeground(QBrush(filterOptions.frontColor));
        listItem->setBackground(QBrush(filterOptions.backColor));
        ui->listWidget->addItem(listItem);
    }
    settings.endGroup();
}

Options::~Options() { delete ui; }

void Options::on_cancelBtn_clicked() { close(); }

void Options::on_saveBtn_clicked()
{
    QSettings settings(QCoreApplication::applicationDirPath().append("\\").append(SETTINGS_FILENAME), QSettings::IniFormat);
    settings.beginGroup("NormalMode");
    settings.setValue("nitems", normalModeFilterMap->size());
    int idx = 0;
    for (auto it = normalModeFilterMap->begin(); it != normalModeFilterMap->end(); *it++, idx++) {
        auto value = "filter=" + it.key();
        value += "|";
        value += "front=" + it.value().frontColor.name();
        value += "|";
        value += "back=" + it.value().backColor.name();
        value += "|";
        value += QString("bold=") + (it.value().isBold ? "true" : "false");
        value += "|";
        value += QString("italic=") + (it.value().isItalic ? "true" : "false");
        value += "|";
        value += QString("caseSensitive=") + (it.value().isCaseInsensitive ? "true" : "false");
        settings.setValue(QString("item") + QString::number(idx), value);
    }
    settings.endGroup();
    accept();
}

void Options::on_frontColorBtn_clicked()
{
    currentFilterOptions.frontColor = QColorDialog::getColor(Qt::black, this, tr("Choose Foreground color"));
    ui->previewLabel->setStyleSheet("background-color:" + currentFilterOptions.backColor.name() + ";color:" + currentFilterOptions.frontColor.name());
}

void Options::on_backColorBtn_clicked()
{
    currentFilterOptions.backColor = QColorDialog::getColor(Qt::white, this, tr("Choose Background color"));
    ui->previewLabel->setStyleSheet("background-color:" + currentFilterOptions.backColor.name() + ";color:" + currentFilterOptions.frontColor.name());
}

void Options::on_ignoreCase_stateChanged(int state) {
  currentFilterOptions.isCaseInsensitive = state == Qt::Checked;
}

void Options::on_bold_stateChanged(int state) {
  currentFilterOptions.isBold = state == Qt::Checked;
}

void Options::on_italic_stateChanged(int state) {
  currentFilterOptions.isItalic = state == Qt::Checked;
}

void Options::on_addBtn_clicked() {
  auto text = ui->filterText->text();
  normalModeFilterMap->insert(text, currentFilterOptions);
  auto listItem = new QListWidgetItem();
  listItem->setData(Qt::DisplayRole, text);
  listItem->setData(
      Qt::FontRole,
      QFont("Cascadia Code", -1,
            currentFilterOptions.isBold ? QFont::Bold : QFont::Normal,
            currentFilterOptions.isItalic));
  listItem->setForeground(QBrush(currentFilterOptions.frontColor));
  listItem->setBackground(QBrush(currentFilterOptions.backColor));
  ui->listWidget->addItem(listItem);

  currentFilterOptions = FilterOptions();
  ui->filterText->setText("");
  ui->frontColorBtn->setStyleSheet("");
  ui->backColorBtn->setStyleSheet("");
  ui->bold->setChecked(false);
  ui->ignoreCase->setChecked(true);
  ui->italic->setChecked(false);
}

void Options::on_deleteBtn_clicked() {
  auto selectedWidgets = ui->listWidget->selectedItems();
  for (auto item : selectedWidgets) {
    auto key = item->data(Qt::DisplayRole).value<QString>();
    normalModeFilterMap->remove(key);
  }
  qDeleteAll(ui->listWidget->selectedItems());
}
