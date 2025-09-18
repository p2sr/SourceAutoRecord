#include "GhostBendyModel.hpp"

// Bendy model data
// 2D (Y,Z) coordinates of vertices.
const float BENDY_VERTS[] = {-13, 16, 13, 16, -13, 40, 13, 40, -12.55, 42.69, 12.55, 42.69, -11.26, 45.2, 11.26, 45.2, -9.19, 47.35, 9.19, 47.35, -6.5, 49.01, 6.5, 49.01, -3.36, 50.04, 3.36, 50.04, 0, 50.4, 11, 61, 10.63, 63.85, 9.53, 66.5, 7.78, 68.78, 5.5, 70.53, 2.85, 71.63, 0, 72, -2.85, 71.63, -5.5, 70.53, -7.78, 68.78, -9.53, 66.5, -10.63, 63.85, -11, 61, -10.63, 58.15, -9.53, 55.5, -7.78, 53.22, -5.5, 51.47, -2.85, 50.37, 0, 50, 2.85, 50.37, 5.5, 51.47, 7.78, 53.22, 9.53, 55.5, 10.63, 58.15, 9, 16, 1, 16, 1, 0, 9, 0, -1, 16, -9, 16, -9, 0, -1, 0, 9, 44, 3, 44, 3, 20, 9, 20, -3, 44, -9, 44, -9, 20, -3, 20, -13, 19, -13, 22, -13, 25, -13, 28, -13, 31, -13, 34, -13, 37, 13, 19, 13, 22, 13, 25, 13, 28, 13, 31, 13, 34, 13, 37};
// Vertex group ID each vertex belongs to.
const short BENDY_GROUPS[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const short BENDY_INDICES[] = {6, 4, 2, 3, 5, 7, 2, 12, 10, 7, 9, 3, 9, 11, 3, 11, 13, 3, 13, 14, 3, 14, 12, 2, 3, 14, 2, 10, 8, 2, 8, 6, 2, 27, 28, 29, 29, 24, 27, 24, 25, 27, 26, 27, 25, 30, 31, 29, 21, 22, 24, 22, 23, 24, 33, 34, 35, 38, 15, 37, 36, 37, 35, 37, 15, 35, 15, 16, 35, 18, 35, 16, 16, 17, 18, 31, 32, 33, 33, 35, 31, 29, 31, 35, 35, 18, 29, 18, 19, 29, 24, 29, 19, 21, 24, 19, 19, 20, 21, 40, 41, 42, 42, 39, 40, 44, 45, 46, 46, 43, 44, 48, 49, 50, 50, 47, 48, 52, 53, 54, 54, 51, 52, 1, 55, 0, 62, 55, 1, 55, 62, 56, 62, 63, 56, 56, 63, 57, 63, 64, 57, 57, 64, 58, 64, 65, 58, 58, 65, 59, 65, 66, 59, 59, 66, 60, 66, 67, 60, 60, 67, 61, 67, 68, 61, 61, 68, 2, 68, 3, 2};
const int BENDY_TRIANGLE_COUNT = sizeof(BENDY_INDICES) / (sizeof(short) * 3);

Vector GhostBendyModel::GetVertexPosition(int index) {
	if (index < 0 || index >= GetVerticesCount()) {
		return Vector{0, 0, 0};
	}
	return Vector{0, BENDY_VERTS[index * 2], BENDY_VERTS[index * 2 + 1]};
}

int GhostBendyModel::GetVerticesCount() {
	return sizeof(BENDY_VERTS) / (sizeof(float) * 2);
}

GhostBendyModel::Group GhostBendyModel::GetVertexGroup(int index) {
	if (index < 0 || index >= GetVerticesCount()) {
		return INVALID_GROUP;
	}
	return static_cast<Group>(BENDY_GROUPS[index]);
}

GhostBendyModel::VertexInfo GhostBendyModel::GetVertexInfo(int index) {
	if (index < 0 || index >= GetVerticesCount()) {
		return VertexInfo{-1, Vector{0, 0, 0}, INVALID_GROUP};
	}
	return VertexInfo{index, GetVertexPosition(index), GetVertexGroup(index)};
}

bool GhostBendyModel::TryGetTriangleIndices(int triangleIndex, short *i1, short *i2, short *i3) {
	if (triangleIndex < 0 || triangleIndex >= BENDY_TRIANGLE_COUNT) {
		return false;
	}
	*i1 = BENDY_INDICES[triangleIndex * 3];
	*i2 = BENDY_INDICES[triangleIndex * 3 + 1];
	*i3 = BENDY_INDICES[triangleIndex * 3 + 2];
	return true;
}
