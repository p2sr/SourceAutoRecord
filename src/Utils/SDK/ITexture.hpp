#pragma once

#include <cstdint>

#define TEXTUREFLAGS_CLAMPS 0x4
#define TEXTUREFLAGS_CLAMPT 0x8
#define TEXTUREFLAGS_NOMIP 0x100
#define TEXTUREFLAGS_NOLOD 0x200
#define TEXTUREFLAGS_SINGLECOPY 0x40000

class ITexture;
class IVTFTexture;

struct Rect_t {
	int x, y;
	int width, height;
};

class ITextureRegenerator {
public:
	virtual void RegenerateTextureBits(ITexture *tex, IVTFTexture *vtf_tex, Rect_t *rect) = 0;
	virtual void Release() = 0;
	virtual bool HasPreallocatedScratchTexture() const { return false; }
	virtual IVTFTexture *GetPreallocatedScratchTexture() const { return nullptr; }
};

class ITexture {
public:
	virtual const char *GetName() const = 0;
	virtual int GetMappingWidth() const = 0;
	virtual int GetMappingHeight() const = 0;
	virtual int GetActualWidth() const = 0;
	virtual int GetActualHeight() const = 0;
	virtual int GetNumAnimationFrames() const = 0;
	virtual bool IsTranslucent() const = 0;
	virtual bool IsMipmapped() const = 0;
	virtual void GetLowResColorSample(float s, float t, float *color) const = 0;
	virtual void *GetResourceData(uint32_t data_type, size_t *num_bytes) const = 0;
	virtual void IncrementReferenceCount() = 0;
	virtual void DecrementReferenceCount() = 0;
	virtual void SetTextureRegenerator(ITextureRegenerator *regen, bool release_existing = true) = 0;
	virtual void Download(Rect_t *rect = nullptr, int additional_flags = 0) = 0;
	// NOTE: incomplete
};
