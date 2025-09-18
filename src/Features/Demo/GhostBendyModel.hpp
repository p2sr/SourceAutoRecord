#pragma once
#include <Utils/SDK/Math.hpp>

namespace GhostBendyModel {
	enum Group {
		INVALID_GROUP = -1,

		BODY = 0,
		HEAD = 1,
		LEFT_LEG = 2,
		RIGHT_LEG = 3,
		LEFT_ARM = 4,
		RIGHT_ARM = 5,
	};

	struct VertexInfo {
		int index;
		Vector position;
		Group group;
	};

	Vector GetVertexPosition(int index);
	int GetVerticesCount();
	Group GetVertexGroup(int index);
	VertexInfo GetVertexInfo(int index);
	bool TryGetTriangleIndices(int triangleIndex, short *i1, short *i2, short *i3);
};
