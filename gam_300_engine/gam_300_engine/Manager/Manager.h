/**
 * @file Manager.h
 * @brief Implementation of the Manager base class.
 * @details Contains declaration for all manager member functions.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <string>

namespace gam300 {
	class Manager {

	private:
		std::string m_type;		// Manager type identifier.
		bool m_is_started;		// True if startUp() succeeded.

	protected:
		// Set type identifier of Manager.
		void setType(std::string new_type);

	public:
		// Default constructor.
		Manager();

		// Default destructor.
		virtual ~Manager();

		// Get type identifier of Manager.
		std::string getType() const;

		// Startup Manager. 
		// Return 0 if ok, else negative number.
		virtual int startUp();

		// Shutdown Manager.  
		virtual void shutDown();

		// Return status of is_started (true when startUp() was successful).
		bool isStarted() const;
	};

} // end of namespace game300
#endif // __MANAGER_H__