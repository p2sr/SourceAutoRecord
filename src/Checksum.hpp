#pragma once

#include <utility>
#include <cstdint>

bool AddDemoChecksum(const char *filename);
void AddDemoFileChecksums();
void AddDemoVpkChecksums();
void RecordRuntimeVscriptChecksum(const char *scriptName, const char *scriptData);
