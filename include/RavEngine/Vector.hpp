#pragma once
#include <vector>
#include "unordered_vector.hpp"

namespace RavEngine{

    template<typename T>
    using Vector = std::vector<T>;

    template<typename T>
    using UnorderedVector = unordered_vector<T,Vector<T>>;

}
