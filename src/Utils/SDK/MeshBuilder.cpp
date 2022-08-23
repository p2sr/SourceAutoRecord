#include "MeshBuilder.hpp"
#include "Modules/MaterialSystem.hpp"

void MeshBuilder::ComputeBufferSizes(PrimitiveType type, int max_primitives, int *max_verts, int *max_indices) {
	switch (type) {
		case PrimitiveType::POINTS:
			*max_verts = *max_indices = max_primitives;
			break;
		case PrimitiveType::LINES:
			*max_verts = *max_indices = max_primitives * 2;
			break;
		case PrimitiveType::LINE_STRIP:
			*max_verts = max_primitives + 1;
			*max_indices = max_primitives * 2;
			break;
		case PrimitiveType::LINE_LOOP:
			*max_verts = max_primitives;
			*max_indices = max_primitives * 2;
			break;
		case PrimitiveType::TRIANGLES:
			*max_verts = *max_indices = max_primitives * 3;
			break;
		case PrimitiveType::TRIANGLE_STRIP:
			*max_verts = *max_indices = max_primitives + 2;
			break;
		case PrimitiveType::QUADS:
			*max_verts = max_primitives * 4;
			*max_indices = max_primitives * 6;
			break;
		case PrimitiveType::POLYGON:
			*max_verts = max_primitives;
			*max_indices = (max_primitives - 2) * 3;
			break;
		default:
			// impossible if you're a good lil programmer
			*(int *)0 = 0;
			break;
	}
}

void MeshBuilder::GenerateStripIndices(int *num_indices) {
	int first_idx = this->desc.first_vertex;
	int lines = this->num_verts - 1;
	for (int i = 0; i < lines; ++i) {
		this->desc.index_data[i*2 + 0] = first_idx + i;
		this->desc.index_data[i*2 + 1] = first_idx + i + 1;
	}
	*num_indices = (this->num_verts - 1) * 2;
}

void MeshBuilder::GenerateLoopIndices(int *num_indices) {
	int first_idx = this->desc.first_vertex;
	int lines = this->num_verts - 1;
	for (int i = 0; i < lines; ++i) {
		this->desc.index_data[i*2 + 0] = first_idx + i;
		this->desc.index_data[i*2 + 1] = first_idx + i + 1;
	}
	this->desc.index_data[lines * 2 + 0] = first_idx + lines;
	this->desc.index_data[lines * 2 + 1] = first_idx;
	*num_indices = this->num_verts * 2;
}

void MeshBuilder::GeneratePolygonIndices(int *num_indices) {
	int first_idx = this->desc.first_vertex;
	int tris = this->num_verts - 2;
	for (int i = 0; i < tris; ++i) {
		this->desc.index_data[i*3 + 0] = first_idx;
		this->desc.index_data[i*3 + 1] = first_idx + i + 1;
		this->desc.index_data[i*3 + 2] = first_idx + i + 2;
	}
	*num_indices = (this->num_verts - 2) * 3;
}

void MeshBuilder::GenerateQuadIndices(int *num_indices) {
	int first_idx = this->desc.first_vertex;
	int quads = this->num_verts / 4;
	for (int i = 0; i < quads; ++i) {
		int base_idx = first_idx + i*4;
		this->desc.index_data[i*6 + 0] = base_idx;
		this->desc.index_data[i*6 + 1] = base_idx + 1;
		this->desc.index_data[i*6 + 2] = base_idx + 2;
		this->desc.index_data[i*6 + 3] = base_idx;
		this->desc.index_data[i*6 + 4] = base_idx + 2;
		this->desc.index_data[i*6 + 5] = base_idx + 3;
	}
	*num_indices = this->num_verts / 4 * 6;
}

void MeshBuilder::GenerateDirectIndices(int *num_indices) {
	int first_idx = this->desc.first_vertex;
	for (int i = 0; i < this->num_verts; ++i) {
		this->desc.index_data[i] = first_idx + i;
	}
	*num_indices = this->num_verts;
}

MeshBuilder::MeshBuilder(IMaterial *material, PrimitiveType type, int max_primitives) {
	this->render_ctx = materialSystem->GetRenderContext();
	this->render_ctx->vtable->BeginRender(this->render_ctx);

	this->mesh = this->render_ctx->vtable->GetDynamicMesh(this->render_ctx, true, nullptr, nullptr, material);
	this->type = type;
	this->num_verts = 0;

	int max_verts, max_indices;
	this->ComputeBufferSizes(type, max_primitives, &max_verts, &max_indices);

	if (type == PrimitiveType::QUADS || type == PrimitiveType::POLYGON) {
		this->mesh->SetPrimitiveType(PrimitiveType::TRIANGLES);
	} else if (type == PrimitiveType::LINE_STRIP || type == PrimitiveType::LINE_LOOP) {
		this->mesh->SetPrimitiveType(PrimitiveType::LINES);
	} else {
		this->mesh->SetPrimitiveType(type);
	}

	this->mesh->LockMesh(max_verts, max_indices, this->desc);

	this->cur_vertex_data.position = this->desc.vertex_data.position;
	this->cur_vertex_data.color = this->desc.vertex_data.color;
	for (int i = 0; i < 8; ++i) {
		this->cur_vertex_data.tex_coord[i] = this->desc.vertex_data.tex_coord[i];
	}
}

MeshBuilder::~MeshBuilder() {
	if (this->render_ctx) {
		// the blithering fool of a programmer has failed to call End! they must
		// be shunned. nonetheless, let's try to not break the entire game
		this->render_ctx->vtable->OverrideDepthEnable(this->render_ctx, false, false, false);
		this->render_ctx->vtable->EndRender(this->render_ctx);
		this->render_ctx->vtable->Release(this->render_ctx);
		this->render_ctx = nullptr;
	}
}

void MeshBuilder::ForceDepth(bool write, bool test) {
	this->render_ctx->vtable->OverrideDepthEnable(this->render_ctx, true, write, test);
}

void MeshBuilder::Draw() {
	// generate indices
	int num_indices;
	switch (this->type) {
		case PrimitiveType::LINE_STRIP:
			this->GenerateStripIndices(&num_indices);
			break;
		case PrimitiveType::LINE_LOOP:
			this->GenerateLoopIndices(&num_indices);
			break;
		case PrimitiveType::POLYGON:
			this->GeneratePolygonIndices(&num_indices);
			break;
		case PrimitiveType::QUADS:
			this->GenerateQuadIndices(&num_indices);
			break;
		default:
			this->GenerateDirectIndices(&num_indices);
			break;
	}

	// draw mesh
	this->mesh->UnlockMesh(this->num_verts, num_indices, this->desc);
	this->mesh->Draw();

	// cleanup
	this->render_ctx->vtable->OverrideDepthEnable(this->render_ctx, false, false, false);
	this->render_ctx->vtable->EndRender(this->render_ctx);
	this->render_ctx->vtable->Release(this->render_ctx);
	this->render_ctx = nullptr;
}

void MeshBuilder::Position(Vector pos) {
	this->cur_vertex_data.position[0] = pos.x;
	this->cur_vertex_data.position[1] = pos.y;
	this->cur_vertex_data.position[2] = pos.z;
}

void MeshBuilder::Color(::Color col) {
	this->cur_vertex_data.color[0] = col.b;
	this->cur_vertex_data.color[1] = col.g;
	this->cur_vertex_data.color[2] = col.r;
	this->cur_vertex_data.color[3] = col.a;
}

void MeshBuilder::TexCoord(int stage, float s, float t) {
	this->cur_vertex_data.tex_coord[stage][0] = s;
	this->cur_vertex_data.tex_coord[stage][1] = t;
}

void MeshBuilder::AdvanceVertex() {
#define INC_VERT_PTR(type, field) this->cur_vertex_data.field = (type *)((uintptr_t)this->cur_vertex_data.field + this->desc.vertex_size.field)
	INC_VERT_PTR(float, position);
	INC_VERT_PTR(float, normal);
	INC_VERT_PTR(unsigned char, color);
	for (int i = 0; i < 8; ++i) {
		INC_VERT_PTR(float, tex_coord[i]);
	}
#undef INC_VERT_PTR

	++this->num_verts;
}
