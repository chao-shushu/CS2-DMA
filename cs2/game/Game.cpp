#include "Game.h"
#include "../utils/Logger.h"

bool CGame::InitAddress()
{
	LOG_INFO("Game", "InitAddress: resolving client.dll...");
	this->Address.ClientDLL = GetProcessModuleHandle(ProcessMgr.HANDLE, ProcessMgr.ProcessID, "client.dll");
	LOG_INFO("Game", "InitAddress: client.dll = 0x{:X}", this->Address.ClientDLL);

	this->Address.MatchDLL = GetProcessModuleHandle(ProcessMgr.HANDLE, ProcessMgr.ProcessID, "matchmaking.dll");
	LOG_DEBUG("Game", "InitAddress: matchmaking.dll = 0x{:X}", this->Address.MatchDLL);

	this->Address.EntityList = GetClientDLLAddress() + Offset::EntityList;
	this->Address.Matrix = GetClientDLLAddress() + Offset::Matrix;
	this->Address.LocalController = GetClientDLLAddress() + Offset::LocalPlayerController;

	this->Address.LocalPawn = GetClientDLLAddress() + Offset::LocalPlayerPawn;
	this->Address.GlobalVars = GetClientDLLAddress() + Offset::GlobalVars;

	LOG_DEBUG("Game", "Addresses: EntityList=0x{:X} Matrix=0x{:X} LocalCtrl=0x{:X} LocalPawn=0x{:X} GlobalVars=0x{:X}",
		this->Address.EntityList, this->Address.Matrix, this->Address.LocalController, this->Address.LocalPawn, this->Address.GlobalVars);

	UpdateEntityListEntry();

	LOG_INFO("Game", "InitAddress: done (ClientDLL=0x{:X})", this->Address.ClientDLL);
	return this->Address.ClientDLL != 0;
}

DWORD64 CGame::GetClientDLLAddress()
{
	return this->Address.ClientDLL;
}

DWORD64 CGame::GetMatchDLLAddress()
{
	return this->Address.MatchDLL;
}

DWORD64 CGame::GetEntityListAddress()
{
	return this->Address.EntityList;
}

DWORD64 CGame::GetMatrixAddress()
{
	return this->Address.Matrix;
}

DWORD64 CGame::GetEntityListEntry()
{
	return this->Address.EntityListEntry;
}

DWORD64 CGame::GetLocalControllerAddress()
{
	return this->Address.LocalController;
}

DWORD64 CGame::GetLocalPawnAddress()
{
	return this->Address.LocalPawn;
}

DWORD64 CGame::GetGlobalVarsAddress()
{
	return this->Address.GlobalVars;
}

bool CGame::UpdateEntityListEntry()
{
	DWORD64 EntityListEntry = 0;
	if (!ProcessMgr.ReadMemory<DWORD64>(gGame.GetEntityListAddress(), EntityListEntry)) {
		LOG_TRACE("Game", "UpdateEntityListEntry: read EntityList FAILED");
		return false;
	}
	if (!ProcessMgr.ReadMemory<DWORD64>(EntityListEntry + 0x10, EntityListEntry)) {
		LOG_TRACE("Game", "UpdateEntityListEntry: read sub-entry FAILED");
		return false;
	}

	this->Address.EntityListEntry = EntityListEntry;
	LOG_TRACE("Game", "UpdateEntityListEntry: 0x{:X}", EntityListEntry);

	return this->Address.EntityListEntry != 0;
}


uint64_t cbSize = 0x80000;

VOID cbAddFile(_Inout_ HANDLE h, _In_ LPSTR uszName, _In_ ULONG64 cb, _In_opt_ PVMMDLL_VFS_FILELIST_EXINFO pExInfo)
{
	if (strcmp(uszName, "dtb.txt") == 0)
		cbSize = cb;
}
DWORD64 GetProcessModuleHandle(VMM_HANDLE HANDLE, DWORD ProcessID, std::string ModuleName)
{
	PVMMDLL_MAP_MODULEENTRY module_entry;

	// Try with refresh + retries first (handles post-init process discovery)
	constexpr int MAX_RETRIES = 15;
	for (int attempt = 0; attempt < MAX_RETRIES; attempt++)
	{
		if (attempt > 0) {
			VMMDLL_ConfigSet(HANDLE, VMMDLL_OPT_REFRESH_ALL, 1);
			Sleep(2000);
		}

		bool result = VMMDLL_Map_GetModuleFromNameU(HANDLE, ProcessID, (LPSTR)ModuleName.c_str(), &module_entry, NULL);
		if (result) {
			LOG_INFO("Memory", "GetProcessModuleHandle: '{}' resolved at 0x{:X} (attempt {})", ModuleName, module_entry->vaBase, attempt + 1);
			return module_entry->vaBase;
		}
		LOG_DEBUG("Memory", "GetProcessModuleHandle: '{}' attempt {} failed", ModuleName, attempt + 1);

	}

	// Fallback: DTB patching via plugin procinfo
	LOG_WARNING("Memory", "GetProcessModuleHandle: '{}' not found after {} retries, trying DTB patch...", ModuleName, MAX_RETRIES);
	VMMDLL_InitializePlugins(HANDLE);
	Sleep(3000);

	VMMDLL_VFS_FILELIST2 VfsFileList;
	VfsFileList.dwVersion = VMMDLL_VFS_FILELIST_VERSION;
	VfsFileList.h = 0;
	VfsFileList.pfnAddDirectory = 0;
	VfsFileList.pfnAddFile = cbAddFile;

	bool result = VMMDLL_VfsListU(HANDLE, (LPSTR)"\\misc\\procinfo\\", &VfsFileList);
	if (!result)
		return false;

	const size_t buffer_size = cbSize;
	std::unique_ptr<BYTE[]> bytes(new BYTE[buffer_size]);
	DWORD j = 0;
	auto nt = VMMDLL_VfsReadW(HANDLE, (LPWSTR)L"\\misc\\procinfo\\dtb.txt", bytes.get(), buffer_size - 1, &j, 0);
	if (nt != VMMDLL_STATUS_SUCCESS)
		return false;

	std::vector<uint64_t> possible_dtbs;
	std::string lines(reinterpret_cast<char*>(bytes.get()));
	std::istringstream iss(lines);
	std::string line;

	while (std::getline(iss, line))
	{
		Info info = { };

		std::istringstream info_ss(line);
		if (info_ss >> std::hex >> info.index >> std::dec >> info.process_id >> std::hex >> info.dtb >> info.kernelAddr >> info.name)
		{
			if (info.process_id == 0) 
				possible_dtbs.push_back(info.dtb);
			if (ModuleName.find(info.name) != std::string::npos)
				possible_dtbs.push_back(info.dtb);
		}
	}

	LOG_DEBUG("Memory", "DTB: {} candidate DTBs to try", possible_dtbs.size());
	for (size_t i = 0; i < possible_dtbs.size(); i++)
	{
		auto dtb = possible_dtbs[i];
		LOG_TRACE("Memory", "DTB: trying 0x{:X} ({}/{})", dtb, i + 1, possible_dtbs.size());
		VMMDLL_ConfigSet(HANDLE, VMMDLL_OPT_PROCESS_DTB, dtb);
		bool found = VMMDLL_Map_GetModuleFromNameU(HANDLE, ProcessID, (LPSTR)ModuleName.c_str(), &module_entry, NULL);
		if (found)
		{
			LOG_INFO("Memory", "DTB: '{}' found with DTB 0x{:X} at 0x{:X}", ModuleName, dtb, module_entry->vaBase);
			return module_entry->vaBase;
		}
	}

	LOG_ERROR("Memory", "Failed to patch module");
	return false;
}
