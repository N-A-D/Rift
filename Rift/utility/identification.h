#pragma once

#include <cstdint>

namespace rift {

	// The ID class
	// Purpose is to serve as a versionable index
	class ID final {
	public:
		ID() = default;
		ID(std::uint32_t index, std::uint32_t version);
		
		operator std::uint64_t() const noexcept;
		std::uint32_t index() const noexcept;
		std::uint32_t version() const noexcept;

	private:
		std::uint64_t m_number;
	};

}