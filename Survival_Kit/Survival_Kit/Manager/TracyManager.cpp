/**
 * @file TracyManager.cpp
 * @brief Implementation of the TracyManager class.
 * @details 
 * Handles launching and managing the external Tracy profiler process.
 * Utilizes Windows API functions to spawn and track the profiler executable.
 * 
 * @author Amanda Leow Boon Suan (100%)
 * @date 2/10/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "TracyManager.h"
#include <cstdlib>
#include <filesystem>

namespace gam300 {

    TracyManager::TracyManager() : m_processHandle(nullptr) {
        setType("TracyManager");
    }

    TracyManager::~TracyManager() {
        shutDown();
    }

    TracyManager& TracyManager::getInstance() {
        static TracyManager instance;
        return instance;
    }

    int TracyManager::startUp() {
        if (isStarted()) return 0;
        Manager::startUp();
        return 0;
    }

    void TracyManager::shutDown() {
        if (m_processHandle) {

            // Ask Windows to kill the Tracy GUI process (force-kills)
            // remove should u want to test what happends when shutdown
            TerminateProcess(m_processHandle, 0);

            CloseHandle(m_processHandle);
            m_processHandle = nullptr;
        }
        m_running = false;
        Manager::shutDown();
    }

    void TracyManager::setTracyPath(const std::string& exename) {
        m_tracyPath = exename;// getTracyFilePath(exename);
    }

    void TracyManager::launchTracy() {
#ifndef TRACY_ENABLE
        return;
#else
        // Prevent multiple concurrent profiler instances
        if (m_running && m_processHandle) {
            DWORD exitCode = 0;
            if (GetExitCodeProcess(m_processHandle, &exitCode) && exitCode == STILL_ACTIVE) {
                return; // still running
            }
            else {
                // Process was closed
                CloseHandle(m_processHandle);
                m_processHandle = nullptr;
                m_running = false;
            }
        }

        // Verify that the executable path is valid
        if (m_tracyPath.empty() || !std::filesystem::exists(m_tracyPath)) {
            return;
        }

        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        PROCESS_INFORMATION pi;

        // Auto-connect to localhost game
        std::string args = "\"" + m_tracyPath + "\" -a 127.0.0.1 -capture";

        if (CreateProcessA(
            nullptr, (LPSTR)args.c_str(),
            nullptr, nullptr, FALSE,
            0, nullptr, nullptr,
            &si, &pi))
        {
            LM.writeLog("TracyManager::launchTracy() - Failed to launch Tracy Profiler at: %s", m_tracyPath.c_str());
            CloseHandle(pi.hThread);
            m_processHandle = pi.hProcess;
            m_running = true;
        }
#endif
    }

    void TracyManager::update() {
        if (m_running && m_processHandle) {
            DWORD exitCode = 0;
            if (GetExitCodeProcess(m_processHandle, &exitCode) && exitCode != STILL_ACTIVE) {
                // Process has ended
                CloseHandle(m_processHandle);
                m_processHandle = nullptr;
                m_running = false;
            }
        }
    }

} // namespace gam300
