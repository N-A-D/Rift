#pragma once

namespace rift {
	namespace impl {

		class NonCopyable {
		public:
			NonCopyable() = default;
			virtual ~NonCopyable() = default;
			NonCopyable(const NonCopyable&) = delete;
			NonCopyable& operator=(const NonCopyable&) = delete;
		};

	} // namespace impl
} // namespace rift