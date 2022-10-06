// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <stdint.h>
#include <stdbool.h>

#if defined SFIZZ_EXPORT_SYMBOLS
  #if defined _WIN32
    #define SFIZZ_EXPORTED_API __declspec(dllexport)
  #else
    #define SFIZZ_EXPORTED_API __attribute__ ((visibility ("default")))
  #endif
#else
  #define SFIZZ_EXPORTED_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup Messaging
 * @{
 */

/**
 * @brief Representation of a binary blob in OSC format
 * @since 1.0.0
 */
typedef struct {
    const uint8_t* data; /**< Pointer to the data */
    uint32_t size; /**< Data size */
} sfizz_blob_t;

/**
 * @brief Representation of an argument of variant type in OSC format as a union
 * @since 1.0.0
 */
typedef union {
    int32_t i; /**< int32_t union value */
    int64_t h; /**< int64_t union value */
    float f; /**< float union value */
    double d; /**< double union value */
    const char* s; /**< char* union value */
    const sfizz_blob_t* b; /**< blob union value */
    uint8_t m[4]; /**< 4-byte midi message union value */
} sfizz_arg_t;

/**
 * @brief Generic message receiving function
 * @since 1.0.0
 */
typedef void (sfizz_receive_t)(void* data, int delay, const char* path, const char* sig, const sfizz_arg_t* args);

/**
 * @brief Convert the message to OSC using the provided output buffer
 * @since 1.0.0
 *
 * @param buffer        The output buffer
 * @param capacity      The capacity of the buffer
 * @param path          The path
 * @param sig           The signature
 * @param args          The arguments
 * @return              The size necessary to store the converted message in
 *                      entirety, <= capacity if the written message is valid.
 */
SFIZZ_EXPORTED_API uint32_t sfizz_prepare_message(
    void* buffer, uint32_t capacity,
    const char* path, const char* sig, const sfizz_arg_t* args);

/**
 * @brief Extract the contents of an OSC message
 * @since 1.0.0
 *
 * @param srcBuffer     The data of the OSC message
 * @param srcCapacity   The size of the OSC message
 * @param argsBuffer    A buffer where the function can allocate the arguments
 * @param argsCapacity  The capacity of the argument buffer
 * @param outPath       A pointer to the variable which receives the path
 * @param outSig        A pointer to the variable which receives the signature
 * @param outArgs       A pointer to the variable which receives the arguments
 * @return              On success, this is the number of bytes read.
 *                      On failure, it is 0 if the OSC message is invalid,
 *                      -1 if there was not enough buffer for the arguments.
 */
SFIZZ_EXPORTED_API int32_t sfizz_extract_message(
    const void* srcBuffer, uint32_t srcCapacity,
    void* argsBuffer, uint32_t argsCapacity,
    const char** outPath, const char** outSig, const sfizz_arg_t** outArgs);

/**
 * @}
 */

#ifdef __cplusplus
} // extern "C"
#endif
