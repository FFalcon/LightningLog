#ifndef ISOLATEWINDOW_H
#define ISOLATEWINDOW_H

#include <QStandardItemModel>
#include <QWidget>

namespace Ui {
class ContextWindow;
}

class ContextWindow : public QWidget {
  Q_OBJECT

public:
  explicit ContextWindow(QWidget *parent,
                         std::unique_ptr<QStandardItemModel> model);
  ~ContextWindow();

private:
  Ui::ContextWindow *ui;
  std::unique_ptr<QStandardItemModel> model;
};

#endif // ISOLATEWINDOW_H
