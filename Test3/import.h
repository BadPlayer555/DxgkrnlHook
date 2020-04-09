#pragma once
#include <ntifs.h>
#include <ntdef.h>
#include <ntddk.h>
#include <ntstatus.h>
#include <ntimage.h>
#include <stdint.h>
#define kprintf(...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, __VA_ARGS__)

#define D3DDDI_MAX_BROADCAST_CONTEXT 64
#define D3DDDI_MAX_WRITTEN_PRIMARIES 16
typedef unsigned int        UINT;
typedef int                 BOOL;
typedef ULONGLONG D3DGPU_VIRTUAL_ADDRESS;
typedef struct _D3DKMT_SUBMITCOMMANDFLAGS
{
	UINT    NullRendering : 1;  // 0x00000001
	UINT    PresentRedirected : 1;  // 0x00000002
	UINT    Reserved : 30;  // 0xFFFFFFFC
} D3DKMT_SUBMITCOMMANDFLAGS;
typedef UINT D3DKMT_HANDLE;
typedef struct _D3DKMT_SUBMITCOMMAND
{
	D3DGPU_VIRTUAL_ADDRESS      Commands;
	UINT                        CommandLength;
	D3DKMT_SUBMITCOMMANDFLAGS   Flags;
	ULONGLONG                   PresentHistoryToken;                            // in: Present history token for redirected present calls
	UINT                        BroadcastContextCount;
	D3DKMT_HANDLE               BroadcastContext[D3DDDI_MAX_BROADCAST_CONTEXT];
	VOID*                       pPrivateDriverData;
	UINT                        PrivateDriverDataSize;
	UINT                        NumPrimaries;
	D3DKMT_HANDLE               WrittenPrimaries[D3DDDI_MAX_WRITTEN_PRIMARIES];
	UINT                        NumHistoryBuffers;
	D3DKMT_HANDLE*              HistoryBufferArray;
} D3DKMT_SUBMITCOMMAND;
typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	// ...
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;
EXTERN_C LPSTR PsGetProcessImageFileName(PEPROCESS Process);

extern "C" DRIVER_INITIALIZE DriverEntry;

void __fastcall SyscallStub(
	_In_ unsigned int SystemCallIndex,
	_Inout_ void** SystemCallFunction);

extern "C" __declspec(dllimport)
PVOID
NTAPI
RtlImageDirectoryEntryToData(
	PVOID ImageBase,
	BOOLEAN MappedAsImage,
	USHORT DirectoryEntry,
	PULONG Size
);
extern "C"
NTSYSAPI
PVOID
NTAPI
RtlFindExportedRoutineByName(
	_In_ PVOID BaseOfImage,
	_In_ PCSTR RoutineName
);