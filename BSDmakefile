GMAKE?=	gmake
GMAKE_ARGS=	${.MAKEOVERRIDES:@v@${v}=${${v}:Q}@}
GMAKE_RUN=	MAKEFLAGS= MAKELEVEL= ${GMAKE} ${GMAKE_ARGS}

.PHONY: all
all:
	@command -v ${GMAKE} >/dev/null 2>&1 || { echo 'error: ${GMAKE} not found; install gmake' >&2; exit 1; }
	@${GMAKE_RUN}

.DEFAULT:
	@command -v ${GMAKE} >/dev/null 2>&1 || { echo 'error: ${GMAKE} not found; install gmake' >&2; exit 1; }
	@${GMAKE_RUN} ${.TARGET}
