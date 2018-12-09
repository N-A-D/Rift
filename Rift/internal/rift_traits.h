#pragma once

#include <type_traits>

namespace rift {
	namespace impl {

		// Compile-time version of std::all_of 
		// Taken from: https://stackoverflow.com/questions/27221443/implementing-static-version-of-stdall-of-using-template-metaprogramming
		template <bool... values> struct bool_pack;
		template <bool... values> struct all_of 
			: std::is_same<bool_pack<values..., true>, bool_pack<true, values...>> {};

		// Wrapper type to support lamba to std::function conversion
		// Taken from: https://stackoverflow.com/questions/13358672/how-to-convert-a-lambda-to-an-stdfunction-using-templates
		template <class T> struct identity { using type = T; };

	}
}