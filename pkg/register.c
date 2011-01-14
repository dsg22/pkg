#include <sys/param.h>

#include <err.h>
#include <stdio.h>
#include <pkg.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "register.h"

void
usage_register(void)
{
	fprintf(stderr, "register ...\n"
			"register\n");
}

int
exec_register(int argc, char **argv)
{
	struct pkg *pkg;
	struct pkgdb *db;

	char ch;
	char *plist = NULL;
	char *prefix = NULL;
	char *mtree = NULL;
	char *depends = NULL;
	char *conflicts = NULL;
	char *v = NULL;

	int ret = 0;

	pkg_new(&pkg);
	while ((ch = getopt(argc, argv, "vc:d:f:p:P:m:o:O:C:n:")) != -1) {
		switch (ch) {
			case 'O':
			case 'v':
				/* IGNORE */
				break;
			case 'c':
				ret += pkg_setcomment(pkg, optarg[0] == '-' ? optarg + 1 : optarg);
				break;
			case 'd':
				ret += pkg_setdesc_from_file(pkg, optarg);
				break;
			case 'f':
				plist = strdup(optarg);
				break;
			case 'p':
				prefix = strdup(optarg);
				break;
			case 'P':
				depends = strdup(optarg);
				break;
			case 'm':
				mtree = strdup(optarg);
				break;
			case 'n':
				v = strrchr(optarg, '-');
				v[0] = '\0';
				v++;
				ret += pkg_setname(pkg, optarg);
				ret += pkg_setversion(pkg, v);
				break;
			case 'o':
				ret += pkg_setorigin(pkg, optarg);
				break;
			case 'C':
				conflicts = strdup(optarg);
				break;
			default:
				printf("%c\n", ch);
				usage_register();
				return (-1);
		}
	}

	if (ret < 0) {
		pkg_free(pkg);
		return (ret);
	}

	if (depends != NULL) {
		ret += ports_parse_depends(pkg, depends);
		if (ret < 0)
			return (ret);
	}
	struct pkg **deps = pkg_deps(pkg);
	if (deps != NULL) {
		for (int i = 0; deps[i] != NULL; i++) {
			printf("----> %s\n", pkg_name(deps[i]));
		}
	}

	if (conflicts != NULL) {
		ret += ports_parse_conflicts(pkg, conflicts);
		if (ret < 0)
			return (ret);
	}

	ret += ports_parse_plist(pkg, plist, prefix);

	if (ret < 0)
		return (ret);

	if (prefix != NULL)
		free(prefix);

	if (plist != NULL)
		free(plist);

	if (conflicts != NULL)
		free(conflicts);

	if (depends != NULL)
		free(depends);

	if (pkgdb_open(&db) == -1) {
		pkgdb_warn(db);
		return (-1);
	}

	pkgdb_register_pkg(db, pkg);
	pkgdb_close(db);
	pkg_free(pkg);

	return (0);
}
