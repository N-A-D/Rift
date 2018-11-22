#pragma once

#include <type_traits>

namespace rift {
	namespace util {

		// compile-time version of std::all_of

		template <bool ...values> struct static_all_of;

		template <bool ...tail>
		struct static_all_of<true, tail...> : static_all_of<tail...> {};

		template <bool... tail>
		struct static_all_of<false, tail...> : std::false_type {};

		template <> struct static_all_of<> : std::true_type {};

		// Wrapper type to support lamba to std::function conversion
		template <class T>
		struct identity {
			using type = T;
		};

	}
}