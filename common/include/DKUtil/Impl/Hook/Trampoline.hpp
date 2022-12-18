#pragma once


#include "Shared.hpp"

#include "Assembly.hpp"


// taken from CommonLibSSE
namespace DKUtil::Hook
{
	class Trampoline
	{
	public:
		using deleter_type = std::function<void(void* a_mem, std::size_t a_size)>;

		Trampoline() = default;
		Trampoline(const Trampoline&) = delete;

		Trampoline(Trampoline&& a_rhs) noexcept { move_from(std::move(a_rhs)); }

		explicit Trampoline(std::string_view a_name) :
			_name(a_name)
		{}

		~Trampoline() { release(); }

		Trampoline& operator=(const Trampoline&) = delete;

		Trampoline& operator=(Trampoline&& a_rhs) noexcept 
		{
			if (this != std::addressof(a_rhs)) {
				move_from(std::move(a_rhs));
			}
			return *this;
		}

		void create(std::size_t a_size) { return create(a_size, nullptr); }

		void create(std::size_t a_size, void* a_module)
		{
			if (a_size == 0) {
				ERROR("DKU_H: Cannot create a trampoline with a zero size");
			}

			if (!a_module) {
				const auto& [text, size] = Module::get().section(Module::Section::textx);
				a_module = AsPointer(text + size);
			}

			auto* mem = do_create(a_size, AsAddress(a_module));
			if (!mem) {
				ERROR("DKU_H: Failed to create trampoline");
			}

			set_trampoline(mem, a_size, [](void* a_mem, std::size_t) { ::VirtualFree(a_mem, 0, MEM_RELEASE); });
		}

		void set_trampoline(void* a_trampoline, std::size_t a_size) { set_trampoline(a_trampoline, a_size, {}); }

		void set_trampoline(void* a_trampoline, std::size_t a_size, deleter_type a_deleter)
		{
			auto* trampoline = static_cast<std::byte*>(a_trampoline);
			if (trampoline) {
				std::memset(trampoline, Assembly::INT3, a_size);
			}

			release();

			_deleter = std::move(a_deleter);
			_data = trampoline;
			_capacity = a_size;
			_size = 0;

			log_stats();
		}

		template <typename T = void*>
		[[nodiscard]] T allocate(std::size_t a_size)
		{
			auto result = do_allocate(a_size);
			log_stats();
			return std::bit_cast<T>(result);
		}

		template <typename T = void*>
		[[nodiscard]] T allocate(Xbyak::CodeGenerator& a_code)
		{
			auto result = do_allocate(a_code.getSize());
			log_stats();
			std::memcpy(result, a_code.getCode(), a_code.getSize());
			return std::bit_cast<T>(result);
		}

		template <typename T = void*>
		[[nodiscard]] T allocate()
		{
			return std::bit_cast<T>(allocate(sizeof(T)));
		}

		[[nodiscard]] constexpr std::size_t empty() const noexcept { return _capacity == 0; }
		[[nodiscard]] constexpr std::size_t capacity() const noexcept { return _capacity; }
		[[nodiscard]] constexpr std::size_t allocated_size() const noexcept { return _size; }
		[[nodiscard]] constexpr std::size_t free_size() const noexcept { return _capacity - _size; }

		template <std::size_t N>
		std::uintptr_t write_branch(std::uintptr_t a_src, std::uintptr_t a_dst)
		{
			std::uint8_t data = 0;
			if constexpr (N == 5) {
				// E9 cd
				// JMP rel32
				data = 0xE9;
			} else if constexpr (N == 6) {
				// FF /4
				// JMP r/m64
				data = 0x25;
			} else {
				static_assert(false && N, "invalid branch size");
			}

			return write_branch<N>(a_src, a_dst, data);
		}

		template <std::size_t N, class F>
		std::uintptr_t write_branch(std::uintptr_t a_src, F a_dst)
		{
			return write_branch<N>(a_src, AsAddress(a_dst));
		}

		template <std::size_t N>
		std::uintptr_t write_call(std::uintptr_t a_src, std::uintptr_t a_dst)
		{
			std::uint8_t data = 0;
			if constexpr (N == 5) {
				// E8 cd
				// CALL rel32
				data = 0xE8;
			} else if constexpr (N == 6) {
				// FF /2
				// CALL r/m64
				data = 0x15;
			} else {
				static_assert(false && N, "invalid call size");
			}

			return write_branch<N>(a_src, a_dst, data);
		}

		template <std::size_t N, class F>
		std::uintptr_t write_call(std::uintptr_t a_src, F a_dst)
		{
			return write_call<N>(a_src, AsAddress(a_dst));
		}

	private:
		// https://stackoverflow.com/a/54732489
		[[nodiscard]] void* do_create(std::size_t a_size, std::uintptr_t a_address)
		{
			constexpr std::size_t minRange = numbers::gigabyte(2);
			constexpr std::uintptr_t maxAddr = (std::numeric_limits<std::uintptr_t>::max)();

			::DWORD granularity;
			::SYSTEM_INFO si;
			::GetSystemInfo(&si);
			granularity = si.dwAllocationGranularity;

			std::uintptr_t min = a_address >= minRange ? numbers::roundup(a_address - minRange, granularity) : 0;
			const std::uintptr_t max = a_address < (maxAddr - minRange) ? numbers::rounddown(a_address + minRange, granularity) : maxAddr;
			std::uintptr_t addr;

			::MEMORY_BASIC_INFORMATION mbi;
			do {
				if (!::VirtualQuery(AsPointer(min), std::addressof(mbi), sizeof(mbi))) {
					ERROR("DKU_H: VirtualQuery failed with code: 0x{:X}", ::GetLastError());
					return nullptr;
				}

				auto baseAddr = AsAddress(mbi.BaseAddress);
				min = baseAddr + mbi.RegionSize;

				if (mbi.State == MEM_FREE) {
					addr = numbers::roundup(baseAddr, granularity);

					// if rounding didn't advance us into the next region and the region is the required size
					if (addr < min && (min - addr) >= a_size) {
						auto* mem = ::VirtualAlloc(AsPointer(addr), a_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
						if (mem) {
							DEBUG("DKU_H: Trampoline created successfully!\n"
								"base     : 0x{:X}\n"
								"entry    : 0x{:X} with size {}B\n"
								"distance : 0x{:X}",
								a_address, AsAddress(mem), a_size, static_cast<std::int64_t>(a_address - AsAddress(mem)));

							return mem;
						} else {
							WARN("DKU_H: VirtualAlloc failed with code: 0x{:X}", ::GetLastError());
						}
					}
				}
			} while (min < max);

			return nullptr;
		}

		[[nodiscard]] void* do_allocate(std::size_t a_size)
		{
			if (a_size > free_size()) {
				ERROR("DKU_H: Failed to handle allocation request");
			}

			auto* mem = _data + _size;
			_size += a_size;

			return mem;
		}

		void write_5branch(std::uintptr_t a_src, std::uintptr_t a_dst, OpCode a_opcode)
		{
			using namespace Assembly;

			std::uintptr_t mem{ 0 };

			if (const auto it = _5branches.find(a_dst); it != _5branches.end()) {
				mem = AsAddress(it->second);
			} else {
				mem = allocate<std::uintptr_t>(sizeof(Imm64) + sizeof(JmpRip<false>));
				_5branches.emplace(a_dst, std::bit_cast<std::byte*>(mem));
			}

			const auto disp = mem + sizeof(Imm64) - (a_src + sizeof(JmpRel));
			if (!in_range(disp)) {  // the trampoline should already be in range, so this should never happen
				ERROR("DKU_H: Displacement is out of range");
			}

			JmpRel detour{};
			detour.Jmp = static_cast<OpCode>(a_opcode);
			detour.Disp = static_cast<Imm32>(disp);
			WriteData(a_src, &detour, sizeof(JmpRel), false);

			AsPun<std::uintptr_t>(mem) = a_dst;
			mem += sizeof(Imm64);

			AsPun<JmpRip<false>>(mem) = JmpRip(-sizeof(Imm64) - sizeof(JmpRip<false>));
		}

		void write_6branch(std::uintptr_t a_src, std::uintptr_t a_dst, ModRM a_modrm)
		{
			using namespace Assembly;

			std::uintptr_t mem{ 0 };
			if (const auto it = _6branches.find(a_dst); it != _6branches.end()) {
				mem = AsAddress(it->second);
			} else {
				mem = allocate<std::uintptr_t>();
				_6branches.emplace(a_dst, std::bit_cast<std::byte*>(mem));
			}

			const auto disp = mem - (a_src + sizeof(JmpRel));
			if (!in_range(disp)) {  // the trampoline should already be in range, so this should never happen
				ERROR("DKU_H: Displacement is out of range");
			}

			JmpRip detour{};
			detour.Jmp = static_cast<OpCode>(0xFF);
			detour.Rip = static_cast<ModRM>(a_modrm);
			detour.Disp = static_cast<Disp32>(disp);
			WriteData(a_src, &detour, sizeof(detour), false);

			AsPun<std::uintptr_t>(mem) = a_dst;
		}

		template <std::size_t N>
		[[nodiscard]] std::uintptr_t write_branch(std::uintptr_t a_src, std::uintptr_t a_dst, OpCode a_data)
		{
			const auto nextOp = a_src + N;
			const auto func = nextOp + AsPun<Disp32>(a_src + N - sizeof(Disp32));

			if constexpr (N == 5) {
				write_5branch(a_src, a_dst, a_data);
			} else if constexpr (N == 6) {
				write_6branch(a_src, a_dst, a_data);
			} else {
				static_assert(false && N, "invalid branch size");
			}

			return func;
		}

		void move_from(Trampoline&& a_rhs)
		{
			_5branches = std::move(a_rhs._5branches);
			_6branches = std::move(a_rhs._6branches);
			_name = std::move(a_rhs._name);

			_deleter = std::move(a_rhs._deleter);

			_data = a_rhs._data;
			a_rhs._data = nullptr;

			_capacity = a_rhs._capacity;
			a_rhs._capacity = 0;

			_size = a_rhs._size;
			a_rhs._size = 0;
		}

		void log_stats() const
		{
			auto pct = (static_cast<double>(_size) / static_cast<double>(_capacity)) * 100.0;
			DEBUG("{} => {}B / {}B ({:05.2f}%)", _name, _size, _capacity, pct);
		}

		[[nodiscard]] bool in_range(std::ptrdiff_t a_disp) const
		{
			constexpr auto min = (std::numeric_limits<std::int32_t>::min)();
			constexpr auto max = (std::numeric_limits<std::int32_t>::max)();

			return min <= a_disp && a_disp <= max;
		}

		void release()
		{
			if (_data && _deleter) {
				_deleter(_data, _capacity);
			}

			_5branches.clear();
			_6branches.clear();
			_data = nullptr;
			_capacity = 0;
			_size = 0;
		}

		std::map<std::uintptr_t, std::byte*> _5branches;
		std::map<std::uintptr_t, std::byte*> _6branches;
		std::string _name{ "Default Trampoline"sv };
		deleter_type _deleter;
		std::byte* _data{ nullptr };
		std::size_t _capacity{ 0 };
		std::size_t _size{ 0 };
	};


	[[nodiscard]] inline Trampoline* default_trampoline() noexcept
	{
		static Trampoline instance;
		return std::addressof(instance);
	}
}  // namespace DKUtil::Hook
