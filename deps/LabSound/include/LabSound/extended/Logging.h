// SPDX-License-Identifier: MIT

/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

// retrieved from https://github.com/rxi/log.c

#ifndef LABSOUND_LOGGING_H
#define LABSOUND_LOGGING_H

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

#define LOG_VERSION "0.1.0"

typedef void (*log_LockFn)(void *udata, int lock);

enum { LOGLEVEL_TRACE = 0, LOGLEVEL_DEBUG, LOGLEVEL_INFO, 
       LOGLEVEL_WARN, LOGLEVEL_ERROR, LOGLEVEL_FATAL };

#define LOG_TRACE(...) LabSoundLog(LOGLEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) LabSoundLog(LOGLEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...)  LabSoundLog(LOGLEVEL_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)  LabSoundLog(LOGLEVEL_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) LabSoundLog(LOGLEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) LabSoundLog(LOGLEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

void log_set_udata(void *udata);
void log_set_lock(log_LockFn fn);
void log_set_fp(FILE *fp);
void log_set_level(int level);
void log_set_quiet(int enable);

void LabSoundLog(int level, const char *file, int line, const char *fmt, ...);
void LabSoundAssertLog(const char * file, int line, const char * function, const char * assertion);

#if 0
#if defined(_DEBUG) || defined(DEBUG) || defined(LABSOUND_ENABLE_LOGGING)
#define LOG(...) LabSoundLog(__FILE__, __LINE__, __VA_ARGS__);
#define LOG_ERROR(...) LabSoundLog(__FILE__, __LINE__, __VA_ARGS__)
#define LOG_VERBOSE(channel, ...) LabSoundLog(__FILE__, __LINE__, __VA_ARGS__)
#else
#define LOG(...)
#define LOG_ERROR(channel, ...)
#define LOG_VERBOSE(channel, ...)
#endif

void LabSoundLog(const char * file, int line, const char * fmt, ...);
void LabSoundAssertLog(const char * file, int line, const char * function, const char * assertion);

#endif

#endif
