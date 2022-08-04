#pragma once
//#include <boost/function.hpp>
#include <functional>

namespace RavEngine{

template<typename ... A>
using Function = std::function<A...>;

}
