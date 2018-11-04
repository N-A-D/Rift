#pragma once

#include <random>
#include <cassert>
#include <type_traits>

namespace rift {
	namespace details {

		// Returns a random floating point number in the range [0,1]
		template <class T>
		typename std::enable_if<std::is_floating_point<T>::value, T>::type
			random() {
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<T> dis(0, 1);
			return dis(gen);
		}

		// Returns a random floating point number in range [min, max]
		template <class T>
		typename std::enable_if<std::is_floating_point<T>::value, T>::type
			random_in_range(T min, T max) {
			assert(min <= max);
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<T> dis(min, max);
			return dis(gen);
		}

	}
}