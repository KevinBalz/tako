#include "FileSystem.hpp"
#include <Windows.h>
#include "WinUtility.hpp"
#include <optional>

std::optional<HANDLE> GetFileHandle(const char* filePath)
{
	HANDLE fileHandle = ::CreateFile(filePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		LOG_ERR("Can't open file {}: {}", filePath, GetLastErrorMessage());
		return {};
	}

	return fileHandle;
}

namespace tako::FileSystem
{
	bool ReadFile(const char* filePath, U8* buffer, size_t bufferSize, size_t& bytesRead)
	{
		auto path = std::string("./Assets") + filePath;
		auto fileHandle = GetFileHandle(path.c_str());
		if (!fileHandle)
		{
			return false;
		}

		DWORD bytes;
		BOOL result = ::ReadFile(*fileHandle, buffer, bufferSize, &bytes, NULL);

		if (result == TRUE)
		{
			bytesRead = bytes;
			CloseHandle(*fileHandle);
			return true;
		}

		LOG_ERR("Error reading file {}: {}", filePath, GetLastErrorMessage());
		CloseHandle(*fileHandle);
		return false;
	}

	size_t GetFileSize(const char* filePath)
	{
		auto path = std::string("./Assets") + filePath;
		auto fileHandle = GetFileHandle(path.c_str());
		ASSERT(fileHandle);

		LARGE_INTEGER size;
		GetFileSizeEx(fileHandle.value(), &size);

		return size.QuadPart;
	}
}

