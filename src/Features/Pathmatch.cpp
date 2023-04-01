// THIS FILE IS INTENTIONALLY NOT INCLUDED IN THE WINDOWS BUILD
// dircache by JJL772 (https://github.com/JJL772/dircache)

#include <atomic>
#include <cstring>
#include <dirent.h>
#include <pthread.h>
#include <string>
#include <time.h>
#include <unordered_map>
#include <vector>
#include "Event.hpp"
#include "Hook.hpp"
#include "SAR.hpp"
#include "Utils/Memory.hpp"

// The amount of time before which added entries are flushed
#define DIRCACHE_STALE_SECONDS 60

////////////////////////////////////////////////////////////////////////////////
// Struct decls
////////////////////////////////////////////////////////////////////////////////

/**
 * dirent_t represents a directory entry on the disk
 * These have a vector of entries, an atomic ref count
 * and an addedat field for the time in which it was added
 * Entries that are considered stale (configured by the user)
 * will be purged from the db and replaced with fresh entries.
 * Stale entries only get purged when their refcount reaches 0
 * Thus, it's important to dirclose
 */
struct dirent_t {
	std::vector<dirent> entries;  // List of entries
	std::atomic_uint32_t nref;    // Ref count from dirdbcontext-s
	double addedat;               // When this entry was added to the db
};

/**
 * dircontext_t just contains a position in the read stream
 * and a pointer to the dirent_t that we're supposed to be
 * reading from.
 * This is the definition of the details behind the
 */
struct dircontext_t {
	std::string key;  // For fast deletions
	size_t pos;
	dirent_t *ent;
};

////////////////////////////////////////////////////////////////////////////////
// Common threading utils and hash helpers
//  RW lock is based on the posix rwmutex, also autolock helpers
////////////////////////////////////////////////////////////////////////////////

struct ReadWriteLock {
	ReadWriteLock() {
		pthread_rwlockattr_init(&attr);
		pthread_rwlock_init(&lock, &attr);
	}
	ReadWriteLock(const ReadWriteLock&) = delete;
	ReadWriteLock(ReadWriteLock&&) = delete;
	
	~ReadWriteLock() {
		pthread_rwlock_destroy(&lock);
		pthread_rwlockattr_destroy(&attr);
	}
	
	void read_lock() {
		pthread_rwlock_rdlock(&lock);
	}
	
	void write_lock() {
		pthread_rwlock_wrlock(&lock);
	}
	
	void unlock() {
		pthread_rwlock_unlock(&lock);
	}
	
	pthread_rwlockattr_t attr;
	pthread_rwlock_t lock;
};

/**
 * Auto lock for reads on a RW mutex
 */
struct AutoReadLock {
	AutoReadLock(ReadWriteLock& lock) : lock_(lock) {
		lock_.read_lock();
	}
	~AutoReadLock() {
		lock_.unlock();
	}
	
	ReadWriteLock& lock_;
};

/**
 * Auto lock for writes on a RW mutex
 */
struct AutoWriteLock {
	AutoWriteLock(ReadWriteLock& lock) : lock_(lock) {
		lock_.write_lock();
	}
	~AutoWriteLock() {
		lock_.unlock();
	}
	
	ReadWriteLock& lock_;
};

////////////////////////////////////////////////////////////////////////////////
// Global db accessors
////////////////////////////////////////////////////////////////////////////////

// Returns the internal directory db
static auto &dir_db() {
	static std::unordered_map<std::string, dirent_t *> dirdb;
	return dirdb;
}

// Returns global db lock
static auto &dir_db_lock() {
	static ReadWriteLock lock;
	return lock;
}

static dircontext_t *dc_build_around_ent(const char *path, dirent_t *dent) {
	auto *ctx = new dircontext_t;
	dent->nref.fetch_add(1);  // Inc ref count
	ctx->key = std::string(path);
	ctx->ent = dent;
	ctx->pos = 0;
	return ctx;
}

////////////////////////////////////////////////////////////////////////////////
// Private helpers
////////////////////////////////////////////////////////////////////////////////

/**
 * Returns time in ms
 */
static double dc_get_time() {
	timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return (tp.tv_sec * 1e3) + (tp.tv_nsec / 1e6);
}

/**
 * Find or populate the dir in the db
 * Calls readdir outright if the dir doesn't exist in the db yet,
 * then stores off those results.
 */
static dircontext_t *dc_find_or_populate(const char *path) {
	auto &db = dir_db();

	{
		AutoReadLock lock(dir_db_lock());
		if (auto ent = db.find((char *)path); ent != db.end()) {
			return dc_build_around_ent(path, ent->second);
		}
	}

	AutoWriteLock lock(dir_db_lock());

	// before we try and cache it, check if it was cached while we were acquiring our
	// write lock (avoid TOCTOU condition)
	if (auto ent = db.find((char *)path); ent != db.end()) {
		return dc_build_around_ent(path, ent->second);
	}

	// read contents and store into the db.
	dirent **namelist = nullptr;
	// Grab all dir entries
	int r = scandir(
		path, &namelist, [](const dirent *d) -> int { return 1; }, [](const dirent **a, const dirent **b) -> int { return strcmp((*a)->d_name, (*b)->d_name); });
	// Bail out on error
	if (r == -1) {
		return nullptr;
	}

	// Build a new directory entry
	auto *dent = new dirent_t();
	dent->addedat = dc_get_time();
	dent->nref.store(0);
	for (int i = 0; i < r; ++i) {
		// RANT MODE ENGAGED
		// The POSIX spec here is AWFUL. For these values, libc is allowed to allocate only as much
		// memory as is needed to store the ACTUAL name. That means if we try and push the value
		// directly into this vector, it copies data from undefined memory, which - in allocator edge
		// cases - may not be mapped! The ONLY GOOD SOLUTION here is to manually copy the data into a
		// buffer, making sure to only copy as much of the name as actually exists, and then push *that*
		// into the buffer. POSIX, from the bottom of my heart, go fuck yourself.
		dirent d;
		memcpy(&d, namelist[i], namelist[i]->d_reclen);
		dent->entries.push_back(d);
		free(namelist[i]);
	}

	free(namelist);

	// Insert into the db
	db.insert({path, dent});

	// Finally build a returnable value
	return dc_build_around_ent(path, dent);
}

#include <stdio.h>

static void dc_close(dircontext_t *context) {
	// If the corresponding entry is stale and unreferenced, evict it
	if (dc_get_time() - context->ent->addedat >= DIRCACHE_STALE_SECONDS * 1000) {
		dir_db_lock().write_lock();  // Lock now so that nothing adds a reference while we delete

		auto old_refs = context->ent->nref.fetch_sub(1);  // Dec refcount
		if (old_refs == 1) {
			delete context->ent;
			dir_db().erase(context->key);
		}

		dir_db_lock().unlock();
	} else {
		context->ent->nref.fetch_sub(1);  // Dec refcount
	}

	delete context;
}

////////////////////////////////////////////////////////////////////////////////
// Public implementation
////////////////////////////////////////////////////////////////////////////////

// Invalidate all entries
void dircache_invalidate() {
	dir_db_lock().write_lock();
	for (auto &p : dir_db()) {
		delete p.second;
	}
	dir_db().clear();
	dir_db_lock().unlock();
}

// readdir(3)
dirent *dircache_readdir(dircontext_t *dir) {
	if (dir->pos >= dir->ent->entries.size())
		return nullptr;
	return &dir->ent->entries[dir->pos++];
}

// opendir(3)
dircontext_t *dircache_opendir(const char *path) {
	return dc_find_or_populate(path);
}

// closedir(3)
void dircache_closedir(dircontext_t *dir) {
	dc_close(dir);
}

enum class PathMod {
	UNCHANGED,
	LOWERED,
	CHANGED,
	FAILED,
};

static PathMod pathmatchDetour(const char *in, char **out, bool allow_basename_mismatch, char *out_buf, size_t out_buf_len) {
	(void)allow_basename_mismatch;

	size_t path_len = strlen(in);
	char *buf = path_len + 1 > out_buf_len ? (char *)malloc(path_len + 1) : out_buf;

	// strcpy, but normalize slashes
	for (size_t i = 0; i < path_len; ++i) {
		buf[i] = in[i] == '\\' ? '/' : in[i];
	}
	buf[path_len] = 0;

	// iterate over dir components
	char *start = buf;
	while (*start) {
		size_t len = 0;
		while (start[len] && start[len] != '/') ++len;

		if (len == 0) {
			// leading or double slash
			++start;
			continue;
		}

		// temporarily remove this component from the string to do an opendir on the parent
		char c = start[0];
		start[0] = 0;
		dircontext_t *dir = dircache_opendir(buf);
		start[0] = c;

		bool matched = false;
		if (dir) {
			struct dirent *ent;
			while (ent = dircache_readdir(dir)) {
				if (!strncasecmp(ent->d_name, start, len) && !ent->d_name[len]) {
					// fill in the correct capitalization
					memcpy(start, ent->d_name, len);
					matched = true;
					break;
				}
			}
			dircache_closedir(dir);
		}

		if (!matched) break; // this component didn't match, so certainly no further ones will

		// next component
		start += len;
		if (*start) start += 1; // skip slash
	}

	*out = buf;
	return PathMod::CHANGED;
}

static Hook _g_pm_hook(&pathmatchDetour);

ON_INIT {
	uintptr_t orig;
	if (sar.game->Is(SourceGame_EIPRelPIC)) {
		orig = Memory::Scan("filesystem_stdio.so", "55 57 56 53 83 EC 0C 8B 6C 24 28 8B 5C 24 2C 0F B6 05 ? ? ? ? 84 C0 0F 84 ? ? ? ?");
	} else {
		orig = Memory::Scan("filesystem_stdio.so", "55 89 E5 57 56 53 83 EC 2C 8B 45 10 80 3D A0 ? ? ? ? 89 45 E4 0F 84 ? ? ? ? 8B 45 0C 8B 15 ? ? ? ? C7 00 00 00 00 00");
	}
	_g_pm_hook.SetFunc(orig);
}
