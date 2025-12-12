#include "InstanceIdentifier.hpp"

#include "Event.hpp"

#include <string>
#include <filesystem>
#include <fstream>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif


std::string getInstanceLockFilePath(int index) {
	std::string tempDir = std::filesystem::temp_directory_path().string();

	std::string sarPath = Utils::GetSARPath();
	size_t sarPathHash = std::hash<std::string>{}(sarPath);

	return Utils::ssprintf("%s/sar_instance_%zu_%d.lock", tempDir.c_str(), sarPathHash, index);
}

#ifdef _WIN32

HANDLE g_instanceLockFileHandle = INVALID_HANDLE_VALUE;

bool tryClaimInstanceID(int index) {
	auto handle = CreateFileA(getInstanceLockFilePath(index).c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (handle == INVALID_HANDLE_VALUE) {
		return false;
	}

	OVERLAPPED ol = {0};
	if (!LockFileEx(handle, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, MAXDWORD, MAXDWORD, &ol)) {
		CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
		return false;
	}

	g_instanceLockFileHandle = handle;
	return true;
}

void freeInstanceID() {
	if (g_instanceLockFileHandle == INVALID_HANDLE_VALUE) {
		return;
	}

	UnlockFile(g_instanceLockFileHandle, 0, 0, MAXDWORD, MAXDWORD);
	CloseHandle(g_instanceLockFileHandle);
	g_instanceLockFileHandle = INVALID_HANDLE_VALUE;
}

#else

int g_instanceLockFileHandle = -1;

bool tryClaimInstanceID(int index) {
	auto handle = open(getInstanceLockFilePath(index).c_str(), O_RDWR | O_CREAT, 0666);

	if (handle == -1) {
		return false;
	}

	if (flock(handle, LOCK_EX | LOCK_NB) != 0) {
		if (handle != -1) close(handle);
		g_instanceLockFileHandle = -1;
		return false;
	}

	g_instanceLockFileHandle = handle;
	return true;
}

void freeInstanceID() {
	if (g_instanceLockFileHandle == -1) {
		return;
	}
	flock(g_instanceLockFileHandle, LOCK_UN);
	close(g_instanceLockFileHandle);
	g_instanceLockFileHandle = -1;
}

#endif


int g_instanceID = -1;

void InstanceIdentifier::Claim() {
	const int MAX_INSTANCES = 128;
	for (int i = 0; i < MAX_INSTANCES; i++) {
		if (!tryClaimInstanceID(i)) {
			continue;
		}

		g_instanceID = i;
		return;
	}
}

int InstanceIdentifier::GetID() {
	return g_instanceID;
}

void InstanceIdentifier::Free() {
	freeInstanceID();
	g_instanceID = -1;
}