#pragma once

namespace rift {
	namespace internal {

		class NonCopyable {
		public:
			NonCopyable() = default;
			virtual ~NonCopyable() = default;
			NonCopyable(const NonCopyable&) = delete;
			NonCopyable& operator=(const NonCopyable&) = delete;
		};

	} // namespace internal
} // namespace rift