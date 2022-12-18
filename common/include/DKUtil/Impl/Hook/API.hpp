#pragma once


#include "Assembly.hpp"
#include "Internal.hpp"
#include "Shared.hpp"
#include "Trampoline.hpp"


namespace DKUtil::Hook
{
	inline auto AddASMPatch_Xbyak(
		const std::uintptr_t a_address,
		const offset_pair a_offset,
		const Xbyak::CodeGenerator* a_xbyak,
		const bool a_forward = true) noexcept
	{
		return AddASMPatch(a_address, a_offset, std::make_pair(a_xbyak->getCode(), a_xbyak->getSize()), a_forward);
	}

	inline auto AddASMPatch_Patch(
		const std::uintptr_t a_address,
		const offset_pair a_offset,
		const Patch* a_patch,
		const bool a_forward = true) noexcept
	{
		return AddASMPatch(a_address, a_offset, std::make_pair(a_patch->Data, a_patch->Size), a_forward);
	}

	inline auto AddCaveHook_Xbyak(
		const std::uintptr_t a_address,
		const offset_pair a_offset,
		const FuncInfo a_funcInfo,
		const Xbyak::CodeGenerator* a_prolog,
		const Xbyak::CodeGenerator* a_epilog,
		model::enumeration<HookFlag> a_flag = HookFlag::kSkipNOP) noexcept
	{
		return AddCaveHook(
			a_address, a_offset, a_funcInfo,
			std::make_pair(a_prolog->getCode(), a_prolog->getSize()),
			std::make_pair(a_epilog->getCode(), a_epilog->getSize()),
			a_flag);
	}

	inline auto AddCaveHook_Patch(
		const std::uintptr_t a_address,
		const offset_pair a_offset,
		const FuncInfo a_funcInfo,
		const Patch* a_prolog,
		const Patch* a_epilog,
		model::enumeration<HookFlag> a_flag = HookFlag::kSkipNOP) noexcept
	{
		return AddCaveHook(
			a_address, a_offset, a_funcInfo, 
			std::make_pair(a_prolog->Data, a_prolog->Size), 
			std::make_pair(a_epilog->Data, a_epilog->Size),
			a_flag);
	}

	inline auto AddVMTHook_Xbyak(
		const void* a_vtbl,
		const std::uint16_t a_index,
		const FuncInfo a_funcInfo,
		const Xbyak::CodeGenerator* a_xbyak) noexcept
	{
		return AddVMTHook(a_vtbl, a_index, a_funcInfo, std::make_pair(a_xbyak->getCode(), a_xbyak->getSize()));
	}

	inline auto AddVMTHook_Patch(
		const void* a_vtbl,
		const std::uint16_t a_index,
		const FuncInfo a_funcInfo,
		const Patch* a_patch) noexcept
	{
		return AddVMTHook(a_vtbl, a_index, a_funcInfo, std::make_pair(a_patch->Data, a_patch->Size));
	}

	inline auto AddIATHook_Xbyak(
		std::string_view a_moduleName,
		std::string_view a_libraryName,
		std::string_view a_importName,
		const FuncInfo a_funcInfo,
		const Xbyak::CodeGenerator* a_xbyak) noexcept
	{
		return AddIATHook(a_moduleName, a_libraryName, a_importName, a_funcInfo, std::make_pair(a_xbyak->getCode(), a_xbyak->getSize()));
	}

	inline auto AddIATHook_Patch(
		std::string_view a_moduleName,
		std::string_view a_libraryName,
		std::string_view a_importName,
		const FuncInfo a_funcInfo,
		const Patch* a_patch) noexcept
	{
		return AddIATHook(a_moduleName, a_libraryName, a_importName, a_funcInfo, std::make_pair(a_patch->Data, a_patch->Size));
	}
}  // namespace DKUtil::Hook
