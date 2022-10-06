// =============================================================================
//
// The Fmidi library - a free software toolkit for MIDI file processing
//
// =============================================================================
//          Copyright Jean Pierre Cimalando 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#if !defined(__cplusplus)
# include <stdbool.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(_WIN32) && defined(FMIDI_BUILD) && !defined(FMIDI_STATIC)
# define FMIDI_API __declspec(dllexport)
#elif defined(_WIN32) && !defined(FMIDI_BUILD) && !defined(FMIDI_STATIC)
# define FMIDI_API __declspec(dllimport)
#elif defined(__GNUC__)
# define FMIDI_API __attribute__((visibility("default")))
#else
# define FMIDI_API
#endif

///////////
// INPUT //
///////////

typedef struct fmidi_smf fmidi_smf_t;

FMIDI_API fmidi_smf_t *fmidi_smf_mem_read(const uint8_t *data, size_t length);
FMIDI_API fmidi_smf_t *fmidi_smf_file_read(const char *filename);
FMIDI_API fmidi_smf_t *fmidi_smf_stream_read(FILE *stream);
FMIDI_API void fmidi_smf_free(fmidi_smf_t *smf);

typedef struct fmidi_smf_info {
    uint16_t format;
    uint16_t track_count;
    uint16_t delta_unit;
} fmidi_smf_info_t;

FMIDI_API const fmidi_smf_info_t *fmidi_smf_get_info(const fmidi_smf_t *smf);
FMIDI_API double fmidi_smf_compute_duration(const fmidi_smf_t *smf);

////////////
// OUTPUT //
////////////

FMIDI_API bool fmidi_smf_mem_write(const fmidi_smf_t *smf, uint8_t **data, size_t *length);
FMIDI_API bool fmidi_smf_file_write(const fmidi_smf_t *smf, const char *filename);
FMIDI_API bool fmidi_smf_stream_write(const fmidi_smf_t *smf, FILE *stream);

////////////////////
// IDENTIFICATION //
////////////////////

typedef enum fmidi_fileformat {
    fmidi_fileformat_smf,
    fmidi_fileformat_xmi,
    fmidi_fileformat_mus,
} fmidi_fileformat_t;

FMIDI_API fmidi_fileformat_t fmidi_mem_identify(const uint8_t *data, size_t length);
FMIDI_API fmidi_fileformat_t fmidi_stream_identify(FILE *stream);

FMIDI_API fmidi_smf_t *fmidi_auto_mem_read(const uint8_t *data, size_t length);
FMIDI_API fmidi_smf_t *fmidi_auto_file_read(const char *filename);
FMIDI_API fmidi_smf_t *fmidi_auto_stream_read(FILE *stream);

////////////
// EVENTS //
////////////

typedef enum fmidi_event_type {
    fmidi_event_meta = 1,
    fmidi_event_message = 2,
    fmidi_event_escape = 3,
    fmidi_event_xmi_timbre = 4,
    fmidi_event_xmi_branch_point = 5
} fmidi_event_type_t;

typedef struct fmidi_event {
    fmidi_event_type_t type;
    uint32_t delta;
    uint32_t datalen;
    uint8_t data[1];
} fmidi_event_t;

#define fmidi_event_sizeof(datalen)             \
    (offsetof(fmidi_event_t, data) + datalen)

////////////
// TRACKS //
////////////

typedef struct fmidi_track_iter {
    uint16_t track;
    uint32_t index;
} fmidi_track_iter_t;

FMIDI_API void fmidi_smf_track_begin(fmidi_track_iter_t *it, uint16_t track);
FMIDI_API const fmidi_event_t *fmidi_smf_track_next(
    const fmidi_smf_t *smf, fmidi_track_iter_t *it);

/////////////
// FORMATS //
/////////////

FMIDI_API fmidi_smf_t *fmidi_xmi_mem_read(const uint8_t *data, size_t length);
FMIDI_API fmidi_smf_t *fmidi_xmi_file_read(const char *filename);
FMIDI_API fmidi_smf_t *fmidi_xmi_stream_read(FILE *stream);

FMIDI_API fmidi_smf_t *fmidi_mus_mem_read(const uint8_t *data, size_t length);
FMIDI_API fmidi_smf_t *fmidi_mus_file_read(const char *filename);
FMIDI_API fmidi_smf_t *fmidi_mus_stream_read(FILE *stream);

///////////////
// SEQUENCER //
///////////////

typedef struct fmidi_seq fmidi_seq_t;

typedef struct fmidi_seq_event {
    double time;
    uint16_t track;
    const fmidi_event_t *event;
} fmidi_seq_event_t;

FMIDI_API fmidi_seq_t *fmidi_seq_new(const fmidi_smf_t *smf);
FMIDI_API void fmidi_seq_free(fmidi_seq_t *pl);
FMIDI_API void fmidi_seq_rewind(fmidi_seq_t *pl);
FMIDI_API bool fmidi_seq_peek_event(fmidi_seq_t *pl, fmidi_seq_event_t *plevt);
FMIDI_API bool fmidi_seq_next_event(fmidi_seq_t *pl, fmidi_seq_event_t *plevt);

/////////////
// UTILITY //
/////////////

typedef struct fmidi_smpte {
    uint8_t code[5];
} fmidi_smpte_t;

FMIDI_API double fmidi_smpte_time(const fmidi_smpte_t *smpte);
FMIDI_API double fmidi_delta_time(double delta, uint16_t unit, uint32_t tempo);
FMIDI_API double fmidi_time_delta(double time, uint16_t unit, uint32_t tempo);

////////////
// ERRORS //
////////////

typedef enum fmidi_status {
    fmidi_ok,
    fmidi_err_format,
    fmidi_err_eof,
    fmidi_err_input,
    fmidi_err_largefile,
    fmidi_err_output
} fmidi_status_t;

FMIDI_API fmidi_status_t fmidi_errno();
FMIDI_API const char *fmidi_strerror(fmidi_status_t status);

typedef struct fmidi_error_info {
    fmidi_status_t code;
#if defined(FMIDI_DEBUG)
    const char *file; int line;
#endif
} fmidi_error_info_t;

FMIDI_API const fmidi_error_info_t *fmidi_errinfo();

////////////
// PLAYER //
////////////

typedef struct fmidi_player fmidi_player_t;
FMIDI_API fmidi_player_t *fmidi_player_new(fmidi_smf_t *smf);
FMIDI_API void fmidi_player_tick(fmidi_player_t *seq, double delta);
FMIDI_API void fmidi_player_free(fmidi_player_t *seq);
FMIDI_API void fmidi_player_start(fmidi_player_t *seq);
FMIDI_API void fmidi_player_stop(fmidi_player_t *seq);
FMIDI_API void fmidi_player_rewind(fmidi_player_t *seq);
FMIDI_API bool fmidi_player_running(const fmidi_player_t *seq);
FMIDI_API double fmidi_player_current_time(const fmidi_player_t *seq);
FMIDI_API void fmidi_player_goto_time(fmidi_player_t *seq, double time);
FMIDI_API double fmidi_player_current_speed(const fmidi_player_t *seq);
FMIDI_API void fmidi_player_set_speed(fmidi_player_t *seq, double speed);
FMIDI_API void fmidi_player_event_callback(
    fmidi_player_t *seq, void (*cbfn)(const fmidi_event_t *, void *), void *cbdata);
FMIDI_API void fmidi_player_finish_callback(
    fmidi_player_t *seq, void (*cbfn)(void *), void *cbdata);

//////////////
// PRINTERS //
//////////////

#if !defined(FMIDI_DISABLE_DESCRIBE_API)
FMIDI_API void fmidi_smf_describe(const fmidi_smf_t *smf, FILE *stream);
FMIDI_API void fmidi_event_describe(const fmidi_event_t *evt, FILE *stream);
#endif

////////////
// LIMITS //
////////////

enum { fmidi_file_size_limit = 64 * 1024 * 1024 };

#if defined(__cplusplus)
}  // extern "C"
#endif

//////////////////
// C++ PRINTERS //
//////////////////

#if defined(__cplusplus) && !defined(FMIDI_DISABLE_DESCRIBE_API)
# include <iosfwd>
FMIDI_API std::ostream &operator<<(std::ostream &out, const fmidi_smf_t &smf);
FMIDI_API std::ostream &operator<<(std::ostream &out, const fmidi_event_t &evt);
#endif

//////////////
// C++ RAII //
//////////////

#if defined(__cplusplus)
# include <memory>

struct fmidi_smf_deleter {
    void operator()(fmidi_smf_t *x) const { fmidi_smf_free(x); } };
struct fmidi_seq_deleter {
    void operator()(fmidi_seq_t *x) const { fmidi_seq_free(x); } };
struct fmidi_player_deleter {
    void operator()(fmidi_player_t *x) const { fmidi_player_free(x); } };

typedef std::unique_ptr<fmidi_smf_t, fmidi_smf_deleter> fmidi_smf_u;
typedef std::unique_ptr<fmidi_seq_t, fmidi_seq_deleter> fmidi_seq_u;
typedef std::unique_ptr<fmidi_player_t, fmidi_player_deleter> fmidi_player_u;
#endif

////////////////
// C++ ERRORS //
////////////////

#if defined(__cplusplus)
# include <system_error>
FMIDI_API const std::error_category &fmidi_category();
#endif
