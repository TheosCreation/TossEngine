/***
Bachelor of Software Engineering
Media Design School
Auckland
New Zealand
(c) 2025 Media Design School
File Name : FileWatcher.h
Description : Watches a file or directory for changes based on modification timestamps.
Author : Theo Morris
Mail : theo.morris@mds.ac.nz
***/

#pragma once

#include "Utils.h"
#include <chrono>
#include <thread>

/**
 * @class FileWatcher
 * @brief Monitors a specified file or directory for changes by checking the last write time.
 */
class TOSSENGINE_API FileWatcher {
public:
    /**
     * @brief Constructs a FileWatcher for a specific path.
     * @param watchPath Path to the file or directory to monitor.
     */
    FileWatcher(const std::string& watchPath);

    /**
     * @brief Checks if the monitored file or directory has changed since the last check.
     * @return True if the file or directory was modified, false otherwise.
     */
    bool hasChanged();

private:
    /**
     * @brief Stores the last known write time of the file or directory.
     */
    fs::file_time_type lastWriteTime;

    /**
     * @brief Path to the file or directory being monitored.
     */
    std::string watchPath;
};