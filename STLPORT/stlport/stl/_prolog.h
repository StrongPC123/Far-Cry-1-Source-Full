/* NOTE : this header has no guards and is MEANT for multiple inclusion !
 * If you are using "header protection" option with your compiler,
 * please also find #pragma which disables it and put it here, to
 * allow reentrancy of this header.
 */
/* We undef "std" on entry , as STLport headers may include native ones. */
# undef std

# ifndef _STLP_CONFIG_H
#  include <stl/_config.h>
# endif

/* If the platform provides any specific prolog actions,
 * like #pragmas, do include platform-specific prolog file */
# if defined (_STLP_HAS_SPECIFIC_PROLOG_EPILOG)
#  include <config/_prolog.h>
# endif
