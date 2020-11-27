#ifndef FILEREADERTASK_H
#define FILEREADERTASK_H

#include "Settings.h"
#include <QObject>
#include <QRunnable>
#include <QStandardItemModel>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

class FileReaderTask : public QObject, public QRunnable {
  Q_OBJECT

public:
  FileReaderTask(std::string _filename, std::shared_ptr<Settings> settings,
                 std::atomic_bool &cancellation_token);
  void run();
  bool fileChanged(std::streampos posg);

signals:
  void initialLoadFinished();
  void newItemCreated(QStandardItem *item);
  void fileWillReload();
  void loadProgressed(int progress);

  private:
  bool initial_load = true;
  std::string filename;
  std::atomic_bool &cancellation_token;
  std::shared_ptr<Settings> settings;
  std::chrono::time_point<std::chrono::system_clock> last_file_refresh;
};
#endif // FILEREADERTASK_H
