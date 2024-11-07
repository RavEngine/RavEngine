#pragma once
#include "CTTI.hpp"
#include <array>
#include <tuple>

namespace RavEngine{
//to_array, which converts a std::tuple into std::array
template<typename Tuple,typename VTuple = std::remove_reference_t<Tuple>,std::size_t... Indices>
consteval inline std::array<std::common_type_t<std::tuple_element_t<Indices, VTuple>...>,sizeof...(Indices)>
to_array(Tuple&& tuple, std::index_sequence<Indices...>)
{
    return { std::get<Indices>(std::forward<Tuple>(tuple))... };
}
template<typename Tuple, typename VTuple = std::remove_reference_t<Tuple>>
consteval inline decltype(auto) to_array(Tuple&& tuple)
{
    return to_array(std::forward<Tuple>(tuple),std::make_index_sequence<std::tuple_size<VTuple>::value> {} );
}

template<typename ... types>
struct Queryable{
	inline static constexpr std::size_t ntypes = sizeof ... (types);
	typedef std::array<ctti_t,ntypes> arraytype;
#define q_qt_def static const arraytype queryTypes{ CTTI<types>() ... }
#ifndef _MSC_VER
	constexpr q_qt_def;
#endif
	
	inline static consteval const arraytype& GetQueryTypes(){
#ifdef _MSC_VER
		q_qt_def;
#endif
		return queryTypes;
	}
};

template<class base, typename ... types>
struct QueryableDelta{
	inline static constexpr std::size_t ntypes = sizeof ... (types) + base::ntypes;
	typedef std::array<ctti_t,ntypes> arraytype;
#define qb_qt_def static const arraytype queryTypes = to_array(std::tuple_cat(std::array<ctti_t, sizeof ... (types)>{ CTTI<types>() ...}, base::GetQueryTypes()))
#ifndef _MSC_VER
	constexpr qb_qt_def;
#endif
		
	inline static consteval const arraytype& GetQueryTypes(){
#ifdef _MSC_VER
		qb_qt_def;
#endif
		return queryTypes;
	}
};
}
