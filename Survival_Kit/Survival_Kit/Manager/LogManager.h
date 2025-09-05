/**
 * @file LogManager.h
 * @brief Declaration of the Log Manager for the game engine.
 * @details Handles writing to log files for debugging and error reporting.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __LOG_MANAGER_H__
#define __LOG_MANAGER_H__

 // System includes.
#include <stdio.h>
#include <map>
#include <stdarg.h>   // Moved from LogManager.cpp
#include <time.h>     // Moved from LogManager.cpp
#include <string.h>   // Moved from LogManager.cpp

// Engine includes.
#include "Manager.h"

// Two-letter acronym for easier access to manager.
#define LM gam300::LogManager::getInstance()

namespace gam300 {

	const std::string LOGFILE_DEFAULT = "Survival_Kit.log";

	class LogManager : public Manager {

	private:
		LogManager();										// Private since a singleton.
		LogManager(LogManager const&);						// Don't allow copy.
		LogManager& operator=(LogManager const&) = delete;  // Don't allow assignment.
		bool m_do_flush;									// True if flush to disk after write.
		FILE* m_p_f;										// Pointer to main logfile.

	public:
		// If logfile is open, close it.
		~LogManager();

		// Get the one and only instance of the LogManager.
		static LogManager& getInstance();

		/**
		 * @brief Start up LogManager.
		 * @return 0 if successful, else -1.
		 * @details Opens main logfile, usually "gam300.log".
		 */
		int startUp();

		/**
		 * @brief Shut down LogManager.
		 * @details Closes all logfiles.
		 */
		void shutDown();

		/**
		 * @brief Write to logfile.
		 * @param fmt Format string supporting printf() formatting.
		 * @param ... Variable arguments for formatting.
		 * @return Number of bytes written (excluding prepends), -1 if error.
		 */
		int writeLog(const char* fmt, ...) const;

		/**
		 * @brief Set flush of logfile after each write.
		 * @param new_do_flush New flush setting (default: true).
		 */
		void setFlush(bool new_do_flush = true);
	};

} // end of namespace gam300
#endif // __LOG_MANAGER_H__