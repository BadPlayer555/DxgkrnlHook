#pragma once
#include "import.h"

using dxgk_submit_command_t = int64_t(__fastcall*)(D3DKMT_SUBMITCOMMAND* data);
extern dxgk_submit_command_t original_submit_command;
extern dxgk_submit_command_t* original_entry;

// NATIVE HELPERS
dxgk_submit_command_t* find_submit_command_entry();
// HOOK HANDLER
int64_t __fastcall submit_command_hook(D3DKMT_SUBMITCOMMAND* data);

bool hook_submit_command();
bool unhook_submit_command();
PVOID get_system_routine_address(LPCWSTR routine_name)
{
	UNICODE_STRING name;
	RtlInitUnicodeString(&name, routine_name);
	return MmGetSystemRoutineAddress(&name);
}
PVOID NTAPI RtlxFindExportedRoutineByName(_In_ PVOID DllBase, _In_ const char* ExportName)
{
	PULONG NameTable;
	PUSHORT OrdinalTable;
	PIMAGE_EXPORT_DIRECTORY ExportDirectory;
	LONG Low = 0, Mid = 0, High, Ret;
	USHORT Ordinal;
	PVOID Function;
	ULONG ExportSize;
	PULONG ExportTable;

	ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData(DllBase, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &ExportSize);
	if (!ExportDirectory)
		return NULL;

	NameTable = (PULONG)((ULONG_PTR)DllBase + ExportDirectory->AddressOfNames);
	OrdinalTable = (PUSHORT)((ULONG_PTR)DllBase + ExportDirectory->AddressOfNameOrdinals);
	High = ExportDirectory->NumberOfNames - 1;
	for (Low = 0; Low <= High; Low++)
	{
		Ret = strcmp(ExportName, (PCHAR)DllBase + NameTable[Low]);
		//PRINT("> NameTable %i : %s\n", Low, (PCHAR)DllBase + NameTable[Low]);
		if (Ret == 0) {
			//kprintf("> Found the thing\n");
			break;
		}
	}

	if (High < Low)
		return NULL;

	Ordinal = OrdinalTable[Low];
	if (Ordinal >= ExportDirectory->NumberOfFunctions)
		return NULL;

	ExportTable = (PULONG)((ULONG_PTR)DllBase + ExportDirectory->AddressOfFunctions);
	Function = (PVOID)((ULONG_PTR)DllBase + ExportTable[Ordinal]);
	return Function;
}
PVOID get_system_module_base(LPCWSTR module_name)
{
	//lkd > dt nt!_LDR_DATA_TABLE_ENTRY - l 0xffff8f8a`0f25f110
	//	at 0xffff8f8a`0f25f110
	//	-------------------------------------------- -
	//	+ 0x000 InLoadOrderlinks : _LIST_ENTRY[0xffff8f8a`0cee8c90 - 0xffff8f8a`0f25b010]
	//	+ 0x010 InMemoryOrderlinks : _LIST_ENTRY[0xfffff3ae`f4708000 - 0x00000000`00017034]
	//	+ 0x020 InInitializationOrderlinks : _LIST_ENTRY[0x00000000`00000000 - 0xffff8f8a`0f25f290]
	//	+ 0x030 DllBase          : 0xfffff3ae`f4520000 Void
	//	+ 0x038 EntryPoint       : 0xfffff3ae`f4751010 Void
	//	+ 0x040 SizeOfImage      : 0x26d000
	//	+ 0x048 FullDllName : _UNICODE_STRING "\SystemRoot\System32\win32kbase.sys"
	//	+ 0x058 BaseDllName : _UNICODE_STRING "win32kbase.sys"
	//	+ 0x068 FlagGroup : [4]  ""

	PVOID module_base = NULL;

	__try {

		PLIST_ENTRY module_list = reinterpret_cast<PLIST_ENTRY>(get_system_routine_address(L"PsLoadedModuleList"));

		if (!module_list)
			return NULL;

		UNICODE_STRING name;
		RtlInitUnicodeString(&name, module_name);

		//  InLoadOrderlinks.Flink at 0xffff8f8a`0f25f110
		//	-------------------------------------------- -
		//	+ 0x000 InLoadOrderlinks :  [0xffff8f8a`0cee8c90 - 0xffff8f8a`0f25b010]
		//	+ 0x048 FullDllName : _UNICODE_STRING "\SystemRoot\System32\win32kbase.sys"

		for (PLIST_ENTRY link = module_list; link != module_list->Blink; link = link->Flink)
		{
			LDR_DATA_TABLE_ENTRY* entry = CONTAINING_RECORD(link, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

			// DbgPrint( "driver: %ws\n", entry->FullDllName.Buffer );

			if (RtlEqualUnicodeString(&entry->BaseDllName, &name, TRUE))
			{
				module_base = entry->DllBase;
				break;
			}
		}

	}
	__except (EXCEPTION_EXECUTE_HANDLER) {

		module_base = NULL;
	}

	return module_base;
}
PVOID get_system_module_export(LPCWSTR module_name, LPCSTR routine_name)
{
	PVOID lpModule = get_system_module_base(module_name);
	kprintf("[+] Found %s module_base %p.\n", module_name, lpModule);
	if (!lpModule)
		return NULL;

	return RtlxFindExportedRoutineByName(lpModule, routine_name);
	//return RtlFindExportedRoutineByName(lpModule, routine_name);
}

