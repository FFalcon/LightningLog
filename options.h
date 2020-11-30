#ifndef OPTIONS_H
#define OPTIONS_H

#include "Settings.h"
#include <QColorDialog>
#include <QDialog>
#include <QSettings>

namespace Ui {
class Options;
}

class Options : public QDialog
{
    Q_OBJECT

public:
    explicit Options(QWidget *parent = nullptr);
    void loadSettings();
    ~Options();

private slots:
    void on_cancelBtn_clicked();
    void on_saveBtn_clicked();

    void on_frontColorBtn_clicked();

    void on_backColorBtn_clicked();

    void on_ignoreCase_stateChanged(int arg1);

    void on_bold_stateChanged(int arg1);

    void on_italic_stateChanged(int arg1);

    void on_addBtn_clicked();

    void on_deleteBtn_clicked();

private:
    Ui::Options *ui;
    FilterOptions currentFilterOptions;
    QMap<QString, FilterOptions> *normalModeFilterMap;
};

#endif // OPTIONS_H
