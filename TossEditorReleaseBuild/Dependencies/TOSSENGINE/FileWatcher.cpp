#include "FileWatcher.h"
#include <iostream>

FileWatcher::FileWatcher(const std::string& watchPath) : watchPath(watchPath) {
    try {
        if (fs::exists(watchPath))
            lastWriteTime = fs::last_write_time(watchPath);
        else
            lastWriteTime = fs::file_time_type::min(); // default to minimal time
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "[FileWatcher] Error initializing: " << e.what() << "\n";
        lastWriteTime = fs::file_time_type::min();
    }
}

bool FileWatcher::hasChanged()
{
    try {
        if (!fs::exists(watchPath)) {
            return false; // File no longer exists
        }

        auto currentWriteTime = fs::last_write_time(watchPath);
        if (currentWriteTime != lastWriteTime) {
            lastWriteTime = currentWriteTime;
            return true;
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "[FileWatcher] Error during file check: " << e.what() << "\n";
        return false;
    }

    return false;
}