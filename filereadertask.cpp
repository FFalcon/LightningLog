#include "filereadertask.h"

FileReaderTask::FileReaderTask(std::string _filename,
                               std::shared_ptr<Settings> _settings,
                               std::atomic_bool &_cancellation_token)
    : QObject(), filename(_filename), cancellation_token(_cancellation_token),
      settings(_settings) {}

bool FileReaderTask::fileChanged(std::streampos posg)
{
    auto now = std::chrono::system_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_file_refresh).count();
    if (delta < 1000)
        return false;
    last_file_refresh = now;

    std::ifstream fs(filename, std::ios::binary);
    if (fs.is_open()) {
        fs.seekg(0, std::ios::end);
        auto eof = fs.tellg();
        if (posg > eof)
            return true;
        return false;
    }
    return true;
}

auto getFileSize(std::string filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    return file.tellg();
}

void FileReaderTask::run() {
    auto progress = 0;
    auto file_size = getFileSize(filename);
    std::ifstream ifs(filename, std::ios::in);
    if (ifs.is_open()) {
        std::string line;
        auto gpos = ifs.tellg();
        while (!cancellation_token) {
            if (fileChanged(gpos)) {
                emit fileWillReload();
                std::cout << "File changed, will reload" << std::endl;
                ifs.close();
                ifs.open(filename, std::ios::in);
                if (ifs.is_open()) {
                    initial_load = true;
                    progress = 0;
                    file_size = getFileSize(filename);
                    ifs.seekg(0, std::ios::beg);
                    gpos = ifs.tellg();
                } else {
                    std::cout << "Couldn't open file" << std::endl;
                }
            }

            if (!std::getline(ifs, line)) {
                if (initial_load) {
                    initial_load = false;
                    emit initialLoadFinished();
                }
                ifs.clear();
                ifs.seekg(gpos);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            gpos = ifs.tellg();

            if (initial_load) {
                progress = gpos * 100 / file_size;
                if (progress % 2 == 0)
                    emit loadProgressed(progress);
            }

            if (settings != nullptr && settings->getNormalModeFilters() != nullptr) {
                FilterOptions *usedFilter = nullptr;
                auto filtersMap = settings->getNormalModeFilters();
                for (auto key : filtersMap->keys()) {
                    auto stdStr = key.toStdString();
                    auto found = std::search(line.begin(), line.end(), stdStr.begin(), stdStr.end()) != line.end();
                    if (found)
                        usedFilter = &(*filtersMap)[key];
                }

                if (usedFilter != nullptr) {
                    auto item = new QStandardItem();
                    item->setData(QString::fromStdString(line), Qt::DisplayRole);
                    item->setData(QFont("Cascadia Code", -1, usedFilter->isBold ? QFont::Bold : QFont::Normal, usedFilter->isItalic), Qt::FontRole);
                    item->setForeground(QBrush(usedFilter->frontColor));
                    item->setBackground(QBrush(usedFilter->backColor));
                    emit newItemCreated(item);
                    continue;
                }
            }
            emit newItemCreated(new QStandardItem(QString::fromStdString(line)));
        }

        emit loadProgressed(200); // avoid hanging progressBar on interrupt while loading
    }
}
