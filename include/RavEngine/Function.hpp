#pragma once
#include <boost/function.hpp>

namespace RavEngine{

template<typename ... A>
using Function = boost::function<A...>;

}
