/***
*makepath.c - create path name from components
*
*
*Purpose:
*       To provide support for creation of full path names from components
*
*******************************************************************************/

//ASH: Need this on xbox for pre-compiled headers.
#include "stdafx.h"
#include "makepath.h"
/***
*void _makepath() - build path name from components
*
*Purpose:
*       create a path name from its individual components
*
*Entry:
*       char *path  - pointer to buffer for constructed path
*       char *drive - pointer to drive component, may or may not contain
*                     trailing ':'
*       char *dir   - pointer to subdirectory component, may or may not include
*                     leading and/or trailing '/' or '\' characters
*       char *fname - pointer to file base name component
*       char *ext   - pointer to extension component, may or may not contain
*                     a leading '.'.
*
*Exit:
*       path - pointer to constructed path name
*
*Exceptions:
*
*******************************************************************************/
#ifndef WIN32
extern "C" void portable_makepath (
        char *path,
        const char *drive,
        const char *dir,
        const char *fname,
        const char *ext
        )
{
        const char *p;

        /* we assume that the arguments are in the following form (although we
         * do not diagnose invalid arguments or illegal filenames (such as
         * names longer than 8.3 or with illegal characters in them)
         *
         *  drive:
         *      A           ; or
         *      A:
         *  dir:
         *      \top\next\last\     ; or
         *      /top/next/last/     ; or
         *      either of the above forms with either/both the leading
         *      and trailing / or \ removed.  Mixed use of '/' and '\' is
         *      also tolerated
         *  fname:
         *      any valid file name
         *  ext:
         *      any valid extension (none if empty or null )
         */

        /* copy drive */

        if (drive && *drive) {
                *path++ = *drive;
                *path++ = (':');
        }

        /* copy dir */

        if ((p = dir) && *p) {
                do {
                        *path++ = *p++;
                }
                while (*p);
                if (*(p-1) != '/' && *(p-1) != ('\\')) {
                        *path++ = ('\\');
                }
        }

        /* copy fname */

        if (p = fname) {
                while (*p) {
                        *path++ = *p++;
                }
        }

        /* copy ext, including 0-terminator - check to see if a '.' needs
         * to be inserted.
         */

        if (p = ext) {
                if (*p && *p != ('.')) {
                        *path++ = ('.');
                }
                while (*path++ = *p++)
                        ;
        }
        else {
                /* better add the 0-terminator */
                *path = ('\0');
        }
}
#endif