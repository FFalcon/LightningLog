#ifndef SETTINGS_H
#define SETTINGS_H

#define SETTINGS_FILENAME "lightninglog.conf"

#include <QColor>

typedef struct FilterOptions
{
    QColor frontColor;
    QColor backColor;
    bool isBold;
    bool isItalic;
    bool isCaseInsensitive;
} FilterOptions;

class Profile
{
private:
    QString profile_name;
    QVector<QString> *profile_trigger_filenames;
    QString profile_regex;
    QMap<QString, FilterOptions> *normal_mode_filters = nullptr;
};

class Settings {
public:
    void setNormalModeFilters(QMap<QString, FilterOptions> *filters) { normalModeFilters = filters; }
    QMap<QString, FilterOptions> *getNormalModeFilters() { return normalModeFilters; }

private:
    QMap<QString, FilterOptions> *normalModeFilters = nullptr;
};

#endif // SETTINGS_H
