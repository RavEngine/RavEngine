#pragma once
#include <typeindex>
#include <array>
#include <tuple>

namespace RavEngine{
//to_array, which converts a std::tuple into std::array
template<typename Tuple,typename VTuple = std::remove_reference_t<Tuple>,std::size_t... Indices>
constexpr std::array<std::common_type_t<std::tuple_element_t<Indices, VTuple>...>,sizeof...(Indices)>
to_array(Tuple&& tuple, std::index_sequence<Indices...>)
{
    return { std::get<Indices>(std::forward<Tuple>(tuple))... };
}
template<typename Tuple, typename VTuple = std::remove_reference_t<Tuple>>
constexpr decltype(auto) to_array(Tuple&& tuple)
{
    return to_array(std::forward<Tuple>(tuple),std::make_index_sequence<std::tuple_size<VTuple>::value> {} );
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
        //concatenate arrays
        return to_array(std::tuple_cat(thisvalues,basevalues));
	}
};
}
