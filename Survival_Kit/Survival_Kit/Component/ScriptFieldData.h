#pragma once
#include <string>
#include <vector>
#include <variant>

namespace gam300 {

    enum class ScriptFieldType {
        FLOAT,
        INT,
        BOOL,
        STRING,
        UNKNOWN
    };

    struct ScriptFieldData {
        std::string name;
        ScriptFieldType type;

        // Use variant to store different types
        std::variant<float, int, bool, std::string> value;

        bool hasRange;
        float rangeMin;
        float rangeMax;

        std::string tooltip;
        std::string displayName;

        ScriptFieldData()
            : type(ScriptFieldType::UNKNOWN)
            , hasRange(false)
            , rangeMin(0.0f)
            , rangeMax(0.0f)
        {
        }
    };

} // namespace gam300