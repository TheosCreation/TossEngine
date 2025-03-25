#pragma once
#include "Utils.h"
#include <chrono>
#include <thread>


namespace fs = std::filesystem;

class TOSSENGINE_API FileWatcher {
public:
    FileWatcher(const std::string& watchPath);

    bool hasChanged();

private:
    fs::file_time_type lastWriteTime;
    std::string watchPath;
};