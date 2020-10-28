#pragma once
#include <typeindex>

namespace RavEngine{

//joins two std::arrays of different size at compile time
template <class Target=void, class... TupleLike>
auto array_concat(TupleLike&&... tuples) {
	return std::apply([](auto&& first, auto&&... rest){
		using T = std::conditional_t<
		!std::is_void<Target>::value, Target, std::decay_t<decltype(first)>>;
		return std::array<T, sizeof...(rest)+1>{{
			decltype(first)(first), decltype(rest)(rest)...
		}};
	}, std::tuple_cat(std::forward<TupleLike>(tuples)...));
}

template<typename ... types>
struct Queryable{
	inline static constexpr size_t ntypes = sizeof ... (types);
	typedef std::array<std::type_index,ntypes> arraytype;
	
	inline static constexpr arraytype GetQueryTypes(){
		return {std::type_index(typeid(types)) ...};
	}
};

template<typename base, typename ... types>
struct QueryableDelta{
	inline static constexpr size_t ntypes = sizeof ... (types) + base::ntypes;
	typedef std::array<std::type_index,ntypes> arraytype;
	
	inline static constexpr arraytype GetQueryTypes(){
		return array_concat({std::type_index(typeid(types)) ...}, base::GetQueryTypes());
	}
};
}
