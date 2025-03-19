#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include <shlobj.h>
#include <algorithm>
#include <cctype>
#include "logger/log.h"

std::wstring get_file_extension(const std::wstring& filePath) {
    size_t pos = filePath.find_last_of(L".");
    if (pos != std::wstring::npos) {
        return filePath.substr(pos + 1);
    }
    return L"";
}

bool is_whitelisted_extension(const std::wstring& fileExtension) {
    const std::wstring allowedExtensions[] = { L"exe", L"dll", L"bin", L"sys" };
    std::wstring lowerExtension = fileExtension;
    std::transform(lowerExtension.begin(), lowerExtension.end(), lowerExtension.begin(), ::towlower);
    for (const auto& ext : allowedExtensions) {
        if (lowerExtension == ext) {
            return true;
        }
    }
    return false;
}

void copyfiletocdumps(const std::wstring& filePath) {
    WCHAR desktopPath[MAX_PATH];
    if (!SHGetSpecialFolderPathW(NULL, desktopPath, CSIDL_DESKTOPDIRECTORY, FALSE)) {
        Logger::log(Logger::Level::PROBLEM, L"failed to get desktop path.");
        return;
    }

    std::wstring cdumpsPath = std::wstring(desktopPath) + L"\\cdumps";
    if (!CreateDirectoryW(cdumpsPath.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        Logger::log(Logger::Level::PROBLEM, L"failed to create cdumps folder.");
        return;
    }

    std::wstring destPath = cdumpsPath + L"\\" + filePath.substr(filePath.find_last_of(L"\\") + 1);

    if (CopyFileW(filePath.c_str(), destPath.c_str(), FALSE)) {
        Logger::log(Logger::Level::SUCCESS, L"file copied to cdumps: " + destPath);
    }
    else {
        Logger::log(Logger::Level::PROBLEM, L"failed to copy file: " + filePath);
    }
}

void monitor_directory(const std::wstring& dirPath) {
    HANDLE hDirectory = CreateFileW(
        dirPath.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL);
    if (hDirectory == INVALID_HANDLE_VALUE) {
        Logger::log(Logger::Level::PROBLEM, L"failed to open folder for monitoring");
        return;
    }
    char buffer[1024];
    DWORD bytesReturned;
    while (true) {
        if (ReadDirectoryChangesW(hDirectory, buffer, sizeof(buffer), TRUE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
            FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
            FILE_NOTIFY_CHANGE_LAST_WRITE, &bytesReturned, NULL, NULL)) {
            FILE_NOTIFY_INFORMATION* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
            do {
                std::wstring fileName(info->FileName, info->FileNameLength / sizeof(WCHAR));
                if (info->Action == FILE_ACTION_ADDED) {
                    std::wstring fileExtension = get_file_extension(fileName);
                    if (is_whitelisted_extension(fileExtension)) {
                        copyfiletocdumps(dirPath + L"\\" + fileName);
                    }
                    else {
                        Logger::log(Logger::Level::WARNING, L"file not whitelisted skipping: " + fileName);
                    }
                }
                if (info->NextEntryOffset == 0) {
                    break;
                }
                info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<BYTE*>(info) + info->NextEntryOffset);
            } while (true);
        }
    }
    CloseHandle(hDirectory);
}

int main() {
    Logger::log(Logger::Level::INFO, L"beginning");
    monitor_directory(L"C:\\");
    return 0;
}