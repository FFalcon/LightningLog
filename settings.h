#ifndef SETTINGS_H
#define SETTINGS_H

#include <QColor>

typedef struct FilterOptions {
  QColor frontColor;
  QColor backColor;
  bool isBold;
  bool isItalic;
  bool isCaseInsensitive;
} FilterOptions;

class Settings {
public:
  void setNormalModeFilters(QMap<QString, FilterOptions> *filters) {
    normalModeFilters = filters;
  }
  QMap<QString, FilterOptions> *getNormalModeFilters() {
    return normalModeFilters;
  }

private:
  QMap<QString, FilterOptions> *normalModeFilters = nullptr;
};

#endif // SETTINGS_H
