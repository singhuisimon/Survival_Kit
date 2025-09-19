#pragma once
#include <string>
#include <vector>
#include "../Pipeline/AssetDatabase.h"

namespace gam300
{

	class AssetManager {

		struct Config {

			std::vector<std::string> sourceRoots;
			std::string intermediateDirectory;
			std::string databaseFile;
		};

	}; //end of class AssetManager

} //end of namespace gam300

