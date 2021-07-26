#pragma once
#include "Command.hpp"
#include "Features/Feature.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

#define SAR_INSPECTION_EXPORT_HEADER                         \
	"Index,Tick,"                                               \
	"m_vecAbsOrigin.x,m_vecAbsOrigin.y,m_vecAbsOrigin.z,"       \
	"m_angAbsRotation.x,m_angAbsRotation.y,m_angAbsRotation.z," \
	"m_vecVelocity.x,m_vecVelocity.y,m_vecVelocity.z,"          \
	"m_fFlags,m_iEFlags,m_flMaxspeed,m_flGravity,"              \
	"m_vecViewOffset.x,m_vecViewOffset.y,m_vecViewOffset.z"

struct InspectionItem {
	int session;
	Vector origin;
	QAngle angles;
	Vector velocity;
	int flags;
	int eFlags;
	float maxSpeed;
	float gravity;
	Vector viewOffset;
};

class EntityInspector : public Feature {
public:
	int entityIndex;

private:
	bool isRunning;
	int lastSession;
	InspectionItem latest;
	std::vector<InspectionItem> data;

public:
	EntityInspector();
	void Start();
	void Record();
	void Stop();
	bool IsRunning();
	InspectionItem GetData();
	void PrintData();
	bool ExportData(std::string filePath);
};

extern EntityInspector *inspector;

extern Variable sar_inspection_save_every_tick;

extern Command sar_inspection_start;
extern Command sar_inspection_stop;
extern Command sar_inspection_print;
extern Command sar_inspection_export;
extern Command sar_inspection_index;
