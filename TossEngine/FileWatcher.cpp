#include "FileWatcher.h"

FileWatcher::FileWatcher(const std::string& watchPath) : watchPath(watchPath) {
    lastWriteTime = fs::last_write_time(watchPath);
}

bool FileWatcher::hasChanged()
{
    auto currentWriteTime = fs::last_write_time(watchPath);
    if (currentWriteTime != lastWriteTime) {
        lastWriteTime = currentWriteTime;
        return true;
    }
    return false;
}