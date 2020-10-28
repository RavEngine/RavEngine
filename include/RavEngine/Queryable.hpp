#pragma once
#include <typeindex>
#include <array>

namespace RavEngine{

//joins two std::arrays of different size at compile time
template <typename Type, std::size_t... sizes>
auto concat(const std::array<Type, sizes>&... arrays)
{
	Type result[(sizes + ...)] = {std::type_index(typeid(Type))};
	std::size_t index{};
	
	((std::copy_n(arrays.begin(), sizes, result.begin() + index), index += sizes), ...);
	
	return result;
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
		const std::array<std::type_index,sizeof ... (types)> thisvalues{ std::type_index(typeid(types)) ...};
		const typename base::arraytype basevalues = base::GetQueryTypes();
		return concat(thisvalues, basevalues);
	}
};
}
