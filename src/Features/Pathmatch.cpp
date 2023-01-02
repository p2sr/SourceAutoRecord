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
#include "Utils/Memory.hpp"

// Uncomment to enable drop-in functionality
#define DIRCACHE_DROPIN

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

template <size_t N>
static void dc_fix_path(const char *path, char (&dest)[N]) {
	strncpy(dest, path, N);
	dest[N - 1] = 0;
	char *c = dest;
	while (*c) {
		c++;
	}
	// Chop off any / at the end of the line
	while (*c && *c == '/') {
		*c = 0;
	}
}

/**
 * Find or populate the dir in the db
 * Calls readdir outright if the dir doesn't exist in the db yet,
 * then stores off those results.
 */
static dircontext_t *dc_find_or_populate(const char *path) {
	{
		AutoReadLock lock(dir_db_lock());
		auto &db = dir_db();
		if (auto ent = db.find((char *)path); ent != db.end()) {
			return dc_build_around_ent(path, ent->second);
		}
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
		dent->entries.push_back(*namelist[i]);
		free(namelist[i]);
	}

	free(namelist);

	// Insert into the db
	dir_db_lock().write_lock();
	dir_db().insert({path, dent});
	dir_db_lock().unlock();

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
	char fixed[PATH_MAX];  // Correct any bad slashes
	dc_fix_path(path, fixed);
	return dc_find_or_populate(fixed);
}

// rewinddir(3)
void dircache_rewinddir(dircontext_t *dir) {
	dir->pos = 0;
}

// telldir(3)
long dircache_telldir(dircontext_t *dir) {
	return dir->pos;
}

// seekdir(3)
void dircache_seekdir(dircontext_t *dir, long loc) {
	if ((unsigned long)loc >= dir->ent->entries.size())
		return;
	dir->pos = loc;
}

// closedir(3)
void dircache_closedir(dircontext_t *dir) {
	dc_close(dir);
}

// scandir(3)
int dircache_scandir(const char *dirp, struct dirent ***namelist, int (*filter)(const struct dirent *), int (*compare)(const struct dirent **, const struct dirent **)) {
	auto ctx = dc_find_or_populate(dirp);
	if (!ctx)
		return -1;

	// Accumulate entries into a list -- This is not quite optimal. Should determine the number of ents first
	*namelist = (dirent **)calloc(ctx->ent->entries.size(), sizeof(dirent *));
	int n = 0;
	for (auto &e : ctx->ent->entries) {
		if (filter && filter(&e))
			continue;
#ifdef DIRCACHE_DROPIN
		auto *p = malloc(sizeof(dirent));
		memcpy(p, &e, sizeof(dirent));
		(*namelist)[n++] = (dirent *)p;
#else
		(*namelist)[n++] = &e;
#endif
	}

	// Resultant list sorted with qsort, as specified by POSIX standard
	if (n && compare)
		qsort(*namelist, n, sizeof(dirent *), (comparison_fn_t)compare);

	dc_close(ctx);
	return n;
}

void dircache_freelist(struct dirent **namelist, int n) {
#ifdef DIRCACHE_DROPIN
	for (int i = 0; i < n; ++i)
		free(namelist[i]);
#endif
	free(namelist);
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
				if (!strncasecmp(ent->d_name, start, len)) {
					// fill in the correct capitalization
					memcpy(start, ent->d_name, len);
					matched = true;
					break;
				}
			}
			dircache_closedir(dir);
		}

		if (!matched) break; // this component didn't match, so certainly no further ones will

		start += len + 1; // next component
	}

	*out = buf;
	return PathMod::CHANGED;
}

static Hook _g_pm_hook(&pathmatchDetour);

ON_INIT {
	auto orig = Memory::Scan("filesystem_stdio.so", "55 57 56 53 83 EC 0C 8B 6C 24 28 8B 5C 24 2C 0F B6 05 ? ? ? ? 84 C0 0F 84 ? ? ? ?");
	_g_pm_hook.SetFunc(orig);
}
