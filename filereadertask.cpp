#include "filereadertask.h"

FileReaderTask::FileReaderTask(std::string _filename, std::shared_ptr<Settings> _settings, std::atomic_bool &_cancellation_token)
    : QObject(), filename(_filename), cancellation_token(_cancellation_token), settings(_settings)
{
    initial_load = true;
}

bool FileReaderTask::fileChanged(std::streampos posg)
{
    auto now = std::chrono::system_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_file_refresh).count();
    if (delta < 250)
        return false;
    last_file_refresh = now;

    std::ifstream fs(filename, std::ios::in);
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
    auto *fileReader = new WindowsFileReader(filename);

    std::string line;
    while (!cancellation_token) {
        if (!initial_load && fileReader->didFileChange()) {
            emit fileWillReload();
            std::cout << "File changed, will reload" << std::endl;
            fileReader->reload();
            initial_load = true;
            progress = 0;
        }

        if (!fileReader->getLine(line)) {
            if (initial_load) {
                initial_load = false;
                emit initialLoadFinished();
                continue;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        auto gpos = fileReader->tell();
        auto file_size = fileReader->getFileSize();

        if (initial_load && fileReader->opened()) {
            progress = gpos * 100 / file_size;
            if (progress % 20 == 0)
                emit loadProgressed(QString::fromStdString(filename), progress);
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

    delete fileReader;
}
