/* Define to 1 if you want to use the new distribution lists */
#cmakedefine KDEPIM_NEW_DISTRLISTS 1

/* The size of a `long', as computed by sizeof. */
/* Needed by akregator; to be moved to a config-akregator.h */
#define SIZEOF_LONG ${SIZEOF_LONG}

#if defined _WIN32 || defined _WIN64
#define KPATH_SEPARATOR ';'
#else
#define KPATH_SEPARATOR ':'
#endif

