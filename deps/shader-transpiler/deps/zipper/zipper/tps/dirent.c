/* /////////////////////////////////////////////////////////////////////////////
 * File:    dirent.c
 *
 * Purpose: Definition of the opendir() API functions for the Win32 platform.
 *
 * Created  19th October 2002
 * Updated: 17th February 2005
 *
 * Home:    http://synesis.com.au/software/
 *
 * Copyright 2002-2005, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the names of Matthew Wilson and Synesis Software nor the names of
 *   any contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////////// */


#ifndef _SYNSOFT_DOCUMENTATION_SKIP_SECTION
# define _SYNSOFT_VER_C_DIRENT_MAJOR      2
# define _SYNSOFT_VER_C_DIRENT_MINOR      0
# define _SYNSOFT_VER_C_DIRENT_REVISION   2
# define _SYNSOFT_VER_C_DIRENT_EDIT       23
#endif /* !_SYNSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////////
 * Includes
 */

#include <windows.h>
#include <errno.h>
#include <stdlib.h>

#include "dirent.h"

/* /////////////////////////////////////////////////////////////////////////////
 * Compiler differences
 */

#if defined(__BORLANDC__)
# define UNIXEM_opendir_PROVIDED_BY_COMPILER
#elif defined(__DMC__)
# define UNIXEM_opendir_PROVIDED_BY_COMPILER
#elif defined(__GNUC__)
# define UNIXEM_opendir_PROVIDED_BY_COMPILER
#elif defined(__INTEL_COMPILER)
#elif defined(_MSC_VER)
#elif defined(__MWERKS__)
#elif defined(__WATCOMC__)
#else
# error Compiler not discriminated
#endif /* compiler */


#if defined(UNIXEM_opendir_PROVIDED_BY_COMPILER) && \
    !defined(UNIXEM_FORCE_ANY_COMPILER)
# error The opendir() API is provided by this compiler, so should not be built here
#endif /* !UNIXEM_opendir_PROVIDED_BY_COMPILER */

/* /////////////////////////////////////////////////////////////////////////////
 * Typedefs
 */

struct dirent_dir
{
    char            directory[_MAX_DIR + 1];    /* . */
    WIN32_FIND_DATA find_data;                  /* The Win32 FindFile data. */
    HANDLE          hFind;                      /* The Win32 FindFile handle. */
    struct dirent   dirent;                     /* The handle's entry. */
};

/* /////////////////////////////////////////////////////////////////////////////
 * Helper functions
 */

static HANDLE findfile_directory(char const *name, LPWIN32_FIND_DATA data)
{
    char    search_spec[_MAX_PATH +1];

    /* Simply add the *.*, ensuring the path separator is
     * included.
     */
    lstrcpyA(search_spec, name);
    if(search_spec[lstrlenA(search_spec) - 1] != '\\')
    {
        lstrcatA(search_spec, "\\*.*");
    }
    else
    {
        lstrcatA(search_spec, "*.*");
    }

    return FindFirstFileA(search_spec, data);
}

/* /////////////////////////////////////////////////////////////////////////////
 * API functions
 */

DIR *opendir(char const *name)
{
    DIR     *result =   NULL;
    DWORD   dwAttr;

    /* Must be a valid name */
    if( !name ||
        !*name ||
        (dwAttr = GetFileAttributesA(name)) == 0xFFFFFFFF)
    {
        errno = ENOENT;
    }
    /* Must be a directory */
    else if(!(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
    {
        errno = ENOTDIR;
    }
    else
    {
        result = (DIR*)malloc(sizeof(DIR));

        if(result == NULL)
        {
            errno = ENOMEM;
        }
        else
        {
            result->hFind = findfile_directory(name, &result->find_data);

            if(result->hFind == INVALID_HANDLE_VALUE)
            {
                free(result);

                result = NULL;
            }
            else
            {
                /* Save the directory, in case of rewind. */
                lstrcpyA(result->directory, name);
                lstrcpyA(result->dirent.d_name, result->find_data.cFileName);
            }
        }
    }

    return result;
}

int closedir(DIR *dir)
{
    int ret;

    if(dir == NULL)
    {
        errno = EBADF;

        ret = -1;
    }
    else
    {
        /* Close the search handle, if not already done. */
        if(dir->hFind != INVALID_HANDLE_VALUE)
        {
            FindClose(dir->hFind);
        }

        free(dir);

        ret = 0;
    }

    return ret;
}

void rewinddir(DIR *dir)
{
    /* Close the search handle, if not already done. */
    if(dir->hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(dir->hFind);
    }

    dir->hFind = findfile_directory(dir->directory, &dir->find_data);

    if(dir->hFind != INVALID_HANDLE_VALUE)
    {
        lstrcpyA(dir->dirent.d_name, dir->find_data.cFileName);
    }
}

struct dirent *readdir(DIR *dir)
{
    /* The last find exhausted the matches, so return NULL. */
    if(dir->hFind == INVALID_HANDLE_VALUE)
    {
        errno = EBADF;

        return NULL;
    }
    else
    {
        /* Copy the result of the last successful match to
         * dirent.
         */
        lstrcpyA(dir->dirent.d_name, dir->find_data.cFileName);

        /* Attempt the next match. */
        if(!FindNextFileA(dir->hFind, &dir->find_data))
        {
            /* Exhausted all matches, so close and null the
             * handle.
             */
            FindClose(dir->hFind);
            dir->hFind = INVALID_HANDLE_VALUE;
        }

        return &dir->dirent;
    }
}

/* ////////////////////////////////////////////////////////////////////////// */
