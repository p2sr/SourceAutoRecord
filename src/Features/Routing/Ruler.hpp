#pragma once
#include "Command.hpp"
#include "Features/Feature.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

struct Ruler {
	Vector start;
	Vector end;

	float length();
	QAngle angles();
	void draw();
};


class RulerManager {
private:
	std::vector<Ruler> rulers;
	
	int creationStage;
	Vector creatorTracePoint;
	Ruler creatorRuler;
public:
	RulerManager();

	void UpdateCreator();
	void DrawRulers();

	void AddRuler(Vector start, Vector end);
	void RemoveRuler(int id);
	void RemoveAllRulers();

	void ProgressCreationStage();
	bool IsCreating();
};

extern RulerManager rulerManager;
