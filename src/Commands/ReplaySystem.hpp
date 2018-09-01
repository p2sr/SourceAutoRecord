#pragma once
#include "Command.hpp"

#define SAR_TAS_HEADER001 "sar-tas-replay v1.7"
#define SAR_TAS_EXTENSION ".str"

extern Command sar_replay_record;
extern Command sar_replay_record_again;
extern Command sar_replay_play;
extern Command sar_replay_stop;
extern Command sar_replay_export;
extern Command sar_replay_import;
