#pragma once

#include "Variable.hpp"

bool RecordFcps1(void *entity, const Vector &ind_push, int mask);
bool RecordFcps2(const Vector &orig_center, const Vector &extents, const Vector &ind_push, Vector &center_out, FcpsTraceAdapter *adapter);

extern Variable sar_fcps_override;
