/* Copyright (c) 2014, Vsevolod Stakhov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef PKG_JOBS_H_
#define PKG_JOBS_H_

#include <sys/cdefs.h>
#include <sys/sbuf.h>
#include <sys/types.h>

#include <stdbool.h>
#include <uthash.h>
#include <utlist.h>
#include <ucl.h>

#include "private/utils.h"
#include "private/pkg.h"
#include "pkg.h"

struct pkg_jobs;
struct job_pattern;

struct pkg_job_universe_item {
	struct pkg *pkg;
	struct job_pattern *jp;
	int priority;
	UT_hash_handle hh;
	struct pkg_job_universe_item *next, *prev;
};

struct pkg_job_request {
	struct pkg_job_universe_item *item;
	bool skip;
	UT_hash_handle hh;
};

struct pkg_solved {
	struct pkg_job_universe_item *items[2];
	pkg_solved_t type;
	bool already_deleted;
	struct pkg_solved *prev, *next;
};

struct pkg_job_seen {
	struct pkg_job_universe_item *un;
	const char *digest;
	UT_hash_handle hh;
};

struct pkg_job_provide {
	struct pkg_job_universe_item *un;
	const char *provide;
	struct pkg_job_provide *next, *prev;
	UT_hash_handle hh;
};

struct pkg_job_replace {
	char *new_uid;
	char *old_uid;
	struct pkg_job_replace *next;
};


struct pkg_jobs_universe {
	struct pkg_job_universe_item *items;
	struct pkg_job_seen *seen;
	struct pkg_job_provide *provides;
	struct pkg_job_replace *uid_replaces;
	struct pkg_jobs *j;
	size_t nitems;
};

struct pkg_jobs {
	struct pkg_jobs_universe *universe;
	struct pkg_job_request	*request_add;
	struct pkg_job_request	*request_delete;
	struct pkg_solved *jobs;
	struct pkgdb	*db;
	pkg_jobs_t	 type;
	pkg_flags	 flags;
	int		 solved;
	int count;
	int total;
	int conflicts_registered;
	bool need_fetch;
	const char *reponame;
	const char *destdir;
	struct job_pattern *patterns;
};

struct job_pattern {
	char		*pattern;
	char		*path;
	match_t		match;
	bool		is_file;
	UT_hash_handle hh;
};

enum pkg_priority_update_type {
	PKG_PRIORITY_UPDATE_REQUEST = 0,
	PKG_PRIORITY_UPDATE_UNIVERSE,
	PKG_PRIORITY_UPDATE_CONFLICT,
	PKG_PRIORITY_UPDATE_DELETE
};

/*
 * Update priorities for all items related with the specified item
 */
void pkg_jobs_update_universe_priority(struct pkg_jobs_universe *universe,
	struct pkg_job_universe_item *it, enum pkg_priority_update_type type);
/*
 * Update priority as the conflict was found
 */
void pkg_jobs_update_conflict_priority(struct pkg_jobs_universe *universe,
	struct pkg_solved *req);

/*
 * Free universe
 */
void pkg_jobs_universe_free(struct pkg_jobs_universe *universe);

/*
 * Create universe for jobs
 */
struct pkg_jobs_universe * pkg_jobs_universe_new(struct pkg_jobs *j);

/*
 * Add a package to the universe
 */
int pkg_jobs_universe_process_package(struct pkg_jobs_universe *universe,
	struct pkg *pkg);

/*
 * Add a package to the universe and store resulting item in `result`
 */
int pkg_jobs_process_universe(struct pkg_jobs_universe *universe,
	struct pkg *pkg, struct pkg_job_universe_item **result);

/*
 * Add a universe item with package to the request
 */
void pkg_jobs_add_req(struct pkg_jobs *j, const char *uid,
	struct pkg_job_universe_item *item);

/*
 * Check if the specified digest was seen in the universe
 */
bool pkg_jobs_universe_seen(struct pkg_jobs_universe *universe,
	const char *digest);

/*
 * Search for an entry corresponding to the uid in the universe
 */
struct pkg_job_universe_item * pkg_jobs_universe_find(struct pkg_jobs_universe
	*universe, const char *uid);

/*
 * Add a single package to the universe
 */
int pkg_jobs_universe_add_pkg(struct pkg_jobs_universe *universe,
	struct pkg *pkg, bool force, struct pkg_job_universe_item **found);

/*
 * Change uid for universe item
 */
void pkg_jobs_universe_change_uid(struct pkg_jobs_universe *universe,
	struct pkg_job_universe_item *unit,
	const char *new_uid, size_t uidlen, bool update_rdeps);

#endif /* PKG_JOBS_H_ */
