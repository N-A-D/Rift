#pragma once

#include <type_traits>

namespace rift {
	namespace internal {

		// Compile-time version of std::all_of 
		// Taken from: https://stackoverflow.com/questions/27221443/internalementing-static-version-of-stdall-of-using-template-metaprogramming
		template <bool... values> struct bool_pack;
		template <bool... values> constexpr bool all_of_v = std::is_same_v<bool_pack<values..., true>, bool_pack<true, values...>>;

		// Wrapper type to support lamba to std::function conversion
		// Taken from: https://stackoverflow.com/questions/13358672/how-to-convert-a-lambda-to-an-stdfunction-using-templates
		template <class T> struct identity { using type = T; };
		template <class T> using identity_t = typename identity<T>::type;

	} // namespace internal
} // namespace rift