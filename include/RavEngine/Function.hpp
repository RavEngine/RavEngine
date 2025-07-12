#pragma once
#include <functional>

namespace RavEngine{

template<typename ... A>
using Function = std::function<A...>;

}
