#include "identification.h"

rift::ID::ID(std::uint32_t index, std::uint32_t version)
	: m_number(std::uint64_t(version) << 32 | std::uint64_t(index))
{
}


rift::ID::operator std::uint64_t() const noexcept
{
	return m_number;
}

std::uint32_t rift::ID::index() const noexcept
{
	return m_number & 0xFFFFFFFFUL;
}

std::uint32_t rift::ID::version() const noexcept
{
	return m_number >> 32;
}
