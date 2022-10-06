// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <sfizz.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Loads or imports an instrument file.
 *
 * The file path can be absolute or relative.
 * @since 1.0.1
 *
 * @param synth   The synth.
 * @param path    A null-terminated string representing a path to an instrument
 *                in SFZ format, or another format which can be imported.
 * @param format  An optional pointer to a string pointer, which receives the
 *                null-terminated name of the format if the file was imported,
 *                or null if the file was loaded directly as SFZ.
 *
 * @return @true when file loading went OK,
 *         @false if some error occured while loading.
 *
 * @par Thread-safety constraints
 * - @b CT: the function must be invoked from the Control thread
 * - @b OFF: the function cannot be invoked while a thread is calling @b RT functions
 */
bool sfizz_load_or_import_file(sfizz_synth_t* synth, const char* path, const char** format);

#ifdef __cplusplus
} // extern "C"
#endif
