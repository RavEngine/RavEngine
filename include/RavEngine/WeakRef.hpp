//
//  WeakSharedObjectRef.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

# pragma once
#include <type_traits>
#include <functional>

template<typename T>
using WeakRef = std::weak_ptr<T>;

