/**
 * @file LogManager.cpp
 * @brief Implementation of the Log Manager for the game engine.
 * @details Handles writing to log files for debugging and error reporting.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

 // Include required headers
#include "LogManager.h"

namespace gam300 {

    // Initialize the singleton instance pointer
    LogManager::LogManager() {
        setType("LogManager");
        m_p_f = NULL;
        m_do_flush = false;
    }

    // Destructor - close the log file if it's open
    LogManager::~LogManager() {
        shutDown();
    }

    // Get the singleton instance
    LogManager& LogManager::getInstance() {
        static LogManager instance;
        return instance;
    }

    // Start up the LogManager - open the main log file
    int LogManager::startUp() {
        // Call parent's startUp() first
        if (Manager::startUp())
            return -1;

        // Try to open the log file using secure version
        errno_t err = fopen_s(&m_p_f, LOGFILE_DEFAULT.c_str(), "w");
        if (err != 0 || m_p_f == NULL) {
            return -1;
        }

        // Write header to log file with timestamp
        time_t now = time(NULL);
        char timestamp[26];  // Enough space for the formatted time string
        struct tm timeinfo;

        // Format the time using secure version
        localtime_s(&timeinfo, &now);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

        // Write the header
        fprintf(m_p_f, "=== GAM300 LOG START: %s ===\n", timestamp);

        // If flush is enabled, make sure it's written to disk
        if (m_do_flush) {
            fflush(m_p_f);
        }

        return 0;
    }

    // Shut down the LogManager - close the log file
    void LogManager::shutDown() {
        // If the log file is open, close it
        if (m_p_f != NULL) {
            // Write footer with timestamp
            time_t now = time(NULL);
            char timestamp[26];
            struct tm timeinfo;

            // Format the time using secure version
            localtime_s(&timeinfo, &now);
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

            // Write the footer
            fprintf(m_p_f, "=== GAM300 LOG END: %s ===\n", timestamp);

            // Close the file
            fclose(m_p_f);
            m_p_f = NULL;
        }

        // Call parent's shutDown()
        Manager::shutDown();
    }

    // Write to the log file with printf-style formatting
    int LogManager::writeLog(const char* fmt, ...) const {
        // If the log file isn't open, return error
        if (m_p_f == NULL) {
            return -1;
        }

        // Get current time for timestamp
        time_t now = time(NULL);
        char timestamp[26];
        struct tm timeinfo;

        // Format the time using secure version
        localtime_s(&timeinfo, &now);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

        // Write timestamp prefix
        fprintf(m_p_f, "[%s] ", timestamp);

        // Set up variable arguments
        va_list args;
        va_start(args, fmt);

        // Write formatted message to log file
        int bytes_written = vfprintf(m_p_f, fmt, args);

        // End variable arguments
        va_end(args);

        // Add a newline if the message doesn't end with one
        if (bytes_written > 0 && fmt[strlen(fmt) - 1] != '\n') {
            fprintf(m_p_f, "\n");
        }

        // If flush is enabled, make sure it's written to disk
        if (m_do_flush) {
            fflush(m_p_f);
        }

        return bytes_written;
    }

    // Set whether to flush after each write
    void LogManager::setFlush(bool new_do_flush) {
        m_do_flush = new_do_flush;
    }

} // end of namespace gam300