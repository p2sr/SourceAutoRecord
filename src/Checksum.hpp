#pragma once

#include <utility>
#include <cstdint>

bool AddDemoChecksum(const char *filename);

enum VerifyResult {
	VERIFY_BAD_DEMO,
	VERIFY_NO_CHECKSUM,
	VERIFY_INVALID_CHECKSUM,
	VERIFY_VALID_CHECKSUM,
};
std::pair<VerifyResult, uint32_t> VerifyDemoChecksum(const char *filename);

void InitSARChecksum();
