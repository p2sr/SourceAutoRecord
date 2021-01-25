#pragma once

bool AddDemoChecksum(const char* filename);

enum VerifyResult {
    VERIFY_BAD_DEMO,
    VERIFY_NO_CHECKSUM,
    VERIFY_INVALID_CHECKSUM,
    VERIFY_VALID_CHECKSUM,
};

VerifyResult VerifyDemoChecksum(const char* filename);
