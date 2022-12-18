#pragma once

#include "DFPE/Prefix.h"

#include "DKUtil/Hook.hpp"


// CLib
namespace RE
{
	class Offset
	{
	public:
		constexpr Offset() = default;
		constexpr Offset(std::ptrdiff_t a_offset) :
			_offset(a_offset)
		{}

		[[nodiscard]] constexpr std::uintptr_t get() const noexcept
		{
			return _offset;
		}

		[[nodiscard]] constexpr std::uintptr_t full() const noexcept
		{
			return dku::Hook::Module::get().base() + _offset;
		}

	private:
		std::ptrdiff_t _offset{ 0 };
	};


	template <typename T = std::uintptr_t, typename U = std::conditional_t<std::is_member_pointer_v<T> || std::is_function_v<std::remove_pointer_t<T>>, std::decay_t<T>, T>>
	class Relocation
	{
	public:
		constexpr Relocation() noexcept = default;
		constexpr Relocation(const dku::dku_h_addr_t auto& a_addr) noexcept :
			_address(std::bit_cast<std::uintptr_t>(a_addr))
		{}
		constexpr Relocation(Offset a_rva) noexcept :
			_address(a_rva.full())
		{}
		constexpr Relocation(Offset a_rva, std::ptrdiff_t a_offset) noexcept :
			_address(a_rva.full() + a_offset)
		{}

		[[nodiscard]] constexpr U get() const
			noexcept(std::is_nothrow_copy_constructible_v<U>)
		{
			return std::bit_cast<U>(_address);
		}

		[[nodiscard]] constexpr decltype(auto) address() const noexcept
		{
			return _address;
		}

		[[nodiscard]] constexpr decltype(auto) operator*() const noexcept
			requires(std::is_pointer_v<U>)
		{
			return *get();
		}

		template <class... Args>
		std::invoke_result_t<const U&, Args...> operator()(Args&&... a_args) const  //
			noexcept(std::is_nothrow_invocable_v<const U&, Args...>)                //
			requires(std::invocable<const U&, Args...>)
		{
			return dku::invoke(get(), std::forward<Args>(a_args)...);
		}

		template <dku::string::static_string Pattern>
		[[nodiscard]] bool match() noexcept
		{
			return dku::Hook::make_pattern<Pattern>().match(_address);
		}

	private:
		std::uintptr_t _address{ 0 };
	};
}  // namespace DFPE::RE