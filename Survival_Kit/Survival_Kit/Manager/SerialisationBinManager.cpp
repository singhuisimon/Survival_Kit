#include "SerialisationBinManager.h"

// This translation unit intentionally contains no out-of-line definitions 
// to avoid template/linker issues.

// Usage: 
// Ensure that all component to be serialized has a function called as_tuple() 
// This will return a tuple class which will be used to serialize the data as 
// binary data. The components will be recursively looped but has not been 
// extensively tested for robustness so inform Yue Jun if any issues are found. 
// Below are examples of potential components that are supported. 

// struct Component1
// {
//     int a;
//     std::string s;
//     std::vector<float> v;
//
//     auto as_tuple()
//     {
//         return std::tie(a, s, v);
//     }
//
//     auto as_tuple() const
//     {
//         return std::tie(a, s, v);
//     }
// };

// struct Component2
// {
//     std::uint32_t id;
//     double score;
//     Inner payload;
//     std::vector<Inner> arr;
//     std::string name;
//
//     auto as_tuple()
//     {
//         return std::tie(id, score, payload, arr, name);
//     }
//
//     auto as_tuple() const
//     {
//         return std::tie(id, score, payload, arr, name);
//     }
// };
