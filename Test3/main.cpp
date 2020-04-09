#include "main.h"

dxgk_submit_command_t original_submit_command = nullptr;
dxgk_submit_command_t* original_entry = nullptr;
dxgk_submit_command_t* find_submit_command_entry()
{
	PVOID NtGdiDdDDISubmitCommand = get_system_module_export(L"win32kbase.sys", "NtGdiDdDDISubmitCommand");
	kprintf("[+] NtGdiDdDDISubmitCommand: %p\n", NtGdiDdDDISubmitCommand); 
	uint8_t* submit_command_address = reinterpret_cast<uint8_t*>(NtGdiDdDDISubmitCommand);
	kprintf("[+] submit_command_address: %p\n", submit_command_address);

	// FIND MOV INSTRUCTION
	auto instruction = submit_command_address;
	for (;
		instruction[0] != 0x48 ||
		instruction[1] != 0x8B ||
		instruction[2] != 0x05;
		instruction++)
	{
		//:)
	}

	// mov rax,QWORD PTR [rip+0x????????]
	// 48 8B 05 ?? ?? ?? ??
	kprintf("[+] instruction: %p\n", instruction);
	auto delta = *reinterpret_cast<int32_t*>(instruction + 3);
	kprintf("[+] delta: %p\n", delta);
	auto result = reinterpret_cast<dxgk_submit_command_t*>(instruction + delta + 7);
	kprintf("[+] DxgkSubmitCommand: %p\n", result);

	return result;
}

int64_t __fastcall submit_command_hook(D3DKMT_SUBMITCOMMAND * data)
{
	kprintf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n");
	kprintf("[+] sucessfully hooked \n");
	kprintf("[+] Commands %p \n", data->Commands);
	kprintf("[+] CommandLength %i \n", data->CommandLength);
	kprintf("[+] BroadcastContextCount %i \n", data->BroadcastContextCount);
	kprintf("[+] PrivateDriverDataSize %i \n", data->PrivateDriverDataSize);
	kprintf("[+] NumPrimaries %i \n", data->NumPrimaries);
	kprintf("[+] NumHistoryBuffers %i \n", data->NumHistoryBuffers);

	PEPROCESS current_process = IoGetCurrentProcess();
	LPSTR process_name = PsGetProcessImageFileName(current_process);
	kprintf("[+] Process %s \n", process_name);
	kprintf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n");
	return original_submit_command(data);
}
bool hook_submit_command()
{
	// SAVE ORIGINAL ENTRY
	original_entry = find_submit_command_entry();
	kprintf("[+] original_entry: %p\n", original_entry);

	if (original_entry == nullptr)
	{
		kprintf("[-] Failed to find NtGdiDdDDISubmitCommand\n");
		return false;
	}
	original_submit_command = *original_entry;
	kprintf("[+] original_submit_command: %p\n", original_submit_command);
	if (original_submit_command == nullptr)
	{
		kprintf("[-] Failed to find DxgkSubmitCommand\n");
		return false;
	}

	// HOOK
	*original_entry = submit_command_hook;
	kprintf("[+] hooked original_entry: %p\n", original_entry);
	kprintf("[+] Hooked DxgkSubmitCommand!\n");
	return true;
}

bool unhook_submit_command()
{
	// UNHOOK
	*original_entry = original_submit_command;

	kprintf("[+] Unhooked DxgkSubmitCommand!\n");

	return true;
}


extern "C" NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	UNREFERENCED_PARAMETER(DriverObject);
	
	kprintf("[+] Hello from kernel mode! \n");
	hook_submit_command(); 
	return STATUS_SUCCESS;
}