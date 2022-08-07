#ifndef FILEREADERTASK_H
#define FILEREADERTASK_H

#include "Settings.h"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <windows.h>
#include <QObject>
#include <QRunnable>
#include <QStandardItemModel>

#define DEFAULT_LINE_SIZE 1024

class FileReaderTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    FileReaderTask(std::string _filename, std::shared_ptr<Settings> settings, std::atomic_bool &cancellation_token);
    void run();
    bool fileChanged(std::streampos posg);

signals:
    void initialLoadFinished();
    void newItemCreated(QStandardItem *item);
    void fileWillReload();
    void loadProgressed(QString filename, int progress);

private:
    bool initial_load = true;
    std::string filename;
    std::atomic_bool &cancellation_token;
    std::shared_ptr<Settings> settings;
    std::chrono::time_point<std::chrono::system_clock> last_file_refresh;
};

class WindowsFileReader
{
public:
    WindowsFileReader(std::string _filename) : filename(_filename) { open(); }
    ~WindowsFileReader() { close(); }
    bool getLine(std::string &line)
    {
        if (currentPosition < lines.size()) {
            line = lines.at(currentPosition++);
            return true;
        }
        return false;
    }
    bool didFileChange()
    {
        auto now = std::chrono::system_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_file_refresh).count();
        if (delta < 250)
            return false;
        last_file_refresh = now;

        HANDLE hTmpFile
            = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0 /* dwFlagsAndAttributes */, nullptr);
        if (hTmpFile == INVALID_HANDLE_VALUE) {
            return true;
        }

        LARGE_INTEGER tmpFileSizeLI;
        BOOL bResult = GetFileSizeEx(hTmpFile, &tmpFileSizeLI);
        if (!bResult) {
            return true;
        }
        ULONGLONG tmpFileSize = tmpFileSizeLI.QuadPart;
        if (fileSize > tmpFileSize)
            return true;
        if (fileSize < tmpFileSize) {
            auto numberOfBytes = tmpFileSize - fileSize;
            char *data = new char[numberOfBytes];
            unsigned long numberBytesRead;
            BOOL bResult = ReadFile(hFile, data, numberOfBytes, &numberBytesRead, nullptr);
            if (!bResult) {
                return true;
            }
            fillLines(data, numberBytesRead);
            fileSize += numberBytesRead;
        }
        return false;
    }
    void seek(std::string::size_type cursor)
    {
        assert(cursor >= 0);
        if (cursor > lines.size())
            currentPosition = lines.size();
        else
            currentPosition = cursor;
    }
    std::string::size_type tell() const { return currentPosition; }
    unsigned long long getFileSize() { return fileSize; }
    void reload()
    {
        close();
        open();
    }
    bool opened() const { return isOpen; }

private:
    void open()
    {
        hFile = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0 /* dwFlagsAndAttributes */, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            fileSize = 0;
            offset = 0;
            currentPosition = 0;
            isOpen = false;
            lines.clear();
            return;
        }

        LARGE_INTEGER file_size;
        BOOL bResult = GetFileSizeEx(hFile, &file_size);
        if (!bResult) {
            fileSize = 0;
            offset = 0;
            currentPosition = 0;
            isOpen = false;
            lines.clear();
            return;
        }

        fileSize = file_size.QuadPart;

        std::vector<char> vData(fileSize + 1);
        unsigned long numberBytesRead;
        bResult = ReadFile(hFile, vData.data(), fileSize, &numberBytesRead, nullptr);
        if (!bResult) {
            fileSize = 0;
            offset = 0;
            currentPosition = 0;
            isOpen = false;
            lines.clear();
            return;
        }

        auto data = vData.data();
        lines.clear();
        fillLines(data, fileSize);
        currentPosition = 0;
        offset = 0;
        isOpen = true;
    }

    void close() { CloseHandle(hFile); }

    void fillLines(char *data, unsigned long long size)
    {
        std::vector<char> stringBuffer;
        for (auto i = 0ULL; i < size; i++) {
            if (data[i] == '\n') {
                if (stringBuffer.back() == '\r') {
                    stringBuffer.back() = '\0';
                } else {
                    stringBuffer.push_back('\0');
                }
                lines.emplace_back(stringBuffer.data());
                stringBuffer.clear();
                continue;
            }
            stringBuffer.push_back(data[i]);
        }
        if (stringBuffer.size() > 0)
            lines.emplace_back(stringBuffer.data());
    }

    HANDLE hFile;
    bool isOpen = false;
    unsigned long long fileSize;
    unsigned long long offset;
    std::string filename;
    std::vector<std::string> lines;
    std::wstring::size_type currentPosition = 0;
    std::chrono::time_point<std::chrono::system_clock> last_file_refresh;
};

#endif // FILEREADERTASK_H
