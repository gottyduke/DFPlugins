#pragma once


#include "DFPE/Prefix.h"
#include "DKUtil/Logger.hpp"
#include "DKUtil/Utility.hpp"

#include <xbyak/xbyak.h>


#define AsAddress(PTR) std::bit_cast<std::uintptr_t>(PTR)
#define AsPointer(ADDR) std::bit_cast<void*>(ADDR)


#define NO_PATCH   \
	{              \
		nullptr, 0 \
	}

#define ASM_MINIMUM_SKIP 2
#define CAVE_MINIMUM_BYTES 0x5
#ifndef CAVE_BUF_SIZE 1 << 7
#	define CAVE_BUF_SIZE 1 << 7
#endif


#define FUNC_INFO(FUNC)                               \
	DKUtil::Hook::FuncInfo                            \
	{                                                 \
		reinterpret_cast<std::uintptr_t>(FUNC), #FUNC \
	}
#define MEM_FUNC_INFO(FUNC)                           \
	DKUtil::Hook::FuncInfo                            \
	{                                                 \
		reinterpret_cast<std::uintptr_t>(FUNC), #FUNC \
	}
#define RT_INFO(FUNC, NAME) \
	DKUtil::Hook::FuncInfo  \
	{                       \
		FUNC, NAME          \
	}

namespace Xbyak
{
	class CodeGenerator;
}


namespace DKUtil
{
	namespace Alias
	{
		using OpCode = std::uint8_t;
		using Disp8 = std::int8_t;
		using Disp16 = std::int16_t;
		using Disp32 = std::int32_t;
		using Imm8 = std::uint8_t;
		using Imm16 = std::uint16_t;
		using Imm32 = std::uint32_t;
		using Imm64 = std::uint64_t;
	}  // namesapce Alias


	template <class To, class From>
	[[nodiscard]] To unrestricted_cast(From a_from) noexcept
	{
		if constexpr (std::is_same_v<
						  std::remove_cv_t<From>,
						  std::remove_cv_t<To>>) {
			return To{ a_from };

			// From != To
		} else if constexpr (std::is_reference_v<From>) {
			return unrestricted_cast<To>(std::addressof(a_from));

			// From: NOT reference
		} else if constexpr (std::is_reference_v<To>) {
			return *unrestricted_cast<
				std::add_pointer_t<
					std::remove_reference_t<To>>>(a_from);

			// To: NOT reference
		} else if constexpr (std::is_pointer_v<From> &&
							 std::is_pointer_v<To>) {
			return static_cast<To>(
				const_cast<void*>(
					static_cast<const volatile void*>(a_from)));
		} else if constexpr ((std::is_pointer_v<From> && std::is_integral_v<To>) ||
							 (std::is_integral_v<From> && std::is_pointer_v<To>)) {
			return std::bit_cast<To>(a_from);
		} else {
			union
			{
				std::remove_cv_t<std::remove_reference_t<From>> from;
				std::remove_cv_t<std::remove_reference_t<To>> to;
			};

			from = std::forward<From>(a_from);
			return to;
		}
	}


#define REL_MAKE_MEMBER_FUNCTION_POD_TYPE_HELPER_IMPL(a_nopropQual, a_propQual, ...)              \
	template <                                                                                    \
		class R,                                                                                  \
		class Cls,                                                                                \
		class... Args>                                                                            \
	struct member_function_pod_type<R (Cls::*)(Args...) __VA_ARGS__ a_nopropQual a_propQual>      \
	{                                                                                             \
		using type = R(__VA_ARGS__ Cls*, Args...) a_propQual;                                     \
	};                                                                                            \
                                                                                                  \
	template <                                                                                    \
		class R,                                                                                  \
		class Cls,                                                                                \
		class... Args>                                                                            \
	struct member_function_pod_type<R (Cls::*)(Args..., ...) __VA_ARGS__ a_nopropQual a_propQual> \
	{                                                                                             \
		using type = R(__VA_ARGS__ Cls*, Args..., ...) a_propQual;                                \
	};

#define REL_MAKE_MEMBER_FUNCTION_POD_TYPE_HELPER(a_qualifer, ...)              \
	REL_MAKE_MEMBER_FUNCTION_POD_TYPE_HELPER_IMPL(a_qualifer, , ##__VA_ARGS__) \
	REL_MAKE_MEMBER_FUNCTION_POD_TYPE_HELPER_IMPL(a_qualifer, noexcept, ##__VA_ARGS__)

#define REL_MAKE_MEMBER_FUNCTION_POD_TYPE(...)                 \
	REL_MAKE_MEMBER_FUNCTION_POD_TYPE_HELPER(, __VA_ARGS__)    \
	REL_MAKE_MEMBER_FUNCTION_POD_TYPE_HELPER(&, ##__VA_ARGS__) \
	REL_MAKE_MEMBER_FUNCTION_POD_TYPE_HELPER(&&, ##__VA_ARGS__)

#define REL_MAKE_MEMBER_FUNCTION_NON_POD_TYPE_HELPER_IMPL(a_nopropQual, a_propQual, ...)              \
	template <                                                                                        \
		class R,                                                                                      \
		class Cls,                                                                                    \
		class... Args>                                                                                \
	struct member_function_non_pod_type<R (Cls::*)(Args...) __VA_ARGS__ a_nopropQual a_propQual>      \
	{                                                                                                 \
		using type = R&(__VA_ARGS__ Cls*, void*, Args...)a_propQual;                                  \
	};                                                                                                \
                                                                                                      \
	template <                                                                                        \
		class R,                                                                                      \
		class Cls,                                                                                    \
		class... Args>                                                                                \
	struct member_function_non_pod_type<R (Cls::*)(Args..., ...) __VA_ARGS__ a_nopropQual a_propQual> \
	{                                                                                                 \
		using type = R&(__VA_ARGS__ Cls*, void*, Args..., ...)a_propQual;                             \
	};

#define REL_MAKE_MEMBER_FUNCTION_NON_POD_TYPE_HELPER(a_qualifer, ...)              \
	REL_MAKE_MEMBER_FUNCTION_NON_POD_TYPE_HELPER_IMPL(a_qualifer, , ##__VA_ARGS__) \
	REL_MAKE_MEMBER_FUNCTION_NON_POD_TYPE_HELPER_IMPL(a_qualifer, noexcept, ##__VA_ARGS__)

#define REL_MAKE_MEMBER_FUNCTION_NON_POD_TYPE(...)                 \
	REL_MAKE_MEMBER_FUNCTION_NON_POD_TYPE_HELPER(, __VA_ARGS__)    \
	REL_MAKE_MEMBER_FUNCTION_NON_POD_TYPE_HELPER(&, ##__VA_ARGS__) \
	REL_MAKE_MEMBER_FUNCTION_NON_POD_TYPE_HELPER(&&, ##__VA_ARGS__)

	template <typename data_t>
	concept dku_h_pod_t =
		std::is_integral_v<data_t> ||
		(std::is_standard_layout_v<data_t> && std::is_trivial_v<data_t>);

	template <typename mem_t>
	concept dku_h_addr_t = std::convertible_to<void*, mem_t> || std::convertible_to<std::uintptr_t, mem_t>;

	namespace detail
	{
		template <class>
		struct member_function_pod_type;

		REL_MAKE_MEMBER_FUNCTION_POD_TYPE();
		REL_MAKE_MEMBER_FUNCTION_POD_TYPE(const);
		REL_MAKE_MEMBER_FUNCTION_POD_TYPE(volatile);
		REL_MAKE_MEMBER_FUNCTION_POD_TYPE(const volatile);

		template <class F>
		using member_function_pod_type_t = typename member_function_pod_type<F>::type;

		template <class>
		struct member_function_non_pod_type;

		REL_MAKE_MEMBER_FUNCTION_NON_POD_TYPE();
		REL_MAKE_MEMBER_FUNCTION_NON_POD_TYPE(const);
		REL_MAKE_MEMBER_FUNCTION_NON_POD_TYPE(volatile);
		REL_MAKE_MEMBER_FUNCTION_NON_POD_TYPE(const volatile);

		template <class F>
		using member_function_non_pod_type_t = typename member_function_non_pod_type<F>::type;

		// https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention

		template <class T>
		struct meets_length_req :
			std::disjunction<
				std::bool_constant<sizeof(T) == 1>,
				std::bool_constant<sizeof(T) == 2>,
				std::bool_constant<sizeof(T) == 4>,
				std::bool_constant<sizeof(T) == 8>>
		{
		};

		template <class T>
		struct meets_function_req :
			std::conjunction<
				std::is_trivially_constructible<T>,
				std::is_trivially_destructible<T>,
				std::is_trivially_copy_assignable<T>,
				std::negation<
					std::is_polymorphic<T>>>
		{
		};

		template <class T>
		struct meets_member_req :
			std::is_standard_layout<T>
		{
		};

		template <class T, class = void>
		struct is_x64_pod :
			std::true_type
		{
		};

		template <class T>
		struct is_x64_pod<
			T,
			std::enable_if_t<
				std::is_union_v<T>>> :
			std::false_type
		{
		};

		template <class T>
		struct is_x64_pod<
			T,
			std::enable_if_t<
				std::is_class_v<T>>> :
			std::conjunction<
				meets_length_req<T>,
				meets_function_req<T>,
				meets_member_req<T>>
		{
		};

		template <class T>
		static constexpr bool is_x64_pod_v = is_x64_pod<T>::value;

		template <
			class F,
			class First,
			class... Rest>
		decltype(auto) invoke_member_function_non_pod(F&& a_func, First&& a_first, Rest&&... a_rest)  //
			noexcept(std::is_nothrow_invocable_v<F, First, Rest...>)
		{
			using result_t = std::invoke_result_t<F, First, Rest...>;
			alignas(result_t) std::byte result[sizeof(result_t)]{};

			using func_t = member_function_non_pod_type_t<F>;
			auto func = std::bit_cast<func_t*>(std::forward<F>(a_func));

			return func(std::forward<First>(a_first), std::addressof(result), std::forward<Rest>(a_rest)...);
		}
	}

	template <class F, class... Args>
	inline std::invoke_result_t<F, Args...> invoke(F&& a_func, Args&&... a_args)  //
		noexcept(std::is_nothrow_invocable_v<F, Args...>)                  //
		requires(std::invocable<F, Args...>)
	{
		if constexpr (std::is_member_function_pointer_v<std::decay_t<F>>) {
			if constexpr (detail::is_x64_pod_v<std::invoke_result_t<F, Args...>>) {  // member functions == free functions in x64
				using func_t = detail::member_function_pod_type_t<std::decay_t<F>>;
				auto* func = dku::unrestricted_cast<func_t*>(std::forward<F>(a_func));
				return func(std::forward<Args>(a_args)...);
			} else {  // shift args to insert result
				return detail::invoke_member_function_non_pod(std::forward<F>(a_func), std::forward<Args>(a_args)...);
			}
		} else {
			return std::forward<F>(a_func)(std::forward<Args>(a_args)...);
		}
	}


	template <typename T = void, typename U>
	[[nodiscard]] constexpr auto adjust_pointer(U* a_ptr, std::ptrdiff_t a_adjust) noexcept
	{
		auto addr = a_ptr ? std::bit_cast<std::uintptr_t>(a_ptr) + a_adjust : 0;
		if constexpr (std::is_const_v<U> && std::is_volatile_v<U>) {
			return std::bit_cast<std::add_cv_t<T>*>(addr);
		} else if constexpr (std::is_const_v<U>) {
			return std::bit_cast<std::add_const_t<T>*>(addr);
		} else if constexpr (std::is_volatile_v<U>) {
			return std::bit_cast<std::add_volatile_t<T>*>(addr);
		} else {
			return std::bit_cast<T*>(addr);
		}
	}

	template <typename T = std::uintptr_t*>
	[[nodiscard]] constexpr auto offset_pointer(const dku_h_addr_t auto& a_ptr, std::ptrdiff_t a_offset) noexcept
	{
		return *adjust_pointer<T>(a_ptr, a_offset);
	}

	template <typename T, typename Mem>
	[[nodiscard]] constexpr T& AsPun(Mem a_mem) noexcept
	{
		return *dku::unrestricted_cast<T*>(a_mem);
	}

	template <typename T>
	constexpr void memzero(volatile T* a_ptr, std::size_t a_size = sizeof(T)) noexcept
	{
		const auto begin = std::bit_cast<volatile char*>(a_ptr);
		constexpr char val{ 0 };
		std::fill_n(begin, a_size, val);
	}


	namespace Hook
	{
		class Trampoline;
		extern Trampoline* default_trampoline() noexcept;

		using REX = std::uint8_t;
		using ModRM = std::uint8_t;
		using SIndex = std::uint8_t;

		using unpacked_data = std::pair<const void*, std::size_t>;
		using offset_pair = std::pair<std::ptrdiff_t, std::ptrdiff_t>;


		enum class HookFlag : std::uint32_t
		{
			kNoFlag = 0,

			kSkipNOP = 1u << 0,              // skip NOPs
			kRestoreBeforeProlog = 1u << 1,  // apply stolens before prolog
			kRestoreAfterProlog = 1u << 2,   // apply stolens after prolog
			kRestoreBeforeEpilog = 1u << 3,  // apply stolens before epilog
			kRestoreAfterEpilog = 1u << 4,   // apply stolens after epilog
		};


		struct Patch
		{
			const void* Data;
			const std::size_t Size;
		};


		struct FuncInfo
		{
			std::uintptr_t Address;
			std::string_view Name;
		};

		using namespace Alias;


		inline std::string_view GetProcessName(HMODULE a_handle = 0) noexcept
		{
			static std::string fileName(MAX_PATH + 1, ' ');
			auto res = ::GetModuleBaseNameA(GetCurrentProcess(), a_handle, fileName.data(), MAX_PATH + 1);
			if (res == 0) {
				fileName = "[ProcessHost]";
				res = 13;
			}

			return { fileName.c_str(), res };
		}


		inline std::string_view GetProcessPath(HMODULE a_handle = 0) noexcept
		{
			static std::string fileName(MAX_PATH + 1, ' ');
			auto res = ::GetModuleFileNameA(a_handle, fileName.data(), MAX_PATH + 1);
			if (res == 0) {
				fileName = "[ProcessHost]";
				res = 13;
			}

			return { fileName.c_str(), res };
		}


		inline void WriteData(const dku_h_addr_t auto& a_dst, const void* a_data, const std::size_t a_size, bool a_requestAlloc = true) noexcept
		{
			if (a_requestAlloc) {
				void(default_trampoline()->allocate(a_size));
			}

			DWORD oldProtect;

			auto success = ::VirtualProtect(AsPointer(a_dst), a_size, PAGE_EXECUTE_READWRITE, std::addressof(oldProtect));
			if (success != FALSE) {
				std::memcpy(AsPointer(a_dst), a_data, a_size);
				success = ::VirtualProtect(AsPointer(a_dst), a_size, oldProtect, std::addressof(oldProtect));
			}

			assert(success != FALSE);
		}

		// imm
		inline void WriteImm(const dku_h_addr_t auto& a_dst, const dku_h_pod_t auto& a_data, bool a_requestAlloc = true) noexcept
		{
			return WriteData(a_dst, std::addressof(a_data), sizeof(a_data), a_requestAlloc);
		}

		// pair patch
		inline void WritePatch(const dku_h_addr_t auto& a_dst, const unpacked_data a_patch, bool a_requestAlloc = true) noexcept
		{
			return WriteData(a_dst, a_patch.first, a_patch.second, a_requestAlloc);
		}

		// xbyak patch
		inline void WritePatch(const dku_h_addr_t auto& a_dst, const Xbyak::CodeGenerator* a_patch, bool a_requestAlloc = true) noexcept
		{
			return WriteData(a_dst, a_patch->getCode(), a_patch->getSize(), a_requestAlloc);
		}

		// struct patch
		inline void WritePatch(const dku_h_addr_t auto& a_dst, const Hook::Patch* a_patch, bool a_requestAlloc = true) noexcept
		{
			return WriteData(a_dst, a_patch->Data, a_patch->Size, a_requestAlloc);
		}

		// util func
		constexpr std::uintptr_t TblToAbs(const dku_h_addr_t auto& a_base, const std::uint16_t a_index, const std::size_t a_size = sizeof(Imm64)) noexcept
		{
			return AsAddress(a_base + a_index * a_size);
		}

		class Module
		{
		public:
			enum class Section : std::size_t
			{
				textx,
				idata,
				rdata,
				data,
				pdata,
				tls,
				textw,
				gfids,
				total
			};
			using SectionDescriptor = std::tuple<Section, std::uintptr_t, std::size_t>;

			constexpr Module() = delete;
			explicit Module(std::uintptr_t a_base)
			{
				if (!a_base) {
					ERROR("DKU_H: Failed to initializing module info with null module base");
				}

				_base = AsAddress(a_base);
				_dosHeader = std::bit_cast<::IMAGE_DOS_HEADER*>(a_base);
				_ntHeader = adjust_pointer<::IMAGE_NT_HEADERS64>(_dosHeader, _dosHeader->e_lfanew);
				_sectionHeader = IMAGE_FIRST_SECTION(_ntHeader);

				const auto total = std::min<std::size_t>(_ntHeader->FileHeader.NumberOfSections, std::to_underlying(Section::total));
				for (auto idx = 0; idx < total; ++idx) {
					const auto section = _sectionHeader[idx];
					auto& sectionNameTbl = dku::static_enum<Section>();
					for (Section name : sectionNameTbl.value_range(Section::textx, Section::gfids)) {
						const auto len = (std::min)(dku::print_enum(name).size(), std::extent_v<decltype(section.Name)>);
						if (std::memcmp(dku::print_enum(name).data(), section.Name + 1, len - 1) == 0) {
							_sections[idx] = std::make_tuple(name, _base + section.VirtualAddress, section.Misc.VirtualSize);
						}
					}
				}
			}
			explicit Module(std::string_view a_filePath)
			{
				const auto base = AsAddress(::GetModuleHandleA(a_filePath.data())) & ~3;
				if (!base) {
					ERROR("DKU_H: Failed to initializing module info with file {}", a_filePath);
				}

				*this = Module(base);
			}

			[[nodiscard]] constexpr auto base() const noexcept { return _base; }
			[[nodiscard]] constexpr auto* dosHeader() const noexcept { return _dosHeader; }
			[[nodiscard]] constexpr auto* ntHeader() const noexcept { return _ntHeader; }
			[[nodiscard]] constexpr auto* sectionHeader() const noexcept { return _sectionHeader; }
			[[nodiscard]] constexpr auto section(Section a_section) noexcept 
			{ 
				auto& [sec, addr, size] = _sections[std::to_underlying(a_section)];
				return std::make_pair(addr, size); 
			}

			[[nodiscard]] static Module& get(const dku_h_addr_t auto a_address) noexcept
			{
				static std::unordered_map<std::uintptr_t, Module> managed;
				
				const auto base = AsAddress(a_address) & ~3;
				if (!managed.contains(base)) {
					managed.try_emplace(base, base);
				} 

				return managed.at(base);
			}

			[[nodiscard]] static Module& get(std::string_view a_filePath = {}) noexcept
			{
				const auto base = AsAddress(::GetModuleHandleA(a_filePath.empty() ? GetProcessPath().data() : a_filePath.data()));
				return get(base);
			}

		private:
			std::uintptr_t _base;
			::IMAGE_DOS_HEADER* _dosHeader;
			::IMAGE_NT_HEADERS64* _ntHeader;
			::IMAGE_SECTION_HEADER* _sectionHeader;
			std::array<SectionDescriptor, std::to_underlying(Section::total)> _sections;
		};

		[[nodiscard]] inline void* GetImportAddress(std::string_view a_moduleName, std::string_view a_libraryName, std::string_view a_importName) noexcept
		{
			if (a_libraryName.empty() || a_importName.empty()) {
				ERROR("DKU_H: IAT hook must have valid library name & method name\nConsider using GetProcessName([Opt]HMODULE)");
			}

			auto& module = Module::get(a_moduleName);
			const auto* dosHeader = module.dosHeader();
			const auto* importTbl = adjust_pointer<const ::IMAGE_IMPORT_DESCRIPTOR>(dosHeader, module.ntHeader()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

			for (void(0); importTbl->Characteristics; ++importTbl) {
				const char* libraryName = adjust_pointer<const char>(dosHeader, importTbl->Name);
				if (!string::iequals(a_libraryName, libraryName)) {
					continue;
				}

				if (!importTbl->FirstThunk || !importTbl->OriginalFirstThunk) {
					break;
				}

				const auto* iat = adjust_pointer<const ::IMAGE_THUNK_DATA>(dosHeader, importTbl->FirstThunk);
				const auto* thunk = adjust_pointer<const ::IMAGE_THUNK_DATA>(dosHeader, importTbl->OriginalFirstThunk);

				for (void(0); iat->u1.Function; ++thunk, ++iat) {
					if (thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) {
						continue;
					}

					const auto* info = adjust_pointer<const ::IMAGE_IMPORT_BY_NAME>(dosHeader, thunk->u1.AddressOfData);

					if (!string::iequals(a_importName, std::bit_cast<const char*>(std::addressof(info->Name[0])))) {
						continue;
					}

					return AsPointer(iat);
				}
			}

			return nullptr;
		}

		// CLib
		namespace detail
		{
			namespace characters
			{
				[[nodiscard]] constexpr bool hexadecimal(char a_ch) noexcept
				{
					return ('0' <= a_ch && a_ch <= '9') ||
					       ('A' <= a_ch && a_ch <= 'F') ||
					       ('a' <= a_ch && a_ch <= 'f');
				}

				[[nodiscard]] constexpr bool space(char a_ch) noexcept
				{
					return a_ch == ' ';
				}

				[[nodiscard]] constexpr bool wildcard(char a_ch) noexcept
				{
					return a_ch == '?';
				}
			}

			namespace rules
			{
				namespace detail
				{
					[[nodiscard]] constexpr OpCode hexacharacters_to_hexadecimal(char a_hi, char a_lo) noexcept
					{
						constexpr auto lut = []() noexcept {
							std::array<OpCode, (std::numeric_limits<OpCode>::max)() + 1> a = {};

							const auto iterate = [&](OpCode a_iFirst, OpCode a_cFirst,
													 OpCode a_cLast) noexcept {
								for (; a_cFirst <= a_cLast; ++a_cFirst, ++a_iFirst) {
									a[a_cFirst] = a_iFirst;
								}
							};

							iterate(0, '0', '9');
							iterate(0xA, 'A', 'F');
							iterate(0xa, 'a', 'f');

							return a;
						}();

						return static_cast<OpCode>(
							lut[static_cast<OpCode>(a_hi)] * 0x10 +
							lut[static_cast<OpCode>(a_lo)]);
					}
				}

				template <char HI, char LO>
				class Hexadecimal
				{
				public:
					[[nodiscard]] static constexpr bool match(OpCode a_byte) noexcept
					{
						constexpr auto expected = detail::hexacharacters_to_hexadecimal(HI, LO);
						return a_byte == expected;
					}
				};

				static_assert(Hexadecimal<'5', '7'>::match(0x57));
				static_assert(Hexadecimal<'6', '5'>::match(0x65));
				static_assert(Hexadecimal<'B', 'D'>::match(0xBD));
				static_assert(Hexadecimal<'1', 'C'>::match(0x1C));
				static_assert(Hexadecimal<'F', '2'>::match(0xF2));
				static_assert(Hexadecimal<'9', 'f'>::match(0x9f));

				static_assert(!Hexadecimal<'D', '4'>::match(0xF8));
				static_assert(!Hexadecimal<'6', '7'>::match(0xAA));
				static_assert(!Hexadecimal<'7', '8'>::match(0xE3));
				static_assert(!Hexadecimal<'6', 'E'>::match(0x61));

				class Wildcard
				{
				public:
					[[nodiscard]] static constexpr bool match(OpCode) noexcept
					{
						return true;
					}
				};

				static_assert(Wildcard::match(0xB9));
				static_assert(Wildcard::match(0x96));
				static_assert(Wildcard::match(0x35));
				static_assert(Wildcard::match(0xE4));

				template <char, char>
				void rule_for() noexcept;

				template <char C1, char C2>
				Hexadecimal<C1, C2>
					rule_for() noexcept
					requires(characters::hexadecimal(C1) && characters::hexadecimal(C2));

				template <char C1, char C2>
				Wildcard rule_for() noexcept
					requires(characters::wildcard(C1) && characters::wildcard(C2));
			}

			template <class... Rules>
			class PatternMatcher
			{
			public:
				static_assert(sizeof...(Rules) >= 1, "must provide at least 1 rule for the pattern matcher");

				[[nodiscard]] constexpr bool match(std::span<const OpCode, sizeof...(Rules)> a_bytes) const noexcept
				{
					std::size_t i = 0;
					return (Rules::match(a_bytes[i++]) && ...);
				}

				[[nodiscard]] constexpr bool match(std::uintptr_t a_address) const noexcept
				{
					return this->match(dku::AsPun<const OpCode[sizeof...(Rules)]>(a_address));
				}

				void match_or_fail(std::uintptr_t a_address) const noexcept
				{
					if (!this->match(dku::AsPun<const OpCode[sizeof...(Rules)]>(a_address))) {
						ERROR("DKU_H: Pattern has failed to match!\n"
							"address : 0x{:X}", a_address);
					}
				}
			};

			void consteval_error(const char* a_error);

			template <string::static_string S, class... Rules>
			[[nodiscard]] constexpr auto do_make_pattern() noexcept
			{
				if constexpr (S.length() == 0) {
					return PatternMatcher<Rules...>();
				} else if constexpr (S.length() == 1) {
					constexpr char c = S[0];
					if constexpr (characters::hexadecimal(c) || characters::wildcard(c)) {
						consteval_error(
							"the given pattern has an unpaired rule (rules are required to be written in pairs of 2)");
					} else {
						consteval_error("the given pattern has trailing characters at the end (which is not allowed)");
					}
				} else {
					using rule_t = decltype(rules::rule_for<S[0], S[1]>());
					if constexpr (std::same_as<rule_t, void>) {
						consteval_error("the given pattern failed to match any known rules");
					} else {
						if constexpr (S.length() <= 3) {
							return do_make_pattern<S.template substr<2>(), Rules..., rule_t>();
						} else if constexpr (characters::space(S.value_at(2))) {
							return do_make_pattern<S.template substr<3>(), Rules..., rule_t>();
						} else {
							consteval_error("a space character is required to split byte patterns");
						}
					}
				}
			}

			template <class... Bytes>
			[[nodiscard]] consteval auto make_byte_array(Bytes... a_bytes) noexcept
				-> std::array<OpCode, sizeof...(Bytes)>
			{
				static_assert((std::integral<Bytes> && ...), "all bytes must be an integral type");
				return { static_cast<OpCode>(a_bytes)... };
			}
		}

		template <string::static_string S>
		[[nodiscard]] constexpr auto make_pattern() noexcept
		{
			return detail::do_make_pattern<S>();
		}

		static_assert(make_pattern<"40 10 F2 ??">().match(
			detail::make_byte_array(0x40, 0x10, 0xF2, 0x41)));
		static_assert(make_pattern<"B8 D0 ?? ?? D4 6E">().match(
			detail::make_byte_array(0xB8, 0xD0, 0x35, 0x2A, 0xD4, 0x6E)));
	}  // namespace Hook
}  // namespace DKUtil
