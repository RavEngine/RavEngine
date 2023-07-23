#pragma once
#include <tuple>

namespace RavEngine {
	// adapted from answers on https://stackoverflow.com/questions/28509273/get-types-of-c-function-parameters

	using namespace std;
	template<typename Sig>
	struct signature;
	template<typename Ret, typename...Args>
	struct signature<Ret(Args...)> {
		using type = tuple<Args...>;
	};

	template<typename Ret, typename Obj, typename...Args>
	struct signature<Ret(Obj::*)(Args...)> {
		using type = tuple<Args...>;
	};
	template<typename Ret, typename Obj, typename...Args>
	struct signature<Ret(Obj::*)(Args...)const> {
		using type = tuple<Args...>;
	};
	template<typename Fun>
	concept is_fun = is_function_v<Fun>;

	template<typename Fun>
	concept is_mem_fun = is_member_function_pointer_v<decay_t<Fun>>;

	template<typename Fun>
	concept is_functor = is_class_v<decay_t<Fun>> && requires(Fun && t) {
		&decay_t<Fun>::operator();
	};

	template<is_functor T>
    using functor_args_t = typename signature<decltype(&decay_t<T>::operator())>::type;

	template<is_functor T>
    auto arguments(T&& t) -> typename signature<decltype(&decay_t<T>::operator())>::type;

	template<is_functor T>
    auto arguments(const T& t) -> typename signature<decltype(&decay_t<T>::operator())>::type;

	// template<is_fun T>
	// auto arguments(T&& t)->signature<T>::type;

	template<is_fun T>
	auto arguments(const T& t) -> typename signature<T>::type;

	template<is_mem_fun T>
    auto arguments(T&& t) -> typename signature<decay_t<T>>::type;
    
	template<is_mem_fun T>
    auto arguments(const T& t) -> typename signature<decay_t<T>>::type;
}
