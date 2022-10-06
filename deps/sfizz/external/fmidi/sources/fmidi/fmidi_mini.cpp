// =============================================================================
//
// The Fmidi library - a free software toolkit for MIDI file processing
// Single-file implementation, based on software revision: 3e6d5b5
//
// =============================================================================
//          Copyright Jean Pierre Cimalando 2018-2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE.md or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
// =============================================================================

#include "fmidi/fmidi.h"
#include <memory>
#include <algorithm>
#include <assert.h>

struct fmidi_player_context {
    fmidi_player_t *plr;
    fmidi_seq_u seq;
    double timepos;
    double speed;
    bool have_event;
    fmidi_seq_event_t sqevt;
    void (*cbfn)(const fmidi_event_t *, void *);
    void *cbdata;
    void (*finifn)(void *);
    void *finidata;
};

struct fmidi_player {
    bool running;
    fmidi_player_context ctx;
};

fmidi_player_t *fmidi_player_new(fmidi_smf_t *smf)
{
    fmidi_player_u plr(new fmidi_player_t);
    plr->running = false;

    fmidi_player_context &ctx = plr->ctx;
    ctx.plr = plr.get();
    ctx.seq.reset(fmidi_seq_new(smf));
    ctx.timepos = 0;
    ctx.speed = 1;
    ctx.have_event = false;
    ctx.cbfn = nullptr;
    ctx.cbdata = nullptr;
    ctx.finifn = nullptr;
    ctx.finidata = nullptr;

    return plr.release();
}

void fmidi_player_tick(fmidi_player_t *plr, double delta)
{
    fmidi_player_context &ctx = plr->ctx;
    fmidi_seq_t &seq = *ctx.seq;
    void (*cbfn)(const fmidi_event_t *, void *) = ctx.cbfn;
    void *cbdata = ctx.cbdata;

    double timepos = ctx.timepos;
    bool have_event = ctx.have_event;
    fmidi_seq_event_t &sqevt = ctx.sqevt;

    timepos += ctx.speed * delta;

    bool more = have_event || fmidi_seq_next_event(&seq, &sqevt);
    if (more) {
        have_event = true;
        while (more && timepos > sqevt.time) {
            const fmidi_event_t &event = *sqevt.event;
            if (cbfn)
                cbfn(&event, cbdata);
            have_event = more = fmidi_seq_next_event(&seq, &sqevt);
        }
    }

    ctx.have_event = have_event;
    ctx.timepos = timepos;

    if (!more) {
        plr->running = false;
        if (ctx.finifn)
            ctx.finifn(ctx.finidata);
    }
}

void fmidi_player_free(fmidi_player_t *plr)
{
    delete plr;
}

void fmidi_player_start(fmidi_player_t *plr)
{
    plr->running = true;
}

void fmidi_player_stop(fmidi_player_t *plr)
{
    plr->running = false;
}

void fmidi_player_rewind(fmidi_player_t *plr)
{
    fmidi_player_context &ctx = plr->ctx;
    fmidi_seq_rewind(ctx.seq.get());
    ctx.timepos = 0;
    ctx.have_event = false;
}

bool fmidi_player_running(const fmidi_player_t *plr)
{
    return plr->running;
}

double fmidi_player_current_time(const fmidi_player_t *plr)
{
    return plr->ctx.timepos;
}

void fmidi_player_goto_time(fmidi_player_t *plr, double time)
{
    fmidi_player_context &ctx = plr->ctx;
    fmidi_seq_t &seq = *ctx.seq;

    uint8_t programs[16];
    uint8_t controls[16 * 128];
    std::fill_n(programs, 16, 0);
    std::fill_n(controls, 16 * 128, 255);

    fmidi_player_rewind(plr);

    for (fmidi_seq_event_t sqevt;
         fmidi_seq_peek_event(&seq, &sqevt) && sqevt.time < time;) {
        const fmidi_event_t &evt = *sqevt.event;
        if (evt.type == fmidi_event_message) {
            uint8_t status = evt.data[0];
            if (status >> 4 == 0b1100 && evt.datalen == 2) {  // program change
                uint8_t channel = status & 0xf;
                programs[channel] = evt.data[1] & 127;
            }
            else if (status >> 4 == 0b1011 && evt.datalen == 3) {  // control change
                uint8_t channel = status & 0xf;
                uint8_t id = evt.data[1] & 127;
                controls[channel * 128 + id] = evt.data[2] & 127;
            }
        }
        fmidi_seq_next_event(&seq, nullptr);
    }

    ctx.timepos = time;

    if (ctx.cbfn) {
        uint8_t evtbuf[fmidi_event_sizeof(3)];
        fmidi_event_t *evt = (fmidi_event_t *)evtbuf;
        evt->type = fmidi_event_message;
        evt->delta = 0;

        for (unsigned c = 0; c < 16; ++c) {
            // all sound off
            evt->datalen = 3;
            evt->data[0] = (0b1011 << 4) | c;
            evt->data[1] = 120;
            evt->data[2] = 0;
            ctx.cbfn(evt, ctx.cbdata);
            // reset all controllers
            evt->datalen = 3;
            evt->data[0] = (0b1011 << 4) | c;
            evt->data[1] = 121;
            evt->data[2] = 0;
            ctx.cbfn(evt, ctx.cbdata);
            // program change
            evt->datalen = 2;
            evt->data[0] = (0b1100 << 4) | c;
            evt->data[1] = programs[c];
            ctx.cbfn(evt, ctx.cbdata);
            // control change
            for (unsigned id = 0; id < 128; ++id) {
                uint8_t val = controls[c * 128 + id];
                if (val < 128) {
                    evt->datalen = 3;
                    evt->data[0] = (0b1011 << 4) | c;
                    evt->data[1] = id;
                    evt->data[2] = val;
                    ctx.cbfn(evt, ctx.cbdata);
                }
            }
        }
    }
}

double fmidi_player_current_speed(const fmidi_player_t *plr)
{
    return plr->ctx.speed;
}

void fmidi_player_set_speed(fmidi_player_t *plr, double speed)
{
    plr->ctx.speed = speed;
}

void fmidi_player_event_callback(
    fmidi_player_t *plr, void (*cbfn)(const fmidi_event_t *, void *), void *cbdata)
{
    fmidi_player_context &ctx = plr->ctx;
    ctx.cbfn = cbfn;
    ctx.cbdata = cbdata;
}

void fmidi_player_finish_callback(
    fmidi_player_t *plr, void (*cbfn)(void *), void *cbdata)
{
    fmidi_player_context &ctx = plr->ctx;
    ctx.finifn = cbfn;
    ctx.finidata = cbdata;
}


#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <tuple>

enum memstream_status {  // make it match fmidi status codes
    ms_ok,
    ms_err_format,
    ms_err_eof,
};

class memstream {
public:
    memstream(const uint8_t *data, size_t length);

    size_t endpos() const;
    size_t getpos() const;
    memstream_status setpos(size_t off);

    memstream_status skip(size_t count);
    memstream_status skipbyte(unsigned byte);

    const uint8_t *peek(size_t length);
    const uint8_t *read(size_t length);

    memstream_status peekbyte(unsigned *retp);
    memstream_status readbyte(unsigned *retp);

    memstream_status readintLE(uint32_t *retp, unsigned length);
    memstream_status readintBE(uint32_t *retp, unsigned length);
    memstream_status readvlq(uint32_t *retp);
    memstream_status peekvlq(uint32_t *retp);

private:
    const uint8_t *base_ = nullptr;
    size_t length_ = 0;
    size_t offset_ = 0;
    typedef std::tuple<memstream_status, uint32_t, unsigned> vlq_result;
    vlq_result doreadvlq();
};

//------------------------------------------------------------------------------
inline memstream::memstream(const uint8_t *data, size_t length)
    : base_(data), length_(length) {
}

inline size_t memstream::endpos() const {
    return length_;
}

inline size_t memstream::getpos() const {
    return offset_;
}

memstream_status memstream::setpos(size_t off)
{
    if (off > length_)
        return ms_err_eof;
    offset_ = off;
    return ms_ok;
}

memstream_status memstream::skip(size_t count)
{
    if (length_ - offset_ < count)
        return ms_err_eof;
    offset_ += count;
    return ms_ok;
}

memstream_status memstream::skipbyte(unsigned byte)
{
    unsigned otherbyte;
    memstream_status status = peekbyte(&otherbyte);
    if (status)
        return status;
    if (byte != otherbyte)
        return ms_err_format;
    ++offset_;
    return ms_ok;
}

const uint8_t *memstream::peek(size_t length)
{
    if (length > length_ - offset_)
        return nullptr;
    return base_ + offset_;
}

const uint8_t *memstream::read(size_t length)
{
    const uint8_t *ptr = peek(length);
    if (ptr)
        offset_ += length;
    return ptr;
}

memstream_status memstream::peekbyte(unsigned *retp)
{
    if (length_ <= offset_)
        return ms_err_eof;
    if (retp)
        *retp = base_[offset_];
    return ms_ok;
}

memstream_status memstream::readbyte(unsigned *retp)
{
    memstream_status ret = peekbyte(retp);
    if (ret)
        return ret;
    ++offset_;
    return ms_ok;
}

memstream_status memstream::readintLE(uint32_t *retp, unsigned length)
{
    const uint8_t *ptr = read(length);
    if (!ptr)
        return ms_err_eof;
    uint32_t ret = 0;
    for (unsigned i = length; i-- > 0;)
        ret = (ret << 8) | ptr[i];
    if (retp)
        *retp = ret;
    return ms_ok;
}

memstream_status memstream::readintBE(uint32_t *retp, unsigned length)
{
    const uint8_t *ptr = read(length);
    if (!ptr)
        return ms_err_eof;
    uint32_t ret = 0;
    for (unsigned i = 0; i < length; ++i)
        ret = (ret << 8) | ptr[i];
    if (retp)
        *retp = ret;
    return ms_ok;
}

memstream_status memstream::readvlq(uint32_t *retp)
{
    memstream_status ret;
    uint32_t value;
    unsigned length;
    std::tie(ret, value, length) = doreadvlq();
    offset_ += length;
    if (retp)
        *retp = value;
    return ret;
}

memstream_status memstream::peekvlq(uint32_t *retp)
{
    memstream_status ret;
    uint32_t value;
    unsigned length;
    std::tie(ret, value, length) = doreadvlq();
    if (retp)
        *retp = value;
    return ret;
}

memstream::vlq_result memstream::doreadvlq()
{
    uint32_t ret = 0;
    unsigned length;
    bool cont = true;
    for (length = 0; cont && length < 4; ++length) {
        if (offset_ + length >= length_)
            return vlq_result{ms_err_eof, 0, 0};
        uint8_t byte = base_[offset_ + length];
        ret = (ret << 7) | (byte & ((1u << 7) - 1));
        cont = byte & (1u << 7);
    }
    if (cont)
        return vlq_result{ms_err_format, 0, 0};
    return vlq_result{ms_ok, ret, length};
}


#include "fmidi/fmidi.h"

#if !defined(FMIDI_DISABLE_DESCRIBE_API)
//------------------------------------------------------------------------------
struct printfmt_quoted {
    printfmt_quoted(const char *text, size_t length)
        : text(text), length(length) {}
    const char *text = nullptr;
    size_t length = 0;
};
std::ostream &operator<<(std::ostream &out, const printfmt_quoted &q);

//------------------------------------------------------------------------------
struct printfmt_bytes {
    printfmt_bytes(const uint8_t *data, size_t size)
        : data(data), size(size) {}
    const uint8_t *data = nullptr;
    size_t size = 0;
};
std::ostream &operator<<(std::ostream &out, const printfmt_bytes &b);
#endif // !defined(FMIDI_DISABLE_DESCRIBE_API)

//------------------------------------------------------------------------------
extern thread_local fmidi_error_info_t fmidi_last_error;

#if defined(FMIDI_DEBUG)
# define RET_FAIL(x, e) do {                                      \
        fmidi_error_info_t &fmidi__err = fmidi_last_error;        \
        fmidi__err.file = __FILE__; fmidi__err.line = __LINE__;   \
        fmidi__err.code = (e); return (x); } while (0)
#else
# define RET_FAIL(x, e)                                         \
    do { fmidi_last_error.code = (e); return (x); } while (0)
#endif

//------------------------------------------------------------------------------
#include <vector>
#include <algorithm>
#include <type_traits>
#include <stdio.h>
#include <assert.h>

class Writer {
public:
    virtual ~Writer() {}
    virtual void put(uint8_t byte) = 0;
    virtual void write(const void *data, size_t size) = 0;
    virtual void rwrite(const void *data, size_t size) = 0;
    virtual void writeLE(const void *data, size_t size) = 0;
    virtual void writeBE(const void *data, size_t size) = 0;
    virtual off_t tell() const = 0;
    virtual bool seek(off_t offset, int whence) = 0;
};

template <class T> class WriterT : public Writer {
public:
    virtual ~WriterT() {}
    void rwrite(const void *data, size_t size) override;
    void writeLE(const void *data, size_t size) override;
    void writeBE(const void *data, size_t size) override;
};

class Memory_Writer : public WriterT<Memory_Writer> {
public:
    explicit Memory_Writer(std::vector<uint8_t> &mem)
        : mem(mem), index(mem.size()) {}
    void put(uint8_t byte) override;
    void write(const void *data, size_t size) override;
    off_t tell() const override;
    bool seek(off_t offset, int whence) override;
private:
    std::vector<uint8_t> &mem;
    size_t index = 0;
};

class Stream_Writer : public WriterT<Stream_Writer> {
public:
    explicit Stream_Writer(FILE *stream)
        : stream(stream) {}
    void put(uint8_t byte) override;
    void write(const void *data, size_t size) override;
    off_t tell() const override;
    bool seek(off_t offset, int whence) override;
private:
    FILE *stream = nullptr;
};

//------------------------------------------------------------------------------
union Endian_check {
    uint32_t value;
    uint8_t head_byte;
};

//------------------------------------------------------------------------------
template <class T>
void WriterT<T>::rwrite(const void *data, size_t size)
{
    const uint8_t *bytes = (const uint8_t *)data;
    for (size_t i = size; i-- > 0;)
        static_cast<T *>(this)->put(bytes[i]);
}

template <class T>
void WriterT<T>::writeLE(const void *data, size_t size)
{
    switch(Endian_check{0x11223344}.head_byte) {
    case 0x11: static_cast<T *>(this)->rwrite(data, size); break;
    case 0x44: static_cast<T *>(this)->write(data, size); break;
    default: assert(false);
    }
}

template <class T>
void WriterT<T>::writeBE(const void *data, size_t size)
{
    switch(Endian_check{0x11223344}.head_byte) {
    case 0x11: static_cast<T *>(this)->write(data, size); break;
    case 0x44: static_cast<T *>(this)->rwrite(data, size); break;
    default: assert(false);
    }
}

inline off_t Memory_Writer::tell() const
{
    return index;
}

inline void Stream_Writer::put(uint8_t byte)
{
    fputc(byte, stream);
}

inline void Stream_Writer::write(const void *data, size_t size)
{
    fwrite(data, size, 1, stream);
}

inline bool Stream_Writer::seek(off_t offset, int whence)
{
    return fseek(stream, offset, whence) == 0;
}

inline off_t Stream_Writer::tell() const
{
    return ftell(stream);
}

thread_local fmidi_error_info_t fmidi_last_error;

fmidi_status_t fmidi_errno()
{
    return fmidi_last_error.code;
}

const fmidi_error_info_t *fmidi_errinfo()
{
    return &fmidi_last_error;
}

const char *fmidi_strerror(fmidi_status_t status)
{
    switch (status) {
    case fmidi_ok: return "success";
    case fmidi_err_format: return "invalid format";
    case fmidi_err_eof: return "premature end of file";
    case fmidi_err_input: return "input error";
    case fmidi_err_largefile: return "file too large";
    case fmidi_err_output: return "output error";
    }
    return nullptr;
}

//------------------------------------------------------------------------------
void Memory_Writer::put(uint8_t byte)
{
    size_t size = mem.size();
    size_t index = this->index;

    if (index < size)
        mem[index] = byte;
    else
    {
        assert(index == size);
        mem.push_back(byte);
    }

    this->index = index + 1;
}

void Memory_Writer::write(const void *data, size_t size)
{
    size_t memsize = mem.size();
    size_t index = this->index;

    const uint8_t *bytes = (const uint8_t *)data;
    size_t ncopy = std::min(size, memsize - index);
    std::copy(bytes, bytes + ncopy, &mem[index]);

    mem.insert(mem.end(), bytes + ncopy, bytes + size);
    this->index = index + size;
}

bool Memory_Writer::seek(off_t offset, int whence)
{
    std::make_unsigned<off_t>::type uoffset(offset);
    size_t size = mem.size();
    size_t index = this->index;

    switch (whence) {
    case SEEK_SET:
        if (uoffset > size)
            return false;
        this->index = uoffset;
        break;
    case SEEK_CUR:
        if (offset >= 0) {
            if (size - index < uoffset)
                return false;
            this->index = index + uoffset;
        }
        else {
            if (index < uoffset)
                return false;
            this->index = index - uoffset;
        }
        break;
    case SEEK_END:
        if (uoffset > size)
            return false;
        this->index = size - uoffset;
        break;
    }
    return true;
}


#include "fmidi/fmidi.h"
#include <vector>

struct fmidi_raw_track {
    std::unique_ptr<uint8_t[]> data;
    uint32_t length;
};

struct fmidi_smf {
    fmidi_smf_info_t info;
    std::unique_ptr<fmidi_raw_track[]> track;
};

//------------------------------------------------------------------------------
uintptr_t fmidi_event_pad(uintptr_t size);
fmidi_event_t *fmidi_event_alloc(std::vector<uint8_t> &buf, uint32_t datalen);
unsigned fmidi_message_sizeof(uint8_t id);

//------------------------------------------------------------------------------
inline uintptr_t fmidi_event_pad(uintptr_t size)
{
    uintptr_t nb = size % alignof(fmidi_event_t);
    return nb ? (size + alignof(fmidi_event_t) - nb) : size;
}
#if !defined(FMIDI_DISABLE_DESCRIBE_API)
#include <fmt/format.h>
#include <fmt/ostream.h>
#endif
#include <string>

double fmidi_smpte_time(const fmidi_smpte *smpte)
{
    const uint8_t *d = smpte->code;
    static const double spftable[4] = { 1.0/24, 1.0/25, 1001.0/30000, 1.0/30 };
    uint8_t hh = d[0];
    double spf = spftable[(hh >> 5) & 0b11];
    hh &= 0b11111;
    uint8_t mm = d[1], ss = d[2], fr = d[3], ff = d[4];
    return (fr + 0.01 * ff) * spf + ss + mm * 60 + hh * 3600;
}

double fmidi_delta_time(double delta, uint16_t unit, uint32_t tempo)
{
    if (unit & (1 << 15)) {
        unsigned tpf = unit & 0xff;  // delta units per frame
        unsigned fps = -(int8_t)(unit >> 8);  // frames per second
        return delta / (tpf * fps);
    }
    else {
        unsigned dpqn = unit;  // delta units per 1/4 note
        double tpqn = 1e-6 * tempo;  // 1/4 note duration
        return delta * tpqn / dpqn;
    }
}

double fmidi_time_delta(double time, uint16_t unit, uint32_t tempo)
{
    if (unit & (1 << 15)) {
        unsigned tpf = unit & 0xff;  // delta units per frame
        unsigned fps = -(int8_t)(unit >> 8);  // frames per second
        return time * (tpf * fps);
    }
    else {
        unsigned dpqn = unit;  // delta units per 1/4 note
        double tpqn = 1e-6 * tempo;  // 1/4 note duration
        return time * dpqn / tpqn;
    }
}

//------------------------------------------------------------------------------
fmidi_event_t *fmidi_event_alloc(std::vector<uint8_t> &buf, uint32_t datalen)
{
    size_t pos = buf.size();
    size_t evsize = fmidi_event_sizeof(datalen);
    size_t padsize = fmidi_event_pad(evsize);
    buf.resize(buf.size() + padsize);
    fmidi_event_t *event = (fmidi_event_t *)&buf[pos];
    return event;
}

unsigned fmidi_message_sizeof(uint8_t id)
{
    if ((id >> 7) == 0) {
        return 0;
    }
    else if ((id >> 4) != 0b1111) {
        static const uint8_t sizetable[8] = {
            3, 3, 3, 3, 2, 2, 3 };
        return sizetable[(id >> 4) & 0b111];
    }
    else {
        static const uint8_t sizetable[16] = {
            0, 2, 3, 2, 1, 1, 1, 0,
            1, 1, 1, 1, 1, 1, 1, 1 };
        return sizetable[id & 0b1111];
    }
}

//------------------------------------------------------------------------------
class fmidi_category_t : public std::error_category {
public:
    const char *name() const noexcept override
        { return "fmidi"; }
    std::string message(int condition) const override
        { return fmidi_strerror((fmidi_status_t)condition); }
};

static fmidi_category_t the_category;

const std::error_category &fmidi_category() {
    return the_category;
};

//------------------------------------------------------------------------------
#if !defined(FMIDI_DISABLE_DESCRIBE_API)
template <class OutputStreamRef>
static bool fmidi_repr_meta(OutputStreamRef out, const uint8_t *data, uint32_t len)
{
    if (len <= 0)
        return false;

    unsigned tag = *data++;
    --len;

    printfmt_quoted qtext{(const char *)data, len};

    switch (tag) {
    default:
        fmt::print(out, "(meta/unknown :tag #x{:02x})", tag);
        return true;
    case 0x00: {  // sequence number
        if (len < 2) return false;
        unsigned number = (data[0] << 8) | data[1];
        fmt::print(out, "(meta/seq-number {})", number);
        return true;
    }
    case 0x01:
        fmt::print(out, "(meta/text {})", qtext);
        return true;
    case 0x02:
        fmt::print(out, "(meta/copyright {})", qtext);
        return true;
    case 0x03:
        fmt::print(out, "(meta/track {})", qtext);
        return true;
    case 0x04:
        fmt::print(out, "(meta/instrument {})", qtext);
        return true;
    case 0x05:
        fmt::print(out, "(meta/lyric {})", qtext);
        return true;
    case 0x06:
        fmt::print(out, "(meta/marker {})", qtext);
        return true;
    case 0x07:
        fmt::print(out, "(meta/cue-point {})", qtext);
        return true;
    case 0x09:
        fmt::print(out, "(meta/device-name {})", qtext);
        return true;
    case 0x20:
        if (len < 1) return false;
        fmt::print(out, "(meta/channel-prefix {})", data[0]);
        return true;
    case 0x21:
        if (len < 1) return false;
        fmt::print(out, "(meta/port {})", data[0]);
        return true;
    case 0x2f:
    case 0x3f:
        fmt::print(out, "(meta/end)");
        return true;
    case 0x51: {
        if (len < 3) return false;
        unsigned t = (data[0] << 16) | (data[1] << 8) | data[2];
        fmt::print(out, "(meta/tempo {} #|{} bpm|#)", t, 60. / (t * 1e-6));
        return true;
    }
    case 0x54: {
        if (len < 5) return false;
        static const char *fpstable[] = {"24", "25", "30000/1001", "30"};
        uint8_t hh = data[0];
        const char *fps = fpstable[(hh >> 5) & 0b11];
        fmt::print(
            out, "(meta/offset {:02d} {:02d} {:02d} {:02d} {:02d}/100 :frames/second {})",
            hh & 0b11111, data[1], data[2], data[3], data[4], fps);
        return true;
    }
    case 0x58:
        if (len < 4) return false;
        fmt::print(out, "(meta/time-sig {} {} {} {})",
                   data[0], data[1], data[2], data[3]);
        return true;
    case 0x59: {
        if (len < 2) return false;
        fmt::print(out, "(meta/key-sig {} :{})",
                   (int8_t)data[0], data[1] ? "minor" : "major");
        return true;
    }
    case 0x7f:
        fmt::print(out, "(meta/sequencer-specific {})", printfmt_bytes{data, len});
        return true;
    }

    return false;
}

template <class OutputStreamRef>
static bool fmidi_repr_midi(OutputStreamRef out, const uint8_t *data, uint32_t len)
{
    if (len <= 0)
        return false;

    unsigned status = *data++;
    --len;

    auto b7 = [data](unsigned i)
        { return data[i] & 0x7f; };
    auto b14 = [data](unsigned i)
        { return (data[i] & 0x7f) | (data[i + 1] & 0x7f) << 7; };

    if (status >> 4 == 0xf) {
        unsigned op = status & 0xf;

        switch (op) {
        case 0b0000:
            fmt::print(out, "(sysex #xf0 {})", printfmt_bytes{data, len});
            return true;
        case 0b0001: {
            if (len < 1) return false;
            unsigned tc = b7(0);
            fmt::print(out, "(time-code {} {})", tc >> 4, tc & 0b1111);
            return true;
        }
        case 0b0010:
            if (len < 2) return false;
            fmt::print(out, "(song-position {})", b14(0));
            return true;
        case 0b0011:
            if (len < 1) return {};
            fmt::print(out, "(song-select {})", b7(0));
            return true;
        case 0b0110:
            fmt::print(out, "(tune-request)");
            return true;
        case 0b1000:
            fmt::print(out, "(timing-clock)");
            return true;
        case 0b1010:
            fmt::print(out, "(start)");
            return true;
        case 0b1011:
            fmt::print(out, "(continue)");
            return true;
        case 0b1100:
            fmt::print(out, "(stop)");
            return true;
        case 0b1110:
            fmt::print(out, "(active-sensing)");
            return true;
        case 0b1111:
            fmt::print(out, "(reset)");
            return true;
        }
    }
    else {
        unsigned op = status >> 4;
        unsigned ch = status & 0xf;

        switch (op) {
        case 0b1000:
            if (len < 2) return false;
            fmt::print(out, "(note-off {} :velocity {} :channel {})", b7(0), b7(1), ch);
            return true;
        case 0b1001:
            if (len < 2) return false;
            fmt::print(out, "(note-on {} :velocity {} :channel {})", b7(0), b7(1), ch);
            return true;
        case 0b1010:
            if (len < 2) return false;
            fmt::print(out, "(poly-aftertouch {} :pressure {} :channel {})", b7(0), b7(1), ch);
            return true;
        case 0b1011:
            if (len < 2) return false;
            fmt::print(out, "(control #x{:02x} {} :channel {})", b7(0), b7(1), ch);
            return true;
        case 0b1100:
            if (len < 1) return false;
            fmt::print(out, "(program {} :channel {})", b7(0), ch);
            return true;
        case 0b1101:
            if (len < 1) return false;
            fmt::print(out, "(aftertouch :pressure {} :channel {})", b7(0), ch);
            return true;
        case 0b1110:
            if (len < 2) return false;
            fmt::print(out, "(pitch-bend {} :channel {})", b14(0), ch);
            return true;
        }
    }

    return false;
}

static bool fmidi_identify_sysex(const uint8_t *msg, size_t len, std::string &text)
{
    if (len < 4 || msg[0] != 0xf0 || msg[len - 1] != 0xf7)
        return false;

    unsigned manufacturer = msg[1];
    unsigned deviceid = msg[2];

    switch (manufacturer) {
        case 0x7e:  // universal non-realtime
            if (len >= 6) {
                switch ((msg[3] << 8) | msg[4]) {
                case 0x0901: text = "GM system on"; return true;
                case 0x0902: text = "GM system off"; return true;
                }
            }
            break;
        case 0x7f:  // universal realtime
            if (len >= 6) {
                switch ((msg[3] << 8) | msg[4]) {
                case 0x0401: text = "GM master volume"; return true;
                case 0x0402: text = "GM master balance"; return true;
                }
            }
            break;
        case 0x41:  // Roland
            if (len >= 9) {
                unsigned model = msg[3];
                unsigned mode = msg[4];
                unsigned address = (msg[5] << 16) | (msg[6] << 8) | msg[7];
                if (mode == 0x12) {  // send
                    switch ((model << 24) | address) {
                    case (0x42u << 24) | 0x00007fu: text = "GS system mode set"; return true;
                    case (0x42u << 24) | 0x40007fu: text = "GS mode set"; return true;
                    default: text = fmt::format("GS parameter #x{:06x}", address); return true;
                    }
                }
            }
            break;
        case 0x43:  // Yamaha
            if (len >= 5) {
                unsigned model = msg[3];
                switch((model << 8) | (deviceid & 0xf0))
                {
                case (0x4c << 8) | 0x10:  // XG
                    if (len >= 8) {
                        unsigned address = (msg[4] << 16) | (msg[5] << 8) | msg[6];
                        switch (address) {
                        case 0x00007e: text = "XG system on"; return true;
                        default: text = fmt::format("XG parameter #x{:06x}", address); return true;
                        }
                        break;
                    }
                }
            }
            break;
    }

    return false;
}

template <class OutputStreamRef>
static void fmidi_repr_smf(OutputStreamRef out, const fmidi_smf_t &smf)
{
    const fmidi_smf_info_t *info = fmidi_smf_get_info(&smf);
    fmt::print(out, "(midi-file");
    fmt::print(out, "\n  :format {}", info->format);

    unsigned unit = info->delta_unit;
    if (unit & (1 << 15))
        fmt::print(out, "\n  :delta-unit (smpte-based :units/frame {} :frames/second {})",
                   unit & 0xff, -(int8_t)(unit >> 8));
    else
        fmt::print(out, "\n  :delta-unit (tempo-based :units/beat {})", unit);

    fmt::print(out, "\n  :tracks"
               "\n  (", unit);

    struct RPN_Info {
        unsigned lsb = 127, msb = 127;
        bool nrpn = false;
    };
    RPN_Info channel_rpn[16];

    std::string strbuf;
    strbuf.reserve(256);

    for (unsigned i = 0, n = info->track_count; i < n; ++i) {
        fmidi_track_iter_t it;
        fmidi_smf_track_begin(&it, i);
        if (i > 0)
            fmt::print(out, "\n   ");
        fmt::print(out, "(;;--- track {} ---;;", i);
        while (const fmidi_event_t *evt = fmidi_smf_track_next(&smf, &it)) {
            RPN_Info *rpn = nullptr;

            const uint8_t *data = evt->data;
            uint32_t datalen = evt->datalen;

            if (evt->type == fmidi_event_message) {
                unsigned status = data[0];
                unsigned channel = status & 0x0f;
                // controllers
                if (datalen == 3 && (status & 0xf0) == 0xb0) {
                    unsigned ctl = data[1] & 0x7f;
                    switch (ctl) {
                    case 0x62: case 0x64:  // (N)RPN LSB
                        rpn = &channel_rpn[channel];
                        rpn->lsb = data[2] & 0x7f, rpn->nrpn = ctl == 0x62;
                        break;
                    case 0x63: case 0x65:  // (N)RPN MSB
                        rpn = &channel_rpn[channel];
                        rpn->msb = data[2] & 0x7f, rpn->nrpn = ctl == 0x63;
                        break;
                    case 0x06: case 0x26:  // Data Entry MSB, LSB
                        rpn = &channel_rpn[channel];
                        break;
                    }
                }
            }

            fmt::print(out, "\n    (:delta {:<5} {}", evt->delta, *evt);
            if (rpn)
                fmt::print(out, " #|{}RPN #x{:02x} #x{:02x}|#",
                           rpn->nrpn ? "N" : "", rpn->msb, rpn->lsb);
            else if (fmidi_identify_sysex(data, datalen, strbuf))
                fmt::print(out, " #|{}|#", strbuf);
            fmt::print(out, ")");
        }
        fmt::print(out, ")");
    }
    fmt::print(out, "))\n");
}

std::ostream &operator<<(std::ostream &out, const fmidi_smf_t &smf)
{
    fmidi_repr_smf<std::ostream &>(out, smf);
    return out;
}

void fmidi_smf_describe(const fmidi_smf_t *smf, FILE *stream)
{
    fmidi_repr_smf<FILE *>(stream, *smf);
}

template <class OutputStreamRef>
static void fmidi_repr_event(OutputStreamRef out, const fmidi_event_t &evt)
{
    const uint8_t *data = evt.data;
    uint32_t len = evt.datalen;

    switch (evt.type) {
    case fmidi_event_meta: {
        if (!fmidi_repr_meta<OutputStreamRef>(out, data, len))
            fmt::print(out, "(meta/unknown)");
        break;
    }
    case fmidi_event_message: {
        if (!fmidi_repr_midi<OutputStreamRef>(out, data, len))
            fmt::print(out, "(unknown)");
        break;
    }
    case fmidi_event_escape: {
        fmt::print(out, "(raw {})", printfmt_bytes{data, len});
        break;
    }
    case fmidi_event_xmi_timbre: {
        fmt::print(out, "(xmi/timbre :patch {} :bank {})", evt.data[0], evt.data[1]);
        break;
    }
    case fmidi_event_xmi_branch_point: {
        fmt::print(out, "(xmi/branch-point {})", evt.data[0]);
        break;
    }
    }
}

std::ostream &operator<<(std::ostream &out, const fmidi_event_t &evt)
{
    fmidi_repr_event<std::ostream &>(out, evt);
    return out;
}

void fmidi_event_describe(const fmidi_event_t *evt, FILE *stream)
{
    fmidi_repr_event<FILE *>(stream, *evt);
}

//------------------------------------------------------------------------------
std::ostream &operator<<(std::ostream &out, const printfmt_quoted &q)
{
    const char *text = q.text;
    size_t length = q.length;
    out.put('"');
    for (size_t i = 0; i < length; ++i) {
        char c = text[i];
        if (c == '\\' || c == '"') out.put('\\');
        out.put(c);
    }
    return out.put('"');
}

std::ostream &operator<<(std::ostream &out, const printfmt_bytes &b)
{
    const uint8_t *data = b.data;
    for (size_t i = 0, n = b.size; i < n; ++i) {
        if (i > 0) out.put(' ');
        fmt::print(out, "#x{:02x}", data[i]);
    }
    return out;
}
#endif // !defined(FMIDI_DISABLE_DESCRIBE_API)


#include <memory>
#include <stdio.h>

////////////////////////
// FILE PATH ENCODING //
////////////////////////

FILE *fmidi_fopen(const char *path, const char *mode);

///////////////
// FILE RAII //
///////////////

struct FILE_deleter;
typedef std::unique_ptr<FILE, FILE_deleter> unique_FILE;

struct FILE_deleter {
    void operator()(FILE *stream) const
        { fclose(stream); }
};
#if defined(_WIN32)
# include <windows.h>
# include <memory>
# include <errno.h>
#endif

FILE *fmidi_fopen(const char *path, const char *mode)
{
#if !defined(_WIN32)
    return fopen(path, mode);
#else
    auto toWideString = [](const char *utf8) -> wchar_t * {
        unsigned wsize = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
        if (wsize == 0)
            return nullptr;
        wchar_t *wide = new wchar_t[wsize];
        wsize = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wide, wsize);
        if (wsize == 0) {
            delete[] wide;
            return nullptr;
        }
        return wide;
    };
    std::unique_ptr<wchar_t[]> wpath(toWideString(path));
    if (!wpath) {
        errno = EINVAL;
        return nullptr;
    }
    std::unique_ptr<wchar_t[]> wmode(toWideString(mode));
    if (!wmode) {
        errno = EINVAL;
        return nullptr;
    }
    return _wfopen(wpath.get(), wmode.get());
#endif
}

#include "fmidi/fmidi.h"
#include <memory>
#include <string.h>

struct fmidi_seq_timing {
    fmidi_smpte startoffset;
    uint32_t tempo;
};

struct fmidi_seq_pending_event {
    const fmidi_event_t *event;
    double delta;
};

struct fmidi_seq_track_info {
    double timepos;
    fmidi_track_iter_t iter;
    fmidi_seq_pending_event next;
    std::shared_ptr<fmidi_seq_timing> timing;
};

struct fmidi_seq {
    const fmidi_smf_t *smf;
    std::unique_ptr<fmidi_seq_track_info[]> track;
};

static double fmidi_convert_delta(
    const fmidi_seq_t *seq, uint16_t trkno, double delta)
{
    uint16_t unit = fmidi_smf_get_info(seq->smf)->delta_unit;
    uint32_t tempo = seq->track[trkno].timing->tempo;
    return fmidi_delta_time(delta, unit, tempo);
}

fmidi_seq_t *fmidi_seq_new(const fmidi_smf_t *smf)
{
    std::unique_ptr<fmidi_seq_t> seq(new fmidi_seq_t);
    seq->smf = smf;

    const fmidi_smf_info_t *info = fmidi_smf_get_info(smf);
    uint16_t format = info->format;
    uint16_t ntracks = info->track_count;

    seq->track.reset(new fmidi_seq_track_info[ntracks]);

    for (unsigned i = 0; i < ntracks; ++i) {
        fmidi_seq_track_info &track = seq->track[i];

        std::shared_ptr<fmidi_seq_timing> timing;
        if (format == 2 || i == 0)
            timing.reset(new fmidi_seq_timing);
        else
            timing = seq->track[0].timing;
        track.timing = timing;
    }

    fmidi_seq_rewind(seq.get());
    return seq.release();
}

void fmidi_seq_free(fmidi_seq_t *seq)
{
    delete seq;
}

void fmidi_seq_rewind(fmidi_seq_t *seq)
{
    const fmidi_smf_t *smf = seq->smf;
    const fmidi_smf_info_t *info = fmidi_smf_get_info(smf);
    uint16_t ntracks = info->track_count;
    bool independent_multi_track =
        ntracks > 1 && seq->track[0].timing != seq->track[1].timing;

    for (unsigned i = 0; i < ntracks; ++i) {
        fmidi_seq_track_info &track = seq->track[i];
        std::shared_ptr<fmidi_seq_timing> timing = track.timing;
        fmidi_smpte &startoffset = timing->startoffset;
        fmidi_smf_track_begin(&track.iter, i);
        track.next.event = nullptr;
        memset(startoffset.code, 0, 5);
        timing->tempo = 500000;
        track.timepos = fmidi_smpte_time(&startoffset);
    }

    for (unsigned i = 0; i < ntracks; ++i) {
        fmidi_seq_track_info &track = seq->track[i];
        std::shared_ptr<fmidi_seq_timing> timing = track.timing;
        fmidi_smpte &startoffset = timing->startoffset;

        const fmidi_event_t *evt;
        fmidi_track_iter_t it;
        fmidi_smf_track_begin(&it, i);
        while ((evt = fmidi_smf_track_next(smf, &it)) &&
               evt->delta == 0 && evt->type == fmidi_event_meta) {
            uint8_t id = evt->data[0];
            if (id == 0x54 && evt->datalen == 6) {  // SMPTE offset
                // disregard SMPTE offset for format 1 MIDI and similar
                if (independent_multi_track)
                    memcpy(startoffset.code, &evt->data[1], 5);
            }
            if (id == 0x51 && evt->datalen == 4) {  // set tempo
                const uint8_t *d24 = &evt->data[1];
                timing->tempo = (d24[0] << 16) | (d24[1] << 8) | d24[2];
            }
        }
        track.timepos = fmidi_smpte_time(&startoffset);
    }
}

static fmidi_seq_pending_event *fmidi_seq_track_current_event(
    fmidi_seq_t *seq, uint16_t trkno)
{
    const fmidi_smf_t *smf = seq->smf;
    fmidi_seq_track_info &track = seq->track[trkno];

    fmidi_seq_pending_event *pending;
    if (track.next.event)
        return &track.next;

    const fmidi_event_t *evt = fmidi_smf_track_next(smf, &track.iter);
    if (!evt)
        return nullptr;

    if (evt->type == fmidi_event_meta) {
        uint8_t tag = evt->data[0];
        if (tag == 0x2f || tag == 0x3f)  // end of track
            return nullptr;  // stop now even if the final event has delta
    }

    pending = &track.next;
    pending->event = evt;
    pending->delta = evt->delta;
    return pending;
}

static int fmidi_seq_next_track(fmidi_seq_t *seq)
{
    const fmidi_smf_info_t *info = fmidi_smf_get_info(seq->smf);
    unsigned ntracks = info->track_count;

    unsigned trkno = 0;
    fmidi_seq_pending_event *pevt;

    pevt = fmidi_seq_track_current_event(seq, 0);
    while (!pevt && ++trkno < ntracks)
        pevt = fmidi_seq_track_current_event(seq, trkno);

    if (!pevt)
        return -1;

    double nearest = fmidi_convert_delta(seq, trkno, pevt->delta) +
        seq->track[trkno].timepos;
    for (unsigned i = trkno + 1; i < ntracks; ++i) {
        if ((pevt = fmidi_seq_track_current_event(seq, i))) {
            double time = fmidi_convert_delta(seq, i, pevt->delta) +
                seq->track[i].timepos;
            if (time < nearest) {
                trkno = i;
                nearest = time;
            }
        }
    }

    return trkno;
}

bool fmidi_seq_peek_event(fmidi_seq_t *seq, fmidi_seq_event_t *sqevt)
{
    unsigned trkno = fmidi_seq_next_track(seq);
    if ((int)trkno == -1)
        return false;

    fmidi_seq_track_info &nexttrk = seq->track[trkno];

    const fmidi_seq_pending_event *pevt =
        fmidi_seq_track_current_event(seq, trkno);
    if (!pevt)
        return false;

    if (sqevt) {
        sqevt->time = fmidi_convert_delta(seq, trkno, pevt->delta) + nexttrk.timepos;
        sqevt->track = trkno;
        sqevt->event = pevt->event;
    }

    return true;
}

static void fmidi_seq_track_advance_by(
    fmidi_seq_t *seq, unsigned trkno, double time)
{
    const fmidi_smf_t *smf = seq->smf;
    const fmidi_smf_info_t *info = fmidi_smf_get_info(smf);
    uint16_t unit = info->delta_unit;
    fmidi_seq_track_info &trk = seq->track[trkno];
    fmidi_seq_timing &tim = *trk.timing;

    fmidi_seq_pending_event *evt = fmidi_seq_track_current_event(seq, trkno);
    if (evt)
        evt->delta -= fmidi_time_delta(time, unit, tim.tempo);
    trk.timepos += time;
}

bool fmidi_seq_next_event(fmidi_seq_t *seq, fmidi_seq_event_t *sqevt)
{
    fmidi_seq_event_t pltmp;
    sqevt = sqevt ? sqevt : &pltmp;

    if (!fmidi_seq_peek_event(seq, sqevt))
        return false;

    double time = sqevt->time;
    unsigned trkno = sqevt->track;
    const fmidi_event_t *evt = sqevt->event;
    fmidi_seq_track_info &trk = seq->track[trkno];

    const fmidi_smf_t *smf = seq->smf;
    const fmidi_smf_info_t *info = fmidi_smf_get_info(smf);
    unsigned ntracks = info->track_count;
    double elapsed = time - trk.timepos;

    for (unsigned i = 0; i < ntracks; ++i)
        if (i != trkno)
            fmidi_seq_track_advance_by(seq, i, elapsed);

    if (evt->type == fmidi_event_meta) {
        if (evt->data[0] == 0x51 && evt->datalen == 4) {  // set tempo
            const uint8_t *d24 = &evt->data[1];
            trk.timing->tempo = (d24[0] << 16) | (d24[1] << 8) | d24[2];
        }
    }

    trk.timepos = time;
    trk.next.event = nullptr;
    return true;
}

#include "fmidi/fmidi.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <string.h>
#include <sys/stat.h>
#if defined(_WIN32)
# define fileno _fileno
#endif

const fmidi_smf_info_t *fmidi_smf_get_info(const fmidi_smf_t *smf)
{
    return &smf->info;
}

double fmidi_smf_compute_duration(const fmidi_smf_t *smf)
{
    double duration = 0;
    fmidi_seq_u seq(fmidi_seq_new(smf));
    fmidi_seq_event_t sqevt;
    while (fmidi_seq_next_event(seq.get(), &sqevt))
        duration = sqevt.time;
    return duration;
}

static fmidi_event_t *fmidi_read_meta_event(
    memstream &mb, std::vector<uint8_t> &evbuf, uint32_t delta)
{
    memstream_status ms;
    unsigned id;
    if ((ms = mb.readbyte(&id)))
        RET_FAIL(nullptr, (fmidi_status)ms);

    uint32_t datalen;
    const uint8_t *data;
    if (id == 0x2f || id == 0x3f) {  // end of track
        if (mb.skipbyte(0)) {
            // omitted final null byte in some broken files
        }
        else {
            // repeated end of track events
            for (bool again = true; again;) {
                size_t offset = mb.getpos();
                again = !mb.readvlq(nullptr) && !mb.skipbyte(0xff) &&
                    (!mb.skipbyte(0x2f) || !mb.skipbyte(0x3f));
                if (!again)
                    mb.setpos(offset);
                else
                    again = !mb.skipbyte(0);
            }
        }
        datalen = 0;
        data = nullptr;
    }
    else {
        if ((ms = mb.readvlq(&datalen)))
            RET_FAIL(nullptr, (fmidi_status)ms);
        if (!(data = mb.read(datalen)))
            RET_FAIL(nullptr, fmidi_err_eof);
    }
    fmidi_event_t *evt = fmidi_event_alloc(evbuf, datalen + 1);
    evt->type = fmidi_event_meta;
    evt->delta = delta;
    evt->datalen = datalen + 1;
    evt->data[0] = id;
    memcpy(&evt->data[1], data, datalen);
    return evt;
}

static fmidi_event_t *fmidi_read_escape_event(
    memstream &mb, std::vector<uint8_t> &evbuf, uint32_t delta)
{
    memstream_status ms;
    uint32_t datalen;
    const uint8_t *data;
    if ((ms = mb.readvlq(&datalen)))
        RET_FAIL(nullptr, (fmidi_status)ms);
    if (!(data = mb.read(datalen)))
        RET_FAIL(nullptr, fmidi_err_eof);

    fmidi_event_t *evt = fmidi_event_alloc(evbuf, datalen);
    evt->type = fmidi_event_escape;
    evt->delta = delta;
    evt->datalen = datalen;
    memcpy(&evt->data[0], data, datalen);
    return evt;
}

static fmidi_event_t *fmidi_read_sysex_event(
    memstream &mb, std::vector<uint8_t> &evbuf, uint32_t delta)
{
    memstream_status ms;
    fmidi_event_t *evt;

    std::vector<uint8_t> syxbuf;
    syxbuf.reserve(256);
    syxbuf.push_back(0xf0);

    uint32_t partlen;
    const uint8_t *part;
    if ((ms = mb.readvlq(&partlen)))
        RET_FAIL(nullptr, (fmidi_status)ms);
    if (!(part = mb.read(partlen)))
        RET_FAIL(nullptr, fmidi_err_eof);

    bool term = false;
    const uint8_t *endp;

    // handle files having multiple concatenated sysex events in one
    while ((endp = (const uint8_t *)memchr(part, 0xf7, partlen))) {
        syxbuf.insert(syxbuf.end(), part, endp + 1);

        evt = fmidi_event_alloc(evbuf, syxbuf.size());
        evt->type = fmidi_event_message;
        evt->delta = delta;
        evt->datalen = syxbuf.size();
        memcpy(&evt->data[0], &syxbuf[0], syxbuf.size());

        uint32_t reallen = endp + 1 - part;
        partlen -= reallen;
        part += reallen;

        if (partlen == 0)
            return evt;

        if (part[0] != 0xf0) {
#if 1
            // trailing garbage, ignore
#else
            // sierra: incorrect length covering part of the next event. repair
            mb.setpos(mb.getpos() - partlen);
#endif
            return evt;
        }
        ++part;
        --partlen;

        syxbuf.clear();
        syxbuf.push_back(0xf0);
    }

    // handle the rest in multiple parts (Casio MIDI)
    while (!term) {
        term = endp;
        if (term && endp + 1 != part + partlen) {
            // ensure no excess bytes
            RET_FAIL(nullptr, fmidi_err_format);
        }
        syxbuf.insert(syxbuf.end(), part, part + partlen);

        if (!term) {
            size_t offset = mb.getpos();
            bool havecont = false;

            uint32_t contdelta;
            unsigned id;
            if (!mb.readvlq(&contdelta) && !mb.readbyte(&id)) {
                // raw sequence incoming? use it as next sysex part
                havecont = id == 0xf7;
            }
            if (havecont) {
                if ((ms = mb.readvlq(&partlen)))
                    RET_FAIL(nullptr, (fmidi_status)ms);
                if (!(part = mb.read(partlen)))
                    RET_FAIL(nullptr, fmidi_err_eof);
                endp = (const uint8_t *)memchr(part, 0xf7, partlen);
            }
            else {
                // no next part? assume unfinished message and repair
                mb.setpos(offset);
                syxbuf.push_back(0xf7);
                term = true;
            }
        }
    }

    evt = fmidi_event_alloc(evbuf, syxbuf.size());
    evt->type = fmidi_event_message;
    evt->delta = delta;
    evt->datalen = syxbuf.size();
    memcpy(&evt->data[0], &syxbuf[0], syxbuf.size());
    return evt;
}

static fmidi_event_t *fmidi_read_message_event(
    memstream &mb, std::vector<uint8_t> &evbuf, unsigned id, uint32_t delta)
{
    uint32_t datalen = fmidi_message_sizeof(id);
    const uint8_t *data;
    if (datalen <= 0)
        RET_FAIL(nullptr, fmidi_err_format);
    if (!(data = mb.read(datalen - 1)))
        RET_FAIL(nullptr, fmidi_err_eof);

    fmidi_event_t *evt = fmidi_event_alloc(evbuf, datalen);
    evt->type = fmidi_event_message;
    evt->delta = delta;
    evt->datalen = datalen;
    evt->data[0] = id;
    memcpy(&evt->data[1], data, datalen - 1);
    return evt;
}

static fmidi_event_t *fmidi_read_event(
    memstream &mb, std::vector<uint8_t> &evbuf, uint8_t *runstatus)
{
    memstream_status ms;
    uint32_t delta;
    unsigned id;
    if ((ms = mb.readvlq(&delta)))
        RET_FAIL(nullptr, (fmidi_status)ms);
    if ((ms = mb.readbyte(&id)))
        RET_FAIL(nullptr, (fmidi_status)ms);

    fmidi_event_t *evt;
    if (id == 0xff) {
        evt = fmidi_read_meta_event(mb, evbuf, delta);
    }
    else if (id == 0xf7) {
        evt = fmidi_read_escape_event(mb, evbuf, delta);
    }
    else if (id == 0xf0) {
        evt = fmidi_read_sysex_event(mb, evbuf, delta);
    }
    else {
        if (id & 128) {
            *runstatus = id;
        }
        else {
            id = *runstatus;
            mb.setpos(mb.getpos() - 1);
        }
        evt = fmidi_read_message_event(mb, evbuf, id, delta);
    }

    return evt;
}

void fmidi_smf_track_begin(fmidi_track_iter_t *it, uint16_t track)
{
    it->track = track;
    it->index = 0;
}

const fmidi_event_t *fmidi_smf_track_next(
    const fmidi_smf_t *smf, fmidi_track_iter_t *it)
{
    if (it->track >= smf->info.track_count)
        return nullptr;

    const fmidi_raw_track &trk = smf->track[it->track];
    const uint8_t *trkdata = trk.data.get();

    const fmidi_event_t *evt = (const fmidi_event_t *)&trkdata[it->index];
    if ((const uint8_t *)evt == trkdata + trk.length)
        return nullptr;

    it->index += fmidi_event_pad(fmidi_event_sizeof(evt->datalen));
    return evt;
}

static bool fmidi_smf_read_contents(fmidi_smf_t *smf, memstream &mb)
{
    uint16_t ntracks = smf->info.track_count;
    smf->track.reset(new fmidi_raw_track[ntracks]);

    std::vector<uint8_t> evbuf;
    evbuf.reserve(8192);

    uint8_t runstatus = 0;  // status runs from track to track

    for (unsigned itrack = 0; itrack < ntracks; ++itrack) {
        fmidi_raw_track &trk = smf->track[itrack];
        size_t trkoffset = mb.getpos();

        memstream_status ms;
        const uint8_t *trackmagic;
        uint32_t tracklen;

        if (!(trackmagic = mb.read(4))) {
            // file has less tracks than promised, repair
            smf->info.track_count = ntracks = itrack;
            break;
        }

        if (memcmp(trackmagic, "MTrk", 4)) {
            if (mb.getpos() == mb.endpos()) {
                // some kind of final junk header, ignore
                smf->info.track_count = ntracks = itrack;
                break;
            }
            RET_FAIL(false, fmidi_err_format);
        }
        if ((ms = mb.readintBE(&tracklen, 4)))
            RET_FAIL(false, (fmidi_status)ms);

        // check track length, broken in many files. disregard if invalid
        bool tracklengood = !mb.skip(tracklen) &&
            (mb.getpos() == mb.endpos() ||
             ((trackmagic = mb.peek(4)) && !memcmp(trackmagic, "MTrk", 4)));
        mb.setpos(trkoffset + 8);

        fmidi_event_t *evt;
        size_t evoffset = mb.getpos();
        bool endoftrack = false;
        evbuf.clear();
        while (!endoftrack && (evt = fmidi_read_event(mb, evbuf, &runstatus))) {
            // some files use 3F instead or 2F for end of track
            endoftrack = evt->type == fmidi_event_meta &&
                (evt->data[0] == 0x2f || evt->data[0] == 0x3f);
            // fmt::print(stderr, "T{} @{:#x} {}\n", itrack, evoffset, *evt);
            evoffset = mb.getpos();
            if (tracklengood && evoffset > trkoffset + 8 + tracklen)
                // next track overlap
                RET_FAIL(false, fmidi_err_format);
        }

        if (!endoftrack) {
            switch (fmidi_last_error.code) {
            case fmidi_err_eof:
                // truncated track? stop reading
                smf->info.track_count = ntracks = itrack + 1;
                break;
            case fmidi_err_format:
                // event with absurdly high delta time? ignore the rest of
                // the track and if possible proceed to the next
                mb.setpos(evoffset);
                if (mb.peekvlq(nullptr) == ms_err_format) {
                    if (!tracklengood)
                        smf->info.track_count = ntracks = itrack + 1;
                    break;
                }
                return false;
            default:
                return false;
            }
        }

        if (endoftrack) {
            // permit meta events coming after end of track
            const uint8_t *head;
            while ((head = mb.peek(2)) && head[0] == 0x00 && head[1] == 0xff) {
                if (!(evt = fmidi_read_event(mb, evbuf, &runstatus))) {
                    if (fmidi_last_error.code == fmidi_err_eof)
                        smf->info.track_count = ntracks = itrack + 1;
                    else
                        return false;
                }
                else if (tracklengood && mb.getpos() > trkoffset + 8 + tracklen)
                    // next track overlap
                    RET_FAIL(false, fmidi_err_format);
            }
        }

        uint32_t evdatalen = trk.length = evbuf.size();
        uint8_t *evdata = new uint8_t[evdatalen];
        trk.data.reset(evdata);
        memcpy(evdata, evbuf.data(), evdatalen);

        if (tracklengood)
            mb.setpos(trkoffset + 8 + tracklen);
    }

    return true;
}

fmidi_smf_t *fmidi_smf_mem_read(const uint8_t *data, size_t length)
{
    memstream mb(data, length);
    memstream_status ms;
    const uint8_t *filemagic;
    uint32_t headerlen;
    uint32_t format;
    uint32_t ntracks;
    uint32_t deltaunit;

    while ((filemagic = mb.peek(4)) && memcmp(filemagic, "MThd", 4))
        mb.skip(1);
    mb.skip(4);

    if (!filemagic)
        RET_FAIL(nullptr, fmidi_err_format);

    if ((ms = mb.readintBE(&headerlen, 4)) ||
        (ms = mb.readintBE(&format, 2)) ||
        (ms = mb.readintBE(&ntracks, 2)) ||
        (ms = mb.readintBE(&deltaunit, 2)))
        RET_FAIL(nullptr, (fmidi_status)ms);

    if (ntracks < 1 || headerlen < 6)
        RET_FAIL(nullptr, fmidi_err_format);

    if ((ms = mb.skip(headerlen - 6)))
        RET_FAIL(nullptr, (fmidi_status)ms);

    std::unique_ptr<fmidi_smf_t> smf(new fmidi_smf_t);
    smf->info.format = format;
    smf->info.track_count = ntracks;
    smf->info.delta_unit = deltaunit;

    if (!fmidi_smf_read_contents(smf.get(), mb))
        return nullptr;

    return smf.release();
}

void fmidi_smf_free(fmidi_smf_t *smf)
{
    delete smf;
}

fmidi_smf_t *fmidi_smf_file_read(const char *filename)
{
    unique_FILE fh(fmidi_fopen(filename, "rb"));
    if (!fh)
        RET_FAIL(nullptr, fmidi_err_input);

    fmidi_smf_t *smf = fmidi_smf_stream_read(fh.get());
    return smf;
}

fmidi_smf_t *fmidi_smf_stream_read(FILE *stream)
{
    struct stat st;
    size_t length;

    rewind(stream);

    if (fstat(fileno(stream), &st) != 0)
        RET_FAIL(nullptr, fmidi_err_input);

    length = st.st_size;
    if (length > fmidi_file_size_limit)
        RET_FAIL(nullptr, fmidi_err_largefile);

    std::unique_ptr<uint8_t[]> buf(new uint8_t[length]);
    if (!fread(buf.get(), length, 1, stream))
        RET_FAIL(nullptr, fmidi_err_input);

    fmidi_smf_t *smf = fmidi_smf_mem_read(buf.get(), length);
    return smf;
}

#include "fmidi/fmidi.h"
#include <cstring>
#include <cassert>

static void write_vlq(uint32_t value, Writer &writer)
{
    unsigned shift = 28;
    unsigned mask = (1u << 7) - 1;
    while (shift > 0 && ((value >> shift) & mask) == 0)
        shift -= 7;
    while (shift > 0) {
        writer.put(((value >> shift) & mask) | (1u << 7));
        shift -= 7;
    }
    writer.put(value & mask);
}

static bool fmidi_smf_write(const fmidi_smf_t *smf, Writer &writer)
{
    writer.write("MThd", 4);

    const uint32_t header_size = 6;
    writer.writeBE(&header_size, 4);

    const fmidi_smf_info_t *info = fmidi_smf_get_info(smf);
    const uint16_t track_count = info->track_count;
    writer.writeBE(&info->format, 2);
    writer.writeBE(&track_count, 2);
    writer.writeBE(&info->delta_unit, 2);

    for (unsigned i = 0; i < track_count; ++i) {
        writer.write("MTrk", 4);

        off_t off_track_length = writer.tell();

        uint32_t track_length = 0;
        writer.writeBE(&track_length, 4);

        int running_status = -1;

        fmidi_track_iter_t iter;
        fmidi_smf_track_begin(&iter, i);

        const fmidi_event_t *event;
        while ((event = fmidi_smf_track_next(smf, &iter))) {
            switch (event->type) {
            case fmidi_event_meta:
                write_vlq(event->delta, writer);
                writer.put(0xff);
                writer.put(event->data[0]);
                write_vlq(event->datalen - 1, writer);
                writer.write(event->data + 1, event->datalen - 1);
                running_status = -1;
                break;
            case fmidi_event_message:
            {
                write_vlq(event->delta, writer);
                uint8_t status = event->data[0];
                if (status == 0xf0) {
                    writer.put(0xf0);
                    write_vlq(event->datalen - 1, writer);
                    writer.write(event->data + 1, event->datalen - 1);
                    running_status = -1;
                }
                else if ((int)status == running_status)
                    writer.write(event->data + 1, event->datalen - 1);
                else {
                    writer.write(event->data, event->datalen);
                    running_status = status;
                }
                break;
            }
            case fmidi_event_escape:
                write_vlq(event->delta, writer);
                writer.put(0xf7);
                write_vlq(event->datalen, writer);
                writer.write(event->data, event->datalen);
                running_status = -1;
                break;
            case fmidi_event_xmi_timbre:
            case fmidi_event_xmi_branch_point:
                break;
            }
        }

        off_t off_track_end = writer.tell();

        track_length =
            std::make_unsigned<off_t>::type(off_track_end) -
            std::make_unsigned<off_t>::type(off_track_length) - 4;

        writer.seek(off_track_length, SEEK_SET);
        writer.writeBE(&track_length, 4);
        writer.seek(off_track_end, SEEK_SET);
    }

    return true;
}

bool fmidi_smf_mem_write(const fmidi_smf_t *smf, uint8_t **data, size_t *length)
{
    std::vector<uint8_t> mem;
    mem.reserve(8192);

    Memory_Writer writer(mem);
    if (!fmidi_smf_write(smf, writer))
        return false;

    assert(data);
    assert(length);

    if (!(*data = (uint8_t *)malloc(mem.size())))
        throw std::bad_alloc();

    memcpy(*data, mem.data(), mem.size());
    *length = mem.size();

    return true;
}

bool fmidi_smf_file_write(const fmidi_smf_t *smf, const char *filename)
{
    unique_FILE fh(fmidi_fopen(filename, "wb"));
    if (!fh)
        RET_FAIL(false, fmidi_err_output);

    return fmidi_smf_stream_write(smf, fh.get());
}

bool fmidi_smf_stream_write(const fmidi_smf_t *smf, FILE *stream)
{
    Stream_Writer writer(stream);
    if (!fmidi_smf_write(smf, writer))
        return false;

    if (fflush(stream) != 0)
        RET_FAIL(false, fmidi_err_output);

    return true;
}

#include "fmidi/fmidi.h"
#include <string.h>

fmidi_fileformat_t fmidi_mem_identify(const uint8_t *data, size_t length)
{
    const uint8_t smf_magic[4] = {'M', 'T', 'h', 'd'};

    for (size_t offset : {0x00, 0x80}) {
        // a few unidentified files start at 0x80 (Sound Canvas MIDI collection)
        if (length + offset >= 4 && memcmp(data + offset, smf_magic, 4) == 0)
            return fmidi_fileformat_smf;
    }

    const uint8_t rmi_magic1[4] = {'R', 'I', 'F', 'F'};
    const uint8_t rmi_magic2[8] = {'R', 'M', 'I', 'D', 'd', 'a', 't', 'a'};
    if (length >= 16 && memcmp(data, rmi_magic1, 4) == 0 && memcmp(data + 8, rmi_magic2, 8) == 0)
        return fmidi_fileformat_smf;

    const uint8_t xmi_magic[20] = {
        'F', 'O', 'R', 'M', 0, 0, 0, 14,
        'X', 'D', 'I', 'R', 'I', 'N', 'F', 'O', 0, 0, 0, 2
    };
    if (length >= 20 && memcmp(data, xmi_magic, 20) == 0)
        return fmidi_fileformat_xmi;

    const uint8_t mus_magic[4] = {'M', 'U', 'S', 0x1a};
    if (length >= 4 && memcmp(data, mus_magic, 4) == 0)
        return fmidi_fileformat_mus;

    RET_FAIL((fmidi_fileformat_t)-1, fmidi_err_format);
}

fmidi_fileformat_t fmidi_stream_identify(FILE *stream)
{
    rewind(stream);

    uint8_t magic[0x100];
    size_t size = fread(magic, 1, sizeof(magic), stream);
    if (ferror(stream))
        RET_FAIL((fmidi_fileformat_t)-1, fmidi_err_input);

    return fmidi_mem_identify(magic, size);
}

fmidi_smf_t *fmidi_auto_mem_read(const uint8_t *data, size_t length)
{
    switch (fmidi_mem_identify(data, length)) {
    case fmidi_fileformat_smf:
        return fmidi_smf_mem_read(data, length);
    case fmidi_fileformat_xmi:
        return fmidi_xmi_mem_read(data, length);
    case fmidi_fileformat_mus:
        return fmidi_mus_mem_read(data, length);
    default:
        return nullptr;
    }
}

fmidi_smf_t *fmidi_auto_file_read(const char *filename)
{
    unique_FILE fh(fmidi_fopen(filename, "rb"));
    if (!fh)
        RET_FAIL(nullptr, fmidi_err_input);

    fmidi_smf_t *smf = fmidi_auto_stream_read(fh.get());
    return smf;
}

fmidi_smf_t *fmidi_auto_stream_read(FILE *stream)
{
    switch (fmidi_stream_identify(stream)) {
    case fmidi_fileformat_smf:
        return fmidi_smf_stream_read(stream);
    case fmidi_fileformat_xmi:
        return fmidi_xmi_stream_read(stream);
    case fmidi_fileformat_mus:
        return fmidi_mus_stream_read(stream);
    default:
        return nullptr;
    }
}

#include "fmidi/fmidi.h"
#include <string.h>

fmidi_smf_t *fmidi_mus_mem_read(const uint8_t *data, size_t length)
{
    const uint8_t magic[] = {'M', 'U', 'S', 0x1a};

    if (length < sizeof(magic) || memcmp(data, magic, 4))
        RET_FAIL(nullptr, fmidi_err_format);

    memstream mb(data + sizeof(magic), length - sizeof(magic));
    memstream_status ms;

    uint32_t score_len;
    uint32_t score_start;
    uint32_t channels;
    uint32_t sec_channels;
    uint32_t instr_cnt;

    if ((ms = mb.readintLE(&score_len, 2)) ||
        (ms = mb.readintLE(&score_start, 2)) ||
        (ms = mb.readintLE(&channels, 2)) ||
        (ms = mb.readintLE(&sec_channels, 2)) ||
        (ms = mb.readintLE(&instr_cnt, 2)) ||
        (ms = mb.skip(2)))
        RET_FAIL(nullptr, fmidi_err_format);

    std::unique_ptr<uint32_t[]> instrs{new uint32_t[instr_cnt]};
    for (uint32_t i = 0; i < instr_cnt; ++i) {
        if ((ms = mb.readintLE(&instrs[i], 2)))
            RET_FAIL(nullptr, fmidi_err_format);
    }

    fmidi_smf_u smf(new fmidi_smf);
    smf->info.format = 0;
    smf->info.track_count = 1;
    smf->info.delta_unit = 70; // DMX 140 Hz -> PPQN at 120 BPM
    smf->track.reset(new fmidi_raw_track[1]);

    fmidi_raw_track &track = smf->track[0];
    std::vector<uint8_t> evbuf;
    evbuf.reserve(8192);

    uint32_t ev_delta = 0;
    uint32_t note_velocity[16] = {};

    for (unsigned channel = 0; channel < 16; ++channel) {
        // initial velocity
        note_velocity[channel] = 64;
        // channel volume
        fmidi_event_t *event = fmidi_event_alloc(evbuf, 3);
        event->type = fmidi_event_message;
        event->delta = ev_delta;
        event->datalen = 3;
        uint8_t *data = event->data;
        data[0] = 0xb0 | channel;
        data[1] = 7;
        data[2] = 127;
    }

    for (bool score_end = false; !score_end;) {
        uint32_t ev_desc;
        if ((ms = mb.readintLE(&ev_desc, 1)))
            RET_FAIL(nullptr, fmidi_err_format);

        const uint8_t mus_channel_to_midi_channel[16] = {
            0,  1,  2,  3,  4,  5,  6,  7,
            8,  10, 11, 12, 13, 14, 15, 9
        };

        bool ev_last = (ev_desc & 128) != 0;
        uint32_t ev_type = (ev_desc >> 4) & 7;
        uint32_t ev_channel = mus_channel_to_midi_channel[ev_desc & 15];

        uint8_t midi[3] {};
        uint8_t midi_size = 0;

        switch (ev_type) {
            // release note
        case 0: {
            uint32_t data1;
            if ((ms = mb.readintLE(&data1, 1)))
                RET_FAIL(nullptr, fmidi_err_format);
            midi[0] = 0x80 | ev_channel;
            midi[1] = data1 & 127;
            midi[2] = 64;
            midi_size = 3;
            break;
        }
            // play note
        case 1: {
            uint32_t data1;
            if ((ms = mb.readintLE(&data1, 1)))
                RET_FAIL(nullptr, fmidi_err_format);
            if (data1 & 128) {
                uint32_t data2;
                if ((ms = mb.readintLE(&data2, 1)))
                    RET_FAIL(nullptr, fmidi_err_format);
                note_velocity[ev_channel] = data2 & 127;
            }
            midi[0] = 0x90 | ev_channel;
            midi[1] = data1 & 127;
            midi[2] = note_velocity[ev_channel];
            midi_size = 3;
            break;
        }
            // pitch wheel
        case 2: {
            uint32_t data1;
            if ((ms = mb.readintLE(&data1, 1)))
                RET_FAIL(nullptr, fmidi_err_format);
            uint32_t bend = (data1 < 128) ? (data1 << 6) :
                (8192 + (data1 - 128) * 8191 / 127);
            midi[0] = 0xe0 | ev_channel;
            midi[1] = bend & 127;
            midi[2] = bend >> 7;
            midi_size = 3;
            break;
        }
            // system event
        case 3: {
            uint32_t data1;
            if ((ms = mb.readintLE(&data1, 1)))
                RET_FAIL(nullptr, fmidi_err_format);
            midi[0] = 0xb0 | ev_channel;
            midi[2] = 0;
            midi_size = 3;
            switch (data1 & 127) {
            case 10: midi[1] = 120; break;
            case 11: midi[1] = 123; break;
            case 12: midi[1] = 126; break;
            case 13: midi[1] = 127; break;
            case 14: midi[1] = 121; break;
            default: midi_size = 0; break;
            }
            break;
        }
            // change controller
        case 4: {
            uint32_t data1;
            if ((ms = mb.readintLE(&data1, 1)))
                RET_FAIL(nullptr, fmidi_err_format);
            uint32_t data2;
            if ((ms = mb.readintLE(&data2, 1)))
                RET_FAIL(nullptr, fmidi_err_format);
            midi[0] = 0xb0 | ev_channel;
            midi[2] = data2 & 127;
            midi_size = 3;
            switch (data1 & 127) {
            case 0:
                // program change
                midi[0] = 0xc0 | ev_channel;
                midi[1] = data2 & 127;
                midi_size = 2;
                break;
            case 1: midi[1] = 0; break;
            case 2: midi[1] = 1; break;
            case 3: midi[1] = 7; break;
            case 4: midi[1] = 10; break;
            case 5: midi[1] = 11; break;
            case 6: midi[1] = 91; break;
            case 7: midi[1] = 93; break;
            case 8: midi[1] = 64; break;
            case 9: midi[1] = 67; break;
            default: midi_size = 0; break;
            }
            break;
        }
            // end of measure
        case 5: {
            break;
        }
            // score end
        case 6: {
            score_end = true;
            break;
        }
            // unknown purpose
        case 7: {
            if ((ms = mb.skip(1)))
                RET_FAIL(nullptr, fmidi_err_format);
            break;
        }
        }

        uint32_t delta_inc = 0;
        if (ev_last) {
            if ((ms = mb.readvlq(&delta_inc)))
                RET_FAIL(nullptr, fmidi_err_format);
            ev_desc += delta_inc;
        }

        if (midi_size > 0) {
            fmidi_event_t *event = fmidi_event_alloc(evbuf, midi_size);
            event->type = fmidi_event_message;
            event->delta = ev_delta;
            event->datalen = midi_size;
            memcpy(event->data, midi, midi_size);
            ev_delta = 0;
        }

        ev_delta += delta_inc;
    }

    fmidi_event_t *event = fmidi_event_alloc(evbuf, 1);
    event->type = fmidi_event_meta;
    event->delta = ev_delta;
    event->datalen = 1;
    event->data[0] = 0x2f;

    uint32_t evdatalen = track.length = evbuf.size();
    uint8_t *evdata = new uint8_t[evdatalen];
    track.data.reset(evdata);
    memcpy(evdata, evbuf.data(), evdatalen);

    return smf.release();
}

fmidi_smf_t *fmidi_mus_file_read(const char *filename)
{
    unique_FILE fh(fmidi_fopen(filename, "rb"));
    if (!fh)
        RET_FAIL(nullptr, fmidi_err_input);

    fmidi_smf_t *smf = fmidi_mus_stream_read(fh.get());
    return smf;
}

fmidi_smf_t *fmidi_mus_stream_read(FILE *stream)
{
    rewind(stream);

    constexpr size_t mus_file_size_limit = 65536;
    uint8_t buf[mus_file_size_limit];

    size_t length = fread(buf, 1, mus_file_size_limit, stream);
    if (ferror(stream))
        RET_FAIL(nullptr, fmidi_err_input);

    fmidi_smf_t *smf = fmidi_mus_mem_read(buf, length);
    return smf;
}

#include "fmidi/fmidi.h"
#include <algorithm>
#include <string.h>
#include <sys/stat.h>
#if defined(_WIN32)
# define fileno _fileno
#endif

#define FOURCC(x)                               \
    (((uint8_t)(x)[0] << 24) |                  \
     ((uint8_t)(x)[1] << 16) |                  \
     ((uint8_t)(x)[2] << 8) |                   \
     ((uint8_t)(x)[3]))

struct fmidi_xmi_timb {
    uint32_t patch;
    uint32_t bank;
};

struct fmidi_xmi_rbrn {
    uint32_t id;
    uint32_t dest;
};

struct fmidi_xmi_note {
    uint32_t delta;
    uint8_t channel;
    uint8_t note;
    uint8_t velo;
};

static bool operator<(const fmidi_xmi_note &a, const fmidi_xmi_note &b)
{
    return a.delta < b.delta;
}

static void fmidi_xmi_emit_noteoffs(
    uint32_t *pdelta, std::vector<fmidi_xmi_note> &noteoffs,
    std::vector<uint8_t> &evbuf)
{
    uint32_t delta = *pdelta;

    std::sort(noteoffs.begin(), noteoffs.end());

    size_t i = 0;
    size_t n = noteoffs.size();

    for (; i < n; ++i) {
        fmidi_xmi_note xn = noteoffs[i];
        if (delta < xn.delta)
            break;

        fmidi_event_t *event = fmidi_event_alloc(evbuf, 3);
        event->type = fmidi_event_message;
        event->delta = xn.delta;
        event->datalen = 3;

        uint8_t *data = event->data;
        data[0] = 0x80 | xn.channel;
        data[1] = xn.note;
        data[2] = xn.velo;

        delta -= xn.delta;
        for (size_t k = i + 1; k < n; ++k)
            noteoffs[k].delta -= xn.delta;
    }

    size_t j = 0;
    for (; i < n; ++i) {
        fmidi_xmi_note xn = noteoffs[i];
        noteoffs[j++] = xn;
    }
    noteoffs.resize(j);

    *pdelta = delta;
}

static bool fmidi_xmi_read_events(
    memstream &mb, fmidi_raw_track &track,
    const fmidi_xmi_timb *timb, uint32_t timb_count,
    const fmidi_xmi_rbrn *rbrn, uint32_t rbrn_count)
{
    memstream_status ms;
    std::vector<uint8_t> evbuf;
    evbuf.reserve(8192);

    std::vector<fmidi_xmi_note> noteoffs;
    noteoffs.reserve(128);

    for (uint32_t i = 0; i < timb_count; ++i) {
        fmidi_event_t *event = fmidi_event_alloc(evbuf, 2);
        event->type = fmidi_event_xmi_timbre;
        event->delta = 0;
        event->datalen = 2;
        uint8_t * data = event->data;
        data[0] = timb[i].patch;
        data[1] = timb[i].bank;
    }

    bool eot = false;
    while (!eot) {
        uint32_t delta = 0;
        unsigned status = 0;

        size_t branch = ~(size_t)0;
        for (uint32_t i = 0; i < rbrn_count && branch == ~(size_t)0; ++i) {
            if (rbrn[i].dest == mb.getpos())
                branch = i;
        }

        while (!(status & 128)) {
            if ((ms = mb.readbyte(&status)))
                RET_FAIL(false, (fmidi_status)ms);
            delta += (status & 128) ? 0 : status;
        }

        if (branch != ~(size_t)0) {
            fmidi_event_t *event = fmidi_event_alloc(evbuf, 1);
            event->type = fmidi_event_xmi_branch_point;
            event->delta = delta;
            event->datalen = 1;
            event->data[0] = rbrn[branch].id;
            delta = 0;
        }

        fmidi_xmi_emit_noteoffs(&delta, noteoffs, evbuf);

        if (status == 0xff) {
            unsigned type;
            uint32_t length;
            if ((ms = mb.readbyte(&type)) ||
                (ms = mb.readvlq(&length)))
                RET_FAIL(false, (fmidi_status)ms);

            const uint8_t *data = mb.read(length);
            if (!data)
                RET_FAIL(false, fmidi_err_eof);

            eot = type == 0x2F;

            if (eot) {
                // emit later
            }
            else if (type == 0x51) {
                // don't emit tempo change
            }
            else {
                fmidi_event_t *event = fmidi_event_alloc(evbuf, length + 1);
                event->type = fmidi_event_meta;
                event->delta = delta;
                event->datalen = length + 1;
                event->data[0] = type;
                memcpy(event->data + 1, data, length);
            }
        }
        else if (status == 0xf0) {
            uint32_t length;
            if ((ms = mb.readvlq(&length)))
                RET_FAIL(false, (fmidi_status)ms);

            const uint8_t *data = mb.read(length);
            if (!data)
                RET_FAIL(false, fmidi_err_eof);

            fmidi_event_t *event = fmidi_event_alloc(evbuf, length + 1);
            event->type = fmidi_event_message;
            event->delta = delta;
            event->datalen = length + 1;
            event->data[0] = 0xf0;
            memcpy(event->data + 1, data, length);
        }
        else if (status == 0xf7) {
            RET_FAIL(false, fmidi_err_format);
        }
        else if ((status & 0xf0) == 0x90) {
            mb.setpos(mb.getpos() - 1);
            const uint8_t *data = mb.read(3);
            if (!data)
                RET_FAIL(false, fmidi_err_eof);

            uint32_t interval;
            if ((ms = mb.readvlq(&interval)))
                RET_FAIL(false, (fmidi_status)ms);

            fmidi_event_t *event = fmidi_event_alloc(evbuf, 3);
            event->type = fmidi_event_message;
            event->delta = delta;
            event->datalen = 3;
            memcpy(event->data, data, 3);

            fmidi_xmi_note noteoff;
            noteoff.delta = interval;
            noteoff.channel = data[0] & 15;
            noteoff.note = data[1];
            noteoff.velo = data[2];
            noteoffs.push_back(noteoff);
        }
        else {
            unsigned length = fmidi_message_sizeof(status);
            mb.setpos(mb.getpos() - 1);
            const uint8_t *data = mb.read(length);
            if (!data)
                RET_FAIL(false, fmidi_err_eof);

            fmidi_event_t *event = fmidi_event_alloc(evbuf, length);
            event->type = fmidi_event_message;
            event->delta = delta;
            event->datalen = length;
            memcpy(event->data, data, length);
        }
    }

    {
        uint32_t delta = UINT32_MAX;
        fmidi_xmi_emit_noteoffs(&delta, noteoffs, evbuf);
    }

    {
        fmidi_event_t *event = fmidi_event_alloc(evbuf, 1);
        event->type = fmidi_event_meta;
        event->delta = 0;
        event->datalen = 1;
        event->data[0] = 0x2F;
    }

    uint32_t evdatalen = track.length = evbuf.size();
    uint8_t *evdata = new uint8_t[evdatalen];
    track.data.reset(evdata);
    memcpy(evdata, evbuf.data(), evdatalen);

    return true;
}

static bool fmidi_xmi_read_track(memstream &mb, fmidi_raw_track &track)
{
    memstream_status ms;

    const uint8_t *fourcc;
    if (!(fourcc = mb.read(4)))
        RET_FAIL(false, fmidi_err_eof);
    if (memcmp(fourcc, "FORM", 4))
        RET_FAIL(false, fmidi_err_format);

    uint32_t formsize;
    if ((ms = mb.readintBE(&formsize, 4)))
        RET_FAIL(false, (fmidi_status)ms);

    const uint8_t *formdata = mb.read(formsize);
    if (!formdata)
        RET_FAIL(false, fmidi_err_eof);
    memstream mbform(formdata, formsize);

    if (!(fourcc = mbform.read(4)))
        RET_FAIL(false, fmidi_err_eof);
    if (memcmp(fourcc, "XMID", 4))
        RET_FAIL(false, fmidi_err_format);

    std::unique_ptr<fmidi_xmi_timb[]> timb;
    uint32_t timb_count = 0;
    std::unique_ptr<fmidi_xmi_rbrn[]> rbrn;
    uint32_t rbrn_count = 0;

    while (mbform.getpos() < mbform.endpos()) {
        if (!(fourcc = mbform.read(4)))
            RET_FAIL(false, fmidi_err_eof);

        uint32_t chunksize;
        if ((ms = mbform.readintBE(&chunksize, 4)))
            RET_FAIL(false, (fmidi_status)ms);

        const uint8_t *chunkdata = mbform.read(chunksize);
        if (!chunkdata)
            RET_FAIL(false, fmidi_err_eof);
        memstream mbchunk(chunkdata, chunksize);

        switch (FOURCC(fourcc)) {
            case FOURCC("TIMB"): {
                if ((ms = mbchunk.readintLE(&timb_count, 2)))
                    RET_FAIL(false, (fmidi_status)ms);

                timb.reset(new fmidi_xmi_timb[timb_count]);
                for (uint32_t i = 0; i < timb_count; ++i) {
                    if ((ms = mbchunk.readintLE(&timb[i].patch, 1)) ||
                        (ms = mbchunk.readintLE(&timb[i].bank, 1)))
                        RET_FAIL(false, (fmidi_status)ms);
                }

                break;
            }
            case FOURCC("RBRN"): {
                if ((ms = mbchunk.readintLE(&rbrn_count, 2)))
                    RET_FAIL(false, (fmidi_status)ms);

                rbrn.reset(new fmidi_xmi_rbrn[rbrn_count]);
                for (uint32_t i = 0; i < rbrn_count; ++i) {
                    if ((ms = mbchunk.readintLE(&rbrn[i].id, 2)) ||
                        (ms = mbchunk.readintLE(&rbrn[i].dest, 4)))
                        RET_FAIL(false, (fmidi_status)ms);
                    if (rbrn[i].id >= 128)
                        RET_FAIL(false, fmidi_err_format);
                }

                break;
            }
            case FOURCC("EVNT"):
                if (!fmidi_xmi_read_events(
                        mbchunk, track,
                        timb.get(), timb_count, rbrn.get(), rbrn_count))
                    return false;

                break;
        }

        if (mb.getpos() & 1) {
            if ((ms = mb.skip(1)))
                RET_FAIL(false, (fmidi_status)ms);
        }
    }

    return true;
}

uint32_t fmidi_xmi_update_unit(fmidi_smf_t *smf)
{
    uint32_t res = 1;
    const fmidi_event_t *evt;
    fmidi_track_iter_t it;
    fmidi_smf_track_begin(&it, 0);
    bool found = false;
    while (!found && (evt = fmidi_smf_track_next(smf, &it))) {
        if (evt->type == fmidi_event_meta) {
            uint8_t id = evt->data[0];
            if (id == 0x51 && evt->datalen == 4) {  // set tempo
                const uint8_t *d24 = &evt->data[1];
                uint32_t tempo = (d24[0] << 16) | (d24[1] << 8) | d24[2];
                res = 3;
                smf->info.delta_unit = tempo * res * 120 / 1000000;
                found = true;
            }
        }
    }
    return res;
}

fmidi_smf_t *fmidi_xmi_mem_read(const uint8_t *data, size_t length)
{
    const uint8_t header[] = {
        'F', 'O', 'R', 'M', 0, 0, 0, 14,
        'X', 'D', 'I', 'R', 'I', 'N', 'F', 'O', 0, 0, 0, 2
    };

    const uint8_t *start = std::search(
        data, data + length, header, header + sizeof(header));
    if (start == data + length)
        RET_FAIL(nullptr, fmidi_err_format);

    length = length - (start - data);
    data = start;

    // ensure padding to even size (The Lost Vikings)
    std::unique_ptr<uint8_t[]> padded;
    if (length & 1) {
        padded.reset(new uint8_t[length + 1]);
        memcpy(padded.get(), data, length);
        padded[length] = 0;
        data = padded.get();
        length = length + 1;
    }

    memstream mb(data + sizeof(header), length - sizeof(header));
    memstream_status ms;

    uint32_t ntracks;
    if ((ms = mb.readintLE(&ntracks, 2)))
        RET_FAIL(nullptr, (fmidi_status)ms);
    if (ntracks < 1)
        RET_FAIL(nullptr, fmidi_err_format);

    const uint8_t *fourcc;
    if (!(fourcc = mb.read(4)))
        RET_FAIL(nullptr, fmidi_err_eof);
    if (memcmp(fourcc, "CAT ", 4))
        RET_FAIL(nullptr, fmidi_err_format);

    uint32_t catsize;
    if ((ms = mb.readintBE(&catsize, 4)))
        RET_FAIL(nullptr, (fmidi_status)ms);
    if (mb.endpos() - mb.getpos() < catsize)
        RET_FAIL(nullptr, fmidi_err_eof);

    if (!(fourcc = mb.read(4)))
        RET_FAIL(nullptr, fmidi_err_eof);
    if (memcmp(fourcc, "XMID", 4))
        RET_FAIL(nullptr, fmidi_err_format);

    fmidi_smf_u smf(new fmidi_smf);
    smf->info.format = (ntracks > 1) ? 2 : 0;
    smf->info.track_count = ntracks;
    smf->info.delta_unit = 60;
    smf->track.reset(new fmidi_raw_track[ntracks]);

    for (uint32_t i = 0; i < ntracks; ++i) {
        if (!fmidi_xmi_read_track(mb, smf->track[i]))
            return nullptr;
        if (mb.getpos() & 1) {
            if ((ms = mb.skip(1)))
                RET_FAIL(nullptr, (fmidi_status)ms);
        }
    }

    uint32_t res = fmidi_xmi_update_unit(smf.get());
    if (res == 0)
        return nullptr;

    for (uint32_t i = 0; i < ntracks; ++i) {
        fmidi_track_iter_t it;
        fmidi_smf_track_begin(&it, i);
        fmidi_event_t *event;
        while ((event = const_cast<fmidi_event_t *>(
                    fmidi_smf_track_next(smf.get(), &it)))) {
            event->delta *= res;
        }
    }

    return smf.release();
}

fmidi_smf_t *fmidi_xmi_file_read(const char *filename)
{
    unique_FILE fh(fmidi_fopen(filename, "rb"));
    if (!fh)
        RET_FAIL(nullptr, fmidi_err_input);

    fmidi_smf_t *smf = fmidi_xmi_stream_read(fh.get());
    return smf;
}

fmidi_smf_t *fmidi_xmi_stream_read(FILE *stream)
{
    struct stat st;
    size_t length;

    rewind(stream);

    if (fstat(fileno(stream), &st) != 0)
        RET_FAIL(nullptr, fmidi_err_input);

    length = st.st_size;
    if (length > fmidi_file_size_limit)
        RET_FAIL(nullptr, fmidi_err_largefile);

    bool pad = length & 1;
    std::unique_ptr<uint8_t[]> buf(new uint8_t[length + pad]);
    if (!fread(buf.get(), length, 1, stream))
        RET_FAIL(nullptr, fmidi_err_input);
    if (pad)
        buf[length] = 0;

    fmidi_smf_t *smf = fmidi_xmi_mem_read(buf.get(), length + pad);
    return smf;
}
