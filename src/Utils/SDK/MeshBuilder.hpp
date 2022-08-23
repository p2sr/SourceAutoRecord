#pragma once

#include <stdint.h>
#include "Math.hpp"
#include "Color.hpp"
#include "Utils/Platform.hpp"

class IMaterial;

enum class VertexCompressionType : unsigned {
	INVALID = 0xFFFFFFFF,
	NONE = 0,
	ON = 1,
};

enum class PrimitiveType {
	POINTS,
	LINES,
	TRIANGLES,
	TRIANGLE_STRIP,
	LINE_STRIP,
	LINE_LOOP,
	POLYGON,
	QUADS,
};

enum class MaterialIndexFormat {
	UNKNOWN = -1,
	FMT_16BIT = 0,
	FMT_32BIT = 1,
};

typedef uint64_t VertexFormat;

struct VertexDesc {
	struct {
		int position;
		int bone_weight;
		int bone_matrix_index;
		int normal;
		int color;
		int specular;
		int tex_coord[8];
		int tangent_s;
		int tangent_t;
		int wrinkle;
		int user_data;
	} vertex_size;

	int actual_vertex_size;
	VertexCompressionType vert_compression_type;
	int num_bone_weights;

	struct {
		float *position;
		float *bone_weight;
		unsigned char *bone_matrix_index;
		float *normal;
		unsigned char *color;
		unsigned char *specular;
		float *tex_coord[8];
		float *tangent_s;
		float *tangent_t;
		float *wrinkle;
		float *user_data;
	} vertex_data;

	int first_vertex;
	int vertex_mem_offset;
};

struct IndexDesc {
	unsigned short *index_data;
	int index_mem_offset;
	int first_index;
	int index_size;
};

struct MeshDesc : public VertexDesc, public IndexDesc { };

struct IVertexBuffer {
	virtual int VertexCount() const = 0;
	virtual VertexFormat GetVertexFormat() const = 0;
	virtual bool IsDynamic() const = 0;
	virtual void BeginCastBuffer(VertexFormat format) = 0;
	virtual void EndCastBuffer() = 0;
	virtual int GetRoomRemaining() const = 0;
	virtual bool Lock(int max_vertex_count, bool append, VertexDesc &desc);
	virtual void Unlock(int vertex_count, VertexDesc &desc);
	virtual void Spew(int vertex_count, const VertexDesc &desc);
	virtual void ValidateData(int vertex_count, const VertexDesc &desc);
};

struct IMesh;
struct IIndexBuffer {
	virtual int IndexCount() const = 0;
	virtual MaterialIndexFormat IndexFormat() const = 0;
	virtual bool IsDynamic() const = 0;
	virtual void BeginCastBuffer(MaterialIndexFormat format) = 0;
	virtual void EndCastBuffer() = 0;
	virtual int GetRoomRemaining() const = 0;
	virtual bool Lock(int max_index_count, bool append, IndexDesc &desc) = 0;
	virtual void Unlock(int index_count, IndexDesc &desc) = 0;
	virtual void ModifyBegin(bool read_only, int first_index, int index_count, IndexDesc &desc) = 0;
	virtual void ModifyEnd(IndexDesc &desc) = 0;
	virtual void Spew(int index_count, const IndexDesc &desc) = 0;
	virtual void ValidateData(int index_count, const IndexDesc &desc) = 0;
	virtual IMesh *GetMesh() = 0;
};

struct IMesh : public IVertexBuffer, public IIndexBuffer {
	virtual void SetPrimitiveType(PrimitiveType type) = 0;
	virtual void Draw(int first_index = -1, int index_count = 0) = 0;
	virtual void SetColorMesh(IMesh *color_mesh, int vertex_offset) = 0;
	virtual void Draw(void *lists, int nlists) = 0;
	virtual void CopyToMeshBuilder() = 0; // DO NOT USE, INCORRECT SIGNATURE
	virtual void Spew(int vertex_count, int index_count, const MeshDesc &desc) = 0;
	virtual void ValidateData(int vertex_count, int index_count, const MeshDesc &desc) = 0;
	virtual void LockMesh(int vertex_count, int index_count, MeshDesc &desc, void *settings = nullptr) = 0;
	virtual void ModifyBegin(int first_vertex, int vertex_count, int first_index, int index_count, MeshDesc &desc);
	virtual void ModifyEnd(MeshDesc &desc) = 0;
	virtual void UnlockMesh(int vertex_count, int index_count, MeshDesc &desc) = 0;
	virtual void ModifyBeginEx(bool read_only, int first_vertex, int vertex_count, int first_index, int index_count, MeshDesc &desc) = 0;
	virtual void SetFlexMesh(IMesh *mesh, int vertex_offset) = 0;
	virtual void DisableFlexMesh() = 0;
	virtual void MarkAsDrawn() = 0;
	virtual void DrawModulated(const void *diffuse_modulation, int first_index = -1, int index_count = 0) = 0;
	virtual unsigned int ComputeMemoryUsed() = 0;
	virtual void *AccessRawHardwareDataStream(uint8_t raw_stream_index, uint32_t num_bytes, uint32_t ui_flags, void *context) = 0;
	virtual void *GetCachedPerFrameMeshData() = 0;
	virtual void ReconstructFromCachedPerFrameMeshData(void *data) = 0;
};

struct IMatRenderContext {
	struct {
		int (__rescall *AddRef)(IMatRenderContext *);
		int (__rescall *Release)(IMatRenderContext *);
		void (__rescall *BeginRender)(IMatRenderContext *);
		void (__rescall *EndRender)(IMatRenderContext *);
		void *_pad1[53];
		IMesh *(__rescall *GetDynamicMesh)(IMatRenderContext *, bool buffered, IMesh *vertex_override, IMesh *index_override, IMaterial *auto_bind);
		void *_pad2[17];
		void (__rescall *OverrideDepthEnable)(IMatRenderContext *, bool enable, bool depth_write_enable, bool depth_test_enable);
	} *vtable;
};

struct MeshBuilder {
private:
	MeshDesc desc;
	IMesh *mesh;
	PrimitiveType type;
	IMatRenderContext *render_ctx;
	int num_verts;

	struct {
		float *position;
		float *normal;
		unsigned char *color;
		float *tex_coord[8];
	} cur_vertex_data;

	void ComputeBufferSizes(PrimitiveType type, int max_primitives, int *max_verts, int *max_indices);

	void GenerateStripIndices(int *num_indices);
	void GenerateLoopIndices(int *num_indices);
	void GeneratePolygonIndices(int *num_indices);
	void GenerateQuadIndices(int *num_indices);
	void GenerateDirectIndices(int *num_indices);

public:
	MeshBuilder(IMaterial *material, PrimitiveType type, int max_primitives);
	~MeshBuilder();

	void ForceDepth(bool write, bool test);

	void Draw();

	// drawing functions
	void Position(Vector pos);
	void Color(Color col);
	void TexCoord(int stage, float s, float t);
	void AdvanceVertex();
};
