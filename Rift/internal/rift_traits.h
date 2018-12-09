#pragma once

#include <type_traits>

namespace rift {
	namespace impl {

		// compile-time version of std::all_of

		template <bool ...values> struct static_all_of;

		template <bool ...tail>
		struct static_all_of<true, tail...> : static_all_of<tail...> {};

		template <bool... tail>
		struct static_all_of<false, tail...> : std::false_type {};

		template <> struct static_all_of<> : std::true_type {};

		// Wrapper type to support lamba to std::function conversion
		// Taken from: https://stackoverflow.com/questions/13358672/how-to-convert-a-lambda-to-an-stdfunction-using-templates
		template <class T> struct Identity { using type = T; };

	}
}