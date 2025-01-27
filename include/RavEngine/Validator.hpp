#pragma once
#include <type_traits>
#include <tuple>

namespace RavEngine {
	template<typename ... A> struct ValidatorProvider;

	template <typename T, typename ... A>
	concept TinVariadic = std::disjunction_v<std::is_same<T, A>...>;

	template<typename ... A>
	struct Validator {
		friend struct ValidatorProvider<A...>;	// access to default constructor

		using types = std::tuple<A...>;

		template<typename T>
		constexpr static bool IsValid() {
			return TinVariadic<T, A...>;
		}

		// no copy, no assign
		Validator(const Validator&) = delete;
		void operator=(Validator const&) = delete;

	private:
		Validator() {};
		// private move
		Validator(Validator&&) = default;
	};


}