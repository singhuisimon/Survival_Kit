/**
 * @file TracyManager.h
 * @brief Declaration of the TracyManager class for launching and managing the Tracy Profiler.
 * @details
 * Provides functionality to start, monitor, and shut down the Tracy profiler executable.
 * Acts as a singleton manager within the engine to ensure only one profiler instance is active.
 *
 * @author Amanda Leow Boon Suan (100%)
 * @date 2/10/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __TRACY_MANAGER_H__
#define __TRACY_MANAGER_H__

#include <string>
#include "../Manager/LogManager.h"
#include "Manager.h"

#ifdef _WIN32
#include <windows.h>
#endif

#define TRACY gam300::TracyManager::getInstance()

namespace gam300 {

    /**
     * @class TracyManager
     * @brief Handles lifecycle management of the Tracy Profiler.
     * @details
     * The TracyManager is responsible for launching the Tracy GUI as an external
     * process and monitoring its state during the engine runtime. It uses Windows API
     * functions to create, track, and terminate the profiler process.
     */
    class TracyManager : public Manager {
    public:
        /**************************************************************************
        * @brief Retrieves the singleton instance of the TracyManager.
        * @return Reference to the TracyManager singleton.
        **************************************************************************/
        static TracyManager& getInstance();

        /**************************************************************************
        * @brief Sets the file path to the Tracy profiler executable.
        * @param exename
        * Absolute or relative path to the Tracy executable file.
        **************************************************************************/
        void setTracyPath(const std::string& exename);

        /**************************************************************************
        * @brief
        * Launches the Tracy profiler as an external process if enabled and not running.
        * @note
        * This function only executes if `TRACY_ENABLE` is defined in the build configuration.
        **************************************************************************/
        void launchTracy();

        /**************************************************************************
        * @brief Monitors the profiler process and resets state if the process closes.
        **************************************************************************/
        void update();

        /**************************************************************************
        * @brief Starts up the TracyManager.
        * @return Integer code (0 if successful).
        **************************************************************************/
        int startUp() override;

        /**************************************************************************
        * @brief
        * Shuts down the TracyManager and terminates the running profiler process.
        **************************************************************************/
        void shutDown() override;

        /**************************************************************************
        * @brief
        * Checks if the Tracy profiler process is currently active.
        * @return
        * True if the profiler is running; otherwise, false.
        **************************************************************************/
        bool isRunning() const { return m_running; }

    private:
        /**************************************************************************
        * @brief
        * Default constructor (private for singleton pattern).
        **************************************************************************/
        TracyManager();

        /**************************************************************************
        * @brief
        * Destructor that ensures profiler process is properly terminated.
        **************************************************************************/
        ~TracyManager();

        // Delete copy and assignment operators to enforce singleton behavior
        TracyManager(const TracyManager&) = delete;
        TracyManager& operator=(const TracyManager&) = delete;

        //---------------------- Internal Data Members ----------------------//

        std::string m_tracyPath;    ///< Path to the Tracy profiler executable.
        bool m_running = false;     ///< Indicates if Tracy profiler is currently running.
        HANDLE m_processHandle;     ///< Handle to the Tracy profiler process (Windows-specific).
    };

} // namespace gam300

#endif // __TRACY_MANAGER_H__
