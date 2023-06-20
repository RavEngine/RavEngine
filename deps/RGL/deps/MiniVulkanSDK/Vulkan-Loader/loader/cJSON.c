/*
  Copyright (c) 2009 Dave Gamble
  Copyright (c) 2015-2021 The Khronos Group Inc.
  Copyright (c) 2015-2021 Valve Corporation
  Copyright (c) 2015-2021 LunarG, Inc.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/* cJSON */
/* JSON parser in C. */

#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"

#include "allocation.h"
#include "loader.h"
#include "log.h"

void *cJSON_malloc(const VkAllocationCallbacks *pAllocator, size_t size) {
    return loader_alloc(pAllocator, size, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
}

void *cJSON_malloc_instance_scope(const VkAllocationCallbacks *pAllocator, size_t size) {
    return loader_alloc(pAllocator, size, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
}

void cJSON_Free(const VkAllocationCallbacks *pAllocator, void *pMemory) { loader_free(pAllocator, pMemory); }

/*
// commented out as it is unused - static error code channel requires external locks to be used.
static const char *ep;

const char *cJSON_GetErrorPtr(void) { return ep; }
*/

char *cJSON_strdup(const VkAllocationCallbacks *pAllocator, const char *str) {
    size_t len;
    char *copy;

    len = strlen(str) + 1;
    copy = (char *)cJSON_malloc(pAllocator, len);
    if (!copy) return 0;
    memcpy(copy, str, len);
    return copy;
}

/* Internal constructor. */
cJSON *cJSON_New_Item(const VkAllocationCallbacks *pAllocator) {
    cJSON *node = (cJSON *)cJSON_malloc(pAllocator, sizeof(cJSON));
    if (node) {
        memset(node, 0, sizeof(cJSON));
        node->pAllocator = (VkAllocationCallbacks *)pAllocator;
    }
    return node;
}

/* Delete a cJSON structure. */
void cJSON_Delete(cJSON *c) {
    cJSON *next;
    while (c) {
        next = c->next;
        if (!(c->type & cJSON_IsReference) && c->child) cJSON_Delete(c->child);
        if (!(c->type & cJSON_IsReference) && c->valuestring) cJSON_Free(c->pAllocator, c->valuestring);
        if (!(c->type & cJSON_StringIsConst) && c->string) cJSON_Free(c->pAllocator, c->string);
        cJSON_Free(c->pAllocator, c);
        c = next;
    }
}

/* Parse the input text to generate a number, and populate the result into item.
 */
const char *parse_number(cJSON *item, const char *num) {
    double n = 0, sign = 1, scale = 0;
    int subscale = 0, signsubscale = 1;

    if (*num == '-') sign = -1, num++; /* Has sign? */
    if (*num == '0') num++;            /* is zero */
    if (*num >= '1' && *num <= '9') do
            n = (n * 10.0) + (*num++ - '0');
        while (*num >= '0' && *num <= '9'); /* Number? */
    if (*num == '.' && num[1] >= '0' && num[1] <= '9') {
        num++;
        do n = (n * 10.0) + (*num++ - '0'), scale--;
        while (*num >= '0' && *num <= '9');
    }                               /* Fractional part? */
    if (*num == 'e' || *num == 'E') /* Exponent? */
    {
        num++;
        if (*num == '+')
            num++;
        else if (*num == '-')
            signsubscale = -1, num++;                                                   /* With sign? */
        while (*num >= '0' && *num <= '9') subscale = (subscale * 10) + (*num++ - '0'); /* Number? */
    }

    n = sign * n * pow(10.0, (scale + subscale * signsubscale)); /* number = +/-
                                                                    number.fraction *
                                                                    10^+/- exponent */

    item->valuedouble = n;
    item->valueint = (int)n;
    item->type = cJSON_Number;
    return num;
}

size_t pow2gt(size_t x) {
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

typedef struct {
    char *buffer;
    size_t length;
    size_t offset;
} printbuffer;

char *ensure(const VkAllocationCallbacks *pAllocator, printbuffer *p, size_t needed) {
    char *newbuffer;
    size_t newsize;
    if (!p || !p->buffer) return 0;
    needed += p->offset;
    if (needed <= p->length) return p->buffer + p->offset;

    newsize = pow2gt(needed);
    newbuffer = (char *)cJSON_malloc(pAllocator, newsize);
    if (!newbuffer) {
        cJSON_Free(pAllocator, p->buffer);
        p->length = 0, p->buffer = 0;
        return 0;
    }
    if (newbuffer) memcpy(newbuffer, p->buffer, p->length);
    cJSON_Free(pAllocator, p->buffer);
    p->length = newsize;
    p->buffer = newbuffer;
    return newbuffer + p->offset;
}

size_t cJSON_update(printbuffer *p) {
    char *str;
    if (!p || !p->buffer) return 0;
    str = p->buffer + p->offset;
    return p->offset + strlen(str);
}

/* Render the number nicely from the given item into a string. */
char *print_number(cJSON *item, printbuffer *p) {
    char *str = 0;
    size_t str_buf_size;
    double d = item->valuedouble;
    if (d == 0) {
        str_buf_size = 2; /* special case for 0. */
        if (p)
            str = ensure(item->pAllocator, p, str_buf_size);
        else
            str = (char *)cJSON_malloc(item->pAllocator, str_buf_size);
        if (str) strncpy(str, "0", str_buf_size);
    } else if (fabs(((double)item->valueint) - d) <= DBL_EPSILON && d <= INT_MAX && d >= INT_MIN) {
        str_buf_size = 21; /* 2^64+1 can be represented in 21 chars. */
        if (p)
            str = ensure(item->pAllocator, p, str_buf_size);
        else
            str = (char *)cJSON_malloc(item->pAllocator, str_buf_size);
        if (str) snprintf(str, str_buf_size, "%d", item->valueint);
    } else {
        str_buf_size = 64; /* This is a nice tradeoff. */
        if (p)
            str = ensure(item->pAllocator, p, str_buf_size);
        else
            str = (char *)cJSON_malloc(item->pAllocator, str_buf_size);
        if (str) {
            if (fabs(floor(d) - d) <= DBL_EPSILON && fabs(d) < 1.0e60)
                snprintf(str, str_buf_size, "%.0f", d);
            else if (fabs(d) < 1.0e-6 || fabs(d) > 1.0e9)
                snprintf(str, str_buf_size, "%e", d);
            else
                snprintf(str, str_buf_size, "%f", d);
        }
    }
    return str;
}

unsigned parse_hex4(const char *str) {
    unsigned h = 0;
    if (*str >= '0' && *str <= '9')
        h += (*str) - '0';
    else if (*str >= 'A' && *str <= 'F')
        h += 10 + (*str) - 'A';
    else if (*str >= 'a' && *str <= 'f')
        h += 10 + (*str) - 'a';
    else
        return 0;
    h = h << 4;
    str++;
    if (*str >= '0' && *str <= '9')
        h += (*str) - '0';
    else if (*str >= 'A' && *str <= 'F')
        h += 10 + (*str) - 'A';
    else if (*str >= 'a' && *str <= 'f')
        h += 10 + (*str) - 'a';
    else
        return 0;
    h = h << 4;
    str++;
    if (*str >= '0' && *str <= '9')
        h += (*str) - '0';
    else if (*str >= 'A' && *str <= 'F')
        h += 10 + (*str) - 'A';
    else if (*str >= 'a' && *str <= 'f')
        h += 10 + (*str) - 'a';
    else
        return 0;
    h = h << 4;
    str++;
    if (*str >= '0' && *str <= '9')
        h += (*str) - '0';
    else if (*str >= 'A' && *str <= 'F')
        h += 10 + (*str) - 'A';
    else if (*str >= 'a' && *str <= 'f')
        h += 10 + (*str) - 'a';
    else
        return 0;
    return h;
}

/* Parse the input text into an unescaped cstring, and populate item. */
static const unsigned char firstByteMark[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
const char *parse_string(cJSON *item, const char *str) {
    const char *ptr = str + 1;
    char *ptr2;
    char *out;
    int len = 0;
    unsigned uc, uc2;
    if (*str != '\"') {
        // ep = str; // commented out as it is unused
        return 0;
    } /* not a string! */

    while (*ptr != '\"' && *ptr && ++len)
        if (*ptr++ == '\\') ptr++; /* Skip escaped quotes. */

    out = (char *)cJSON_malloc(item->pAllocator, len + 1); /* This is how long we need for the string, roughly. */
    if (!out) return 0;

    ptr = str + 1;
    ptr2 = out;
    while (*ptr != '\"' && *ptr) {
        if (*ptr != '\\')
            *ptr2++ = *ptr++;
        else {
            ptr++;
            switch (*ptr) {
                case 'b':
                    *ptr2++ = '\b';
                    break;
                case 'f':
                    *ptr2++ = '\f';
                    break;
                case 'n':
                    *ptr2++ = '\n';
                    break;
                case 'r':
                    *ptr2++ = '\r';
                    break;
                case 't':
                    *ptr2++ = '\t';
                    break;
                case 'u': /* transcode utf16 to utf8. */
                    uc = parse_hex4(ptr + 1);
                    ptr += 4; /* get the unicode char. */

                    if ((uc >= 0xDC00 && uc <= 0xDFFF) || uc == 0) break; /* check for invalid.	*/

                    if (uc >= 0xD800 && uc <= 0xDBFF) /* UTF16 surrogate pairs.	*/
                    {
                        if (ptr[1] != '\\' || ptr[2] != 'u') break; /* missing second-half of surrogate.	*/
                        uc2 = parse_hex4(ptr + 3);
                        ptr += 6;
                        if (uc2 < 0xDC00 || uc2 > 0xDFFF) break; /* invalid second-half of surrogate.	*/
                        uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
                    }

                    len = 4;
                    if (uc < 0x80)
                        len = 1;
                    else if (uc < 0x800)
                        len = 2;
                    else if (uc < 0x10000)
                        len = 3;
                    ptr2 += len;

                    for (size_t i = len; i > 0; i--) {
                        if (i == 1) {
                            *--ptr2 = ((unsigned char)uc | firstByteMark[len]);
                        } else if (i >= 2) {
                            *--ptr2 = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        }
                    }
                    ptr2 += len;
                    break;
                default:
                    *ptr2++ = *ptr;
                    break;
            }
            ptr++;
        }
    }
    *ptr2 = 0;
    if (*ptr == '\"') ptr++;
    item->valuestring = out;
    item->type = cJSON_String;
    return ptr;
}

/* Render the cstring provided to an escaped version that can be printed. */
char *print_string_ptr(const VkAllocationCallbacks *pAllocator, const char *str, printbuffer *p) {
    const char *ptr;
    char *ptr2;
    char *out;
    size_t out_buf_size, len = 0, flag = 0;
    unsigned char token;

    for (ptr = str; *ptr; ptr++) flag |= ((*ptr > 0 && *ptr < 32) || (*ptr == '\"') || (*ptr == '\\')) ? 1 : 0;
    if (!flag) {
        len = ptr - str;
        out_buf_size = len + 1;
        // out_buf_size = len + 3; // Modified to not put quotes around the string
        if (p)
            out = ensure(pAllocator, p, out_buf_size);
        else
            out = (char *)cJSON_malloc_instance_scope(pAllocator, out_buf_size);
        if (!out) return 0;
        ptr2 = out;
        // *ptr2++ = '\"'; // Modified to not put quotes around the string
        strncpy(ptr2, str, out_buf_size);
        // ptr2[len] = '\"'; // Modified to not put quotes around the string
        ptr2[len] = 0;  // ptr2[len + 1] = 0; // Modified to not put quotes around the string
        return out;
    }

    if (!str) {
        out_buf_size = 3;
        if (p)
            out = ensure(pAllocator, p, out_buf_size);
        else
            out = (char *)cJSON_malloc_instance_scope(pAllocator, out_buf_size);
        if (!out) return 0;
        strncpy(out, "\"\"", out_buf_size);
        return out;
    }
    ptr = str;
    token = *ptr;
    while (token && ++len) {
        if (strchr("\"\\\b\f\n\r\t", token))
            len++;
        else if (token < 32)
            len += 5;
        ptr++;
        token = *ptr;
    }

    out_buf_size = len + 1;
    // out_buf_size = len + 3; // Modified to not put quotes around the string
    if (p)
        out = ensure(pAllocator, p, out_buf_size);
    else
        out = (char *)cJSON_malloc_instance_scope(pAllocator, out_buf_size);
    if (!out) return 0;

    ptr2 = out;
    ptr = str;
    // *ptr2++ = '\"'; // Modified to not put quotes around the string
    while (*ptr) {
        if ((unsigned char)*ptr > 31 && *ptr != '\"' && *ptr != '\\')
            *ptr2++ = *ptr++;
        else {
            switch (token = *ptr++) {
                case '\\':
                    *ptr2++ = '\\';
                    break;
                case '\"':
                    *ptr2++ = '\"';
                    break;
                case '\b':
                    *ptr2++ = '\b';
                    break;
                case '\f':
                    *ptr2++ = '\f';
                    break;
                case '\n':
                    *ptr2++ = '\n';
                    break;
                case '\r':
                    *ptr2++ = '\r';
                    break;
                case '\t':
                    *ptr2++ = '\t';
                    break;
                default:
                    snprintf(ptr2, out_buf_size - (ptr2 - out), "u%04x", token);
                    ptr2 += 5;
                    break; /* escape and print */
            }
        }
    }
    // *ptr2++ = '\"'; // Modified to not put quotes around the string
    *ptr2++ = 0;
    return out;
}
/* Invoke print_string_ptr (which is useful) on an item. */
char *print_string(cJSON *item, printbuffer *p) { return print_string_ptr(item->pAllocator, item->valuestring, p); }

/* Predeclare these prototypes. */
const char *parse_value(cJSON *item, const char *value);
char *print_value(cJSON *item, int depth, int fmt, printbuffer *p);
const char *parse_array(cJSON *item, const char *value);
char *print_array(cJSON *item, int depth, int fmt, printbuffer *p);
const char *parse_object(cJSON *item, const char *value);
char *print_object(cJSON *item, int depth, int fmt, printbuffer *p);

/* Utility to jump whitespace and cr/lf */
const char *skip(const char *in) {
    while (in && *in && (unsigned char)*in <= 32) in++;
    return in;
}

/* Parse an object - create a new root, and populate. */
cJSON *cJSON_ParseWithOpts(const VkAllocationCallbacks *pAllocator, const char *value, const char **return_parse_end,
                           int require_null_terminated) {
    const char *end = 0;
    cJSON *c = cJSON_New_Item(pAllocator);
    // ep = 0; // commented out as it is unused
    if (!c) return 0; /* memory fail */

    end = parse_value(c, skip(value));
    if (!end) {
        cJSON_Delete(c);
        return 0;
    } /* parse failure. ep is set. */

    /* if we require null-terminated JSON without appended garbage, skip and
     * then check for a null terminator */
    if (require_null_terminated) {
        end = skip(end);
        if (*end) {
            cJSON_Delete(c);
            // ep = end; // commented out as it is unused
            return 0;
        }
    }
    if (return_parse_end) *return_parse_end = end;
    return c;
}
/* Default options for cJSON_Parse */
cJSON *cJSON_Parse(const VkAllocationCallbacks *pAllocator, const char *value) {
    return cJSON_ParseWithOpts(pAllocator, value, 0, 0);
}

/* Render a cJSON item/entity/structure to text. */
char *cJSON_Print(cJSON *item) { return print_value(item, 0, 1, 0); }
char *cJSON_PrintUnformatted(cJSON *item) { return print_value(item, 0, 0, 0); }

char *cJSON_PrintBuffered(cJSON *item, int prebuffer, int fmt) {
    printbuffer p;
    p.buffer = (char *)cJSON_malloc(item->pAllocator, prebuffer);
    p.length = prebuffer;
    p.offset = 0;
    return print_value(item, 0, fmt, &p);
}

/* Parser core - when encountering text, process appropriately. */
const char *parse_value(cJSON *item, const char *value) {
    if (!value) return 0; /* Fail on null. */
    if (!strncmp(value, "null", 4)) {
        item->type = cJSON_NULL;
        return value + 4;
    }
    if (!strncmp(value, "false", 5)) {
        item->type = cJSON_False;
        return value + 5;
    }
    if (!strncmp(value, "true", 4)) {
        item->type = cJSON_True;
        item->valueint = 1;
        return value + 4;
    }
    if (*value == '\"') {
        return parse_string(item, value);
    }
    if (*value == '-' || (*value >= '0' && *value <= '9')) {
        return parse_number(item, value);
    }
    if (*value == '[') {
        return parse_array(item, value);
    }
    if (*value == '{') {
        return parse_object(item, value);
    }

    // ep = value; // commented out as it is unused
    return 0; /* failure. */
}

/* Render a value to text. */
char *print_value(cJSON *item, int depth, int fmt, printbuffer *p) {
    char *out = 0;
    if (!item) return 0;
    if (p) {
        switch ((item->type) & 255) {
            case cJSON_NULL: {
                out = ensure(item->pAllocator, p, 5);
                if (out) strncpy(out, "null", 5);
                break;
            }
            case cJSON_False: {
                out = ensure(item->pAllocator, p, 6);
                if (out) strncpy(out, "false", 6);
                break;
            }
            case cJSON_True: {
                out = ensure(item->pAllocator, p, 5);
                if (out) strncpy(out, "true", 5);
                break;
            }
            case cJSON_Number:
                out = print_number(item, p);
                break;
            case cJSON_String:
                out = print_string(item, p);
                break;
            case cJSON_Array:
                out = print_array(item, depth, fmt, p);
                break;
            case cJSON_Object:
                out = print_object(item, depth, fmt, p);
                break;
        }
    } else {
        switch ((item->type) & 255) {
            case cJSON_NULL:
                out = cJSON_strdup(item->pAllocator, "null");
                break;
            case cJSON_False:
                out = cJSON_strdup(item->pAllocator, "false");
                break;
            case cJSON_True:
                out = cJSON_strdup(item->pAllocator, "true");
                break;
            case cJSON_Number:
                out = print_number(item, 0);
                break;
            case cJSON_String:
                out = print_string(item, 0);
                break;
            case cJSON_Array:
                out = print_array(item, depth, fmt, 0);
                break;
            case cJSON_Object:
                out = print_object(item, depth, fmt, 0);
                break;
        }
    }
    return out;
}

/* Build an array from input text. */
const char *parse_array(cJSON *item, const char *value) {
    cJSON *child;
    if (*value != '[') {
        // ep = value; // commented out as it is unused
        return 0;
    } /* not an array! */

    item->type = cJSON_Array;
    value = skip(value + 1);
    if (*value == ']') return value + 1; /* empty array. */

    item->child = child = cJSON_New_Item(item->pAllocator);
    if (!item->child) return 0;                    /* memory fail */
    value = skip(parse_value(child, skip(value))); /* skip any spacing, get the value. */
    if (!value) return 0;

    while (*value == ',') {
        cJSON *new_item;
        new_item = cJSON_New_Item(item->pAllocator);
        if (!new_item) return 0; /* memory fail */
        child->next = new_item;
        new_item->prev = child;
        child = new_item;
        value = skip(parse_value(child, skip(value + 1)));
        if (!value) return 0; /* memory fail */
    }

    if (*value == ']') return value + 1; /* end of array */
    // ep = value; // commented out as it is unused
    return 0; /* malformed. */
}

/* Render an array to text */
char *print_array(cJSON *item, int depth, int fmt, printbuffer *p) {
    char **entries;
    char *out = 0, *ptr, *ret;
    size_t len = 5;
    cJSON *child = item->child;
    int numentries = 0, fail = 0, j = 0;
    size_t tmplen = 0, i = 0;

    /* How many entries in the array? */
    while (child) numentries++, child = child->next;
    /* Explicitly handle numentries==0 */
    if (!numentries) {
        if (p)
            out = ensure(item->pAllocator, p, 3);
        else
            out = (char *)cJSON_malloc(item->pAllocator, 3);
        if (out) strncpy(out, "[]", 3);
        return out;
    }

    if (p) {
        /* Compose the output array. */
        i = p->offset;
        ptr = ensure(item->pAllocator, p, 1);
        if (!ptr) return 0;
        *ptr = '[';
        p->offset++;
        child = item->child;
        while (child && !fail) {
            print_value(child, depth + 1, fmt, p);
            p->offset = cJSON_update(p);
            if (child->next) {
                len = fmt ? 2 : 1;
                ptr = ensure(item->pAllocator, p, len + 1);
                if (!ptr) return 0;
                *ptr++ = ',';
                if (fmt) *ptr++ = ' ';
                *ptr = 0;
                p->offset += len;
            }
            child = child->next;
        }
        ptr = ensure(item->pAllocator, p, 2);
        if (!ptr) return 0;
        *ptr++ = ']';
        *ptr = 0;
        out = (p->buffer) + i;
    } else {
        /* Allocate an array to hold the values for each */
        entries = (char **)cJSON_malloc(item->pAllocator, numentries * sizeof(char *));
        if (!entries) return 0;
        memset(entries, 0, numentries * sizeof(char *));
        /* Retrieve all the results: */
        child = item->child;
        while (child && !fail) {
            ret = print_value(child, depth + 1, fmt, 0);
            entries[i++] = ret;
            if (ret)
                len += strlen(ret) + 2 + (fmt ? 1 : 0);
            else
                fail = 1;
            child = child->next;
        }

        /* If we didn't fail, try to malloc the output string */
        if (!fail) out = (char *)cJSON_malloc(item->pAllocator, len);
        /* If that fails, we fail. */
        if (!out) fail = 1;

        /* Handle failure. */
        if (fail) {
            for (j = 0; j < numentries; j++)
                if (entries[j]) cJSON_Free(item->pAllocator, entries[j]);
            cJSON_Free(item->pAllocator, entries);
            return 0;
        }

        /* Compose the output array. */
        *out = '[';
        ptr = out + 1;
        *ptr = 0;
        for (j = 0; j < numentries; j++) {
            tmplen = strlen(entries[j]);
            memcpy(ptr, entries[j], tmplen);
            ptr += tmplen;
            if (j != numentries - 1) {
                *ptr++ = ',';
                if (fmt) *ptr++ = ' ';
                *ptr = 0;
            }
            cJSON_Free(item->pAllocator, entries[j]);
        }
        cJSON_Free(item->pAllocator, entries);
        *ptr++ = ']';
        *ptr++ = 0;
    }
    return out;
}

/* Build an object from the text. */
const char *parse_object(cJSON *item, const char *value) {
    cJSON *child;
    if (*value != '{') {
        // ep = value; // commented out as it is unused
        return 0;
    } /* not an object! */

    item->type = cJSON_Object;
    value = skip(value + 1);
    if (*value == '}') return value + 1; /* empty array. */

    item->child = child = cJSON_New_Item(item->pAllocator);
    if (!item->child) return 0;
    value = skip(parse_string(child, skip(value)));
    if (!value) return 0;
    child->string = child->valuestring;
    child->valuestring = 0;
    if (*value != ':') {
        // ep = value; // commented out as it is unused
        return 0;
    }                                                  /* fail! */
    value = skip(parse_value(child, skip(value + 1))); /* skip any spacing, get the value. */
    if (!value) return 0;

    while (*value == ',') {
        cJSON *new_item;
        new_item = cJSON_New_Item(item->pAllocator);
        if (!new_item) return 0; /* memory fail */
        child->next = new_item;
        new_item->prev = child;
        child = new_item;
        value = skip(parse_string(child, skip(value + 1)));
        if (!value) return 0;
        child->string = child->valuestring;
        child->valuestring = 0;
        if (*value != ':') {
            // ep = value; // commented out as it is unused
            return 0;
        }                                                  /* fail! */
        value = skip(parse_value(child, skip(value + 1))); /* skip any spacing, get the value. */
        if (!value) return 0;
    }

    if (*value == '}') return value + 1; /* end of array */
    // ep = value; // commented out as it is unused
    return 0; /* malformed. */
}

/* Render an object to text. */
char *print_object(cJSON *item, int depth, int fmt, printbuffer *p) {
    char **entries = 0, **names = 0;
    char *out = 0, *ptr, *ret, *str;
    int j;
    cJSON *child = item->child;
    int numentries = 0, fail = 0, k;
    size_t tmplen = 0, i = 0, len = 7;
    /* Count the number of entries. */
    while (child) numentries++, child = child->next;
    /* Explicitly handle empty object case */
    if (!numentries) {
        if (p)
            out = ensure(item->pAllocator, p, fmt ? depth + 4 : 3);
        else
            out = (char *)cJSON_malloc(item->pAllocator, fmt ? depth + 4 : 3);
        if (!out) return 0;
        ptr = out;
        *ptr++ = '{';
        if (fmt) {
            *ptr++ = '\n';
            for (j = 0; j < depth - 1; j++) *ptr++ = '\t';
        }
        *ptr++ = '}';
        *ptr++ = 0;
        return out;
    }
    if (p) {
        /* Compose the output: */
        i = p->offset;
        len = fmt ? 2 : 1;
        ptr = ensure(item->pAllocator, p, len + 1);
        if (!ptr) return 0;
        *ptr++ = '{';
        if (fmt) *ptr++ = '\n';
        *ptr = 0;
        p->offset += len;
        child = item->child;
        depth++;
        while (child) {
            if (fmt) {
                ptr = ensure(item->pAllocator, p, depth);
                if (!ptr) return 0;
                for (j = 0; j < depth; j++) *ptr++ = '\t';
                p->offset += depth;
            }
            print_string_ptr(item->pAllocator, child->string, p);
            p->offset = cJSON_update(p);

            len = fmt ? 2 : 1;
            ptr = ensure(item->pAllocator, p, len);
            if (!ptr) return 0;
            *ptr++ = ':';
            if (fmt) *ptr++ = '\t';
            p->offset += len;

            print_value(child, depth, fmt, p);
            p->offset = cJSON_update(p);

            len = (fmt ? 1 : 0) + (child->next ? 1 : 0);
            ptr = ensure(item->pAllocator, p, len + 1);
            if (!ptr) return 0;
            if (child->next) *ptr++ = ',';
            if (fmt) *ptr++ = '\n';
            *ptr = 0;
            p->offset += len;
            child = child->next;
        }
        ptr = ensure(item->pAllocator, p, fmt ? (depth + 1) : 2);
        if (!ptr) return 0;
        if (fmt)
            for (j = 0; j < depth - 1; j++) *ptr++ = '\t';
        *ptr++ = '}';
        *ptr = 0;
        out = (p->buffer) + i;
    } else {
        /* Allocate space for the names and the objects */
        entries = (char **)cJSON_malloc(item->pAllocator, numentries * sizeof(char *));
        if (!entries) return 0;
        names = (char **)cJSON_malloc(item->pAllocator, numentries * sizeof(char *));
        if (!names) {
            cJSON_Free(item->pAllocator, entries);
            return 0;
        }
        memset(entries, 0, sizeof(char *) * numentries);
        memset(names, 0, sizeof(char *) * numentries);

        /* Collect all the results into our arrays: */
        child = item->child;
        depth++;
        if (fmt) len += depth;
        while (child) {
            names[i] = str = print_string_ptr(item->pAllocator, child->string, 0);
            entries[i++] = ret = print_value(child, depth, fmt, 0);
            if (str && ret)
                len += strlen(ret) + strlen(str) + 2 + (fmt ? 2 + depth : 0);
            else
                fail = 1;
            child = child->next;
        }

        /* Try to allocate the output string */
        if (!fail) out = (char *)cJSON_malloc(item->pAllocator, len);
        if (!out) fail = 1;

        /* Handle failure */
        if (fail) {
            for (j = 0; j < numentries; j++) {
                if (names[i]) cJSON_Free(item->pAllocator, names[j]);
                if (entries[j]) cJSON_Free(item->pAllocator, entries[j]);
            }
            cJSON_Free(item->pAllocator, names);
            cJSON_Free(item->pAllocator, entries);
            return 0;
        }

        /* Compose the output: */
        *out = '{';
        ptr = out + 1;
        if (fmt) *ptr++ = '\n';
        *ptr = 0;
        for (j = 0; j < numentries; j++) {
            if (fmt)
                for (k = 0; k < depth; k++) *ptr++ = '\t';
            tmplen = strlen(names[j]);
            memcpy(ptr, names[j], tmplen);
            ptr += tmplen;
            *ptr++ = ':';
            if (fmt) *ptr++ = '\t';
            strcpy(ptr, entries[j]);
            ptr += strlen(entries[j]);
            if (j != numentries - 1) *ptr++ = ',';
            if (fmt) *ptr++ = '\n';
            *ptr = 0;
            cJSON_Free(item->pAllocator, names[j]);
            cJSON_Free(item->pAllocator, entries[j]);
        }

        cJSON_Free(item->pAllocator, names);
        cJSON_Free(item->pAllocator, entries);
        if (fmt)
            for (j = 0; j < depth - 1; j++) *ptr++ = '\t';
        *ptr++ = '}';
        *ptr++ = 0;
    }
    return out;
}

/* Get Array size/item / object item. */
int cJSON_GetArraySize(cJSON *array) {
    cJSON *c = array->child;
    int i = 0;
    while (c) i++, c = c->next;
    return i;
}
cJSON *cJSON_GetArrayItem(cJSON *array, int item) {
    cJSON *c = array->child;
    while (c && item > 0) item--, c = c->next;
    return c;
}
cJSON *cJSON_GetObjectItem(cJSON *object, const char *string) {
    cJSON *c = object->child;
    while (c && strcmp(c->string, string)) c = c->next;
    return c;
}

/* Utility for array list handling. */
void suffix_object(cJSON *prev, cJSON *item) {
    prev->next = item;
    item->prev = prev;
}
/* Utility for handling references. */
cJSON *create_reference(cJSON *item) {
    cJSON *ref = cJSON_New_Item(item->pAllocator);
    if (!ref) return 0;
    memcpy(ref, item, sizeof(cJSON));
    ref->string = 0;
    ref->type |= cJSON_IsReference;
    ref->next = ref->prev = 0;
    return ref;
}

/* Add item to array/object. */
void cJSON_AddItemToArray(cJSON *array, cJSON *item) {
    cJSON *c = array->child;
    if (!item) return;
    if (!c) {
        array->child = item;
    } else {
        while (c && c->next) c = c->next;
        suffix_object(c, item);
    }
}
void cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item) {
    if (!item) return;
    if (item->string) cJSON_Free(object->pAllocator, item->string);
    item->string = cJSON_strdup(object->pAllocator, string);
    cJSON_AddItemToArray(object, item);
}
void cJSON_AddItemToObjectCS(cJSON *object, const char *string, cJSON *item) {
    if (!item) return;
    if (!(item->type & cJSON_StringIsConst) && item->string) cJSON_Free(object->pAllocator, item->string);
    item->string = (char *)string;
    item->type |= cJSON_StringIsConst;
    cJSON_AddItemToArray(object, item);
}
void cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item) { cJSON_AddItemToArray(array, create_reference(item)); }
void cJSON_AddItemReferenceToObject(cJSON *object, const char *string, cJSON *item) {
    cJSON_AddItemToObject(object, string, create_reference(item));
}

cJSON *cJSON_DetachItemFromArray(cJSON *array, int which) {
    cJSON *c = array->child;
    while (c && which > 0) c = c->next, which--;
    if (!c) return 0;
    if (c->prev) c->prev->next = c->next;
    if (c->next) c->next->prev = c->prev;
    if (c == array->child) array->child = c->next;
    c->prev = c->next = 0;
    return c;
}
void cJSON_DeleteItemFromArray(cJSON *array, int which) { cJSON_Delete(cJSON_DetachItemFromArray(array, which)); }
cJSON *cJSON_DetachItemFromObject(cJSON *object, const char *string) {
    int i = 0;
    cJSON *c = object->child;
    while (c && strcmp(c->string, string)) i++, c = c->next;
    if (c) return cJSON_DetachItemFromArray(object, i);
    return 0;
}
void cJSON_DeleteItemFromObject(cJSON *object, const char *string) { cJSON_Delete(cJSON_DetachItemFromObject(object, string)); }

/* Replace array/object items with new ones. */
void cJSON_InsertItemInArray(cJSON *array, int which, cJSON *newitem) {
    cJSON *c = array->child;
    while (c && which > 0) c = c->next, which--;
    if (!c) {
        cJSON_AddItemToArray(array, newitem);
        return;
    }
    newitem->next = c;
    newitem->prev = c->prev;
    c->prev = newitem;
    if (c == array->child)
        array->child = newitem;
    else
        newitem->prev->next = newitem;
}
void cJSON_ReplaceItemInArray(cJSON *array, int which, cJSON *newitem) {
    cJSON *c = array->child;
    while (c && which > 0) c = c->next, which--;
    if (!c) return;
    newitem->next = c->next;
    newitem->prev = c->prev;
    if (newitem->next) newitem->next->prev = newitem;
    if (c == array->child)
        array->child = newitem;
    else
        newitem->prev->next = newitem;
    c->next = c->prev = 0;
    cJSON_Delete(c);
}
void cJSON_ReplaceItemInObject(cJSON *object, const char *string, cJSON *newitem) {
    int i = 0;
    cJSON *c = object->child;
    while (c && strcmp(c->string, string)) i++, c = c->next;
    if (c) {
        newitem->string = cJSON_strdup(object->pAllocator, string);
        cJSON_ReplaceItemInArray(object, i, newitem);
    }
}

/* Create basic types: */
cJSON *cJSON_CreateNull(const VkAllocationCallbacks *pAllocator) {
    cJSON *item = cJSON_New_Item(pAllocator);
    if (item) item->type = cJSON_NULL;
    return item;
}
cJSON *cJSON_CreateTrue(const VkAllocationCallbacks *pAllocator) {
    cJSON *item = cJSON_New_Item(pAllocator);
    if (item) item->type = cJSON_True;
    return item;
}
cJSON *cJSON_CreateFalse(const VkAllocationCallbacks *pAllocator) {
    cJSON *item = cJSON_New_Item(pAllocator);
    if (item) item->type = cJSON_False;
    return item;
}
cJSON *cJSON_CreateBool(const VkAllocationCallbacks *pAllocator, int b) {
    cJSON *item = cJSON_New_Item(pAllocator);
    if (item) item->type = b ? cJSON_True : cJSON_False;
    return item;
}
cJSON *cJSON_CreateNumber(const VkAllocationCallbacks *pAllocator, double num) {
    cJSON *item = cJSON_New_Item(pAllocator);
    if (item) {
        item->type = cJSON_Number;
        item->valuedouble = num;
        item->valueint = (int)num;
    }
    return item;
}
cJSON *cJSON_CreateString(const VkAllocationCallbacks *pAllocator, const char *string) {
    cJSON *item = cJSON_New_Item(pAllocator);
    if (item) {
        item->type = cJSON_String;
        item->valuestring = cJSON_strdup(pAllocator, string);
    }
    return item;
}
cJSON *cJSON_CreateArray(const VkAllocationCallbacks *pAllocator) {
    cJSON *item = cJSON_New_Item(pAllocator);
    if (item) item->type = cJSON_Array;
    return item;
}
cJSON *cJSON_CreateObject(const VkAllocationCallbacks *pAllocator) {
    cJSON *item = cJSON_New_Item(pAllocator);
    if (item) item->type = cJSON_Object;
    return item;
}

/* Create Arrays: */
cJSON *cJSON_CreateIntArray(const VkAllocationCallbacks *pAllocator, const int *numbers, int count) {
    int i;
    cJSON *n = 0, *p = 0, *a = cJSON_CreateArray(pAllocator);
    for (i = 0; a && i < count; i++) {
        n = cJSON_CreateNumber(pAllocator, numbers[i]);
        if (!i)
            a->child = n;
        else
            suffix_object(p, n);
        p = n;
    }
    return a;
}
cJSON *cJSON_CreateFloatArray(const VkAllocationCallbacks *pAllocator, const float *numbers, int count) {
    int i;
    cJSON *n = 0, *p = 0, *a = cJSON_CreateArray(pAllocator);
    for (i = 0; a && i < count; i++) {
        n = cJSON_CreateNumber(pAllocator, numbers[i]);
        if (!i)
            a->child = n;
        else
            suffix_object(p, n);
        p = n;
    }
    return a;
}
cJSON *cJSON_CreateDoubleArray(const VkAllocationCallbacks *pAllocator, const double *numbers, int count) {
    int i;
    cJSON *n = 0, *p = 0, *a = cJSON_CreateArray(pAllocator);
    for (i = 0; a && i < count; i++) {
        n = cJSON_CreateNumber(pAllocator, numbers[i]);
        if (!i)
            a->child = n;
        else
            suffix_object(p, n);
        p = n;
    }
    return a;
}
cJSON *cJSON_CreateStringArray(const VkAllocationCallbacks *pAllocator, const char **strings, int count) {
    int i;
    cJSON *n = 0, *p = 0, *a = cJSON_CreateArray(pAllocator);
    for (i = 0; a && i < count; i++) {
        n = cJSON_CreateString(pAllocator, strings[i]);
        if (!i)
            a->child = n;
        else
            suffix_object(p, n);
        p = n;
    }
    return a;
}

/* Duplication */
cJSON *cJSON_Duplicate(cJSON *item, int recurse) {
    cJSON *newitem, *cptr, *nptr = 0, *newchild;
    /* Bail on bad ptr */
    if (!item) return 0;
    /* Create new item */
    newitem = cJSON_New_Item(item->pAllocator);
    if (!newitem) return 0;
    /* Copy over all vars */
    newitem->type = item->type & (~cJSON_IsReference), newitem->valueint = item->valueint, newitem->valuedouble = item->valuedouble;
    if (item->valuestring) {
        newitem->valuestring = cJSON_strdup(item->pAllocator, item->valuestring);
        if (!newitem->valuestring) {
            cJSON_Delete(newitem);
            return 0;
        }
    }
    if (item->string) {
        newitem->string = cJSON_strdup(item->pAllocator, item->string);
        if (!newitem->string) {
            cJSON_Delete(newitem);
            return 0;
        }
    }
    /* If non-recursive, then we're done! */
    if (!recurse) return newitem;
    /* Walk the ->next chain for the child. */
    cptr = item->child;
    while (cptr) {
        newchild = cJSON_Duplicate(cptr, 1); /* Duplicate (with recurse) each item in the ->next chain */
        if (!newchild) {
            cJSON_Delete(newitem);
            return 0;
        }
        if (nptr) {
            nptr->next = newchild, newchild->prev = nptr;
            nptr = newchild;
        } /* If newitem->child already set, then crosswire ->prev and ->next and
             move on */
        else {
            newitem->child = newchild;
            nptr = newchild;
        } /* Set newitem->child and move to it */
        cptr = cptr->next;
    }
    return newitem;
}

void cJSON_Minify(char *json) {
    char *into = json;
    while (*json) {
        if (*json == ' ')
            json++;
        else if (*json == '\t')
            json++; /* Whitespace characters. */
        else if (*json == '\r')
            json++;
        else if (*json == '\n')
            json++;
        else if (*json == '/' && json[1] == '/')
            while (*json && *json != '\n') json++; /* double-slash comments, to end of line. */
        else if (*json == '/' && json[1] == '*') {
            while (*json && !(*json == '*' && json[1] == '/')) json++;
            json += 2;
        } /* multiline comments. */
        else if (*json == '\"') {
            *into++ = *json++;
            while (*json && *json != '\"') {
                if (*json == '\\') *into++ = *json++;
                *into++ = *json++;
            }
            *into++ = *json++;
        } /* string literals, which are \" sensitive. */
        else
            *into++ = *json++; /* All other characters. */
    }
    *into = 0; /* and null-terminate. */
}

VkResult loader_get_json(const struct loader_instance *inst, const char *filename, cJSON **json) {
    FILE *file = NULL;
    char *json_buf = NULL;
    size_t len;
    VkResult res = VK_SUCCESS;

    assert(json != NULL);

    *json = NULL;

#if defined(_WIN32)
    int filename_utf16_size = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
    if (filename_utf16_size > 0) {
        wchar_t *filename_utf16 = (wchar_t *)loader_stack_alloc(filename_utf16_size * sizeof(wchar_t));
        if (MultiByteToWideChar(CP_UTF8, 0, filename, -1, filename_utf16, filename_utf16_size) == filename_utf16_size) {
            file = _wfopen(filename_utf16, L"rb");
        }
    }
#elif COMMON_UNIX_PLATFORMS
    file = fopen(filename, "rb");
#else
#warning fopen not available on this platform
#endif

    if (!file) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0, "loader_get_json: Failed to open JSON file %s", filename);
        res = VK_ERROR_INITIALIZATION_FAILED;
        goto out;
    }
    // NOTE: We can't just use fseek(file, 0, SEEK_END) because that isn't guaranteed to be supported on all systems
    size_t fread_ret_count = 0;
    do {
        char buffer[256];
        fread_ret_count = fread(buffer, 1, 256, file);
    } while (fread_ret_count == 256 && !feof(file));
    len = ftell(file);
    fseek(file, 0, SEEK_SET);
    json_buf = (char *)loader_instance_heap_calloc(inst, len + 1, VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
    if (json_buf == NULL) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0,
                   "loader_get_json: Failed to allocate space for JSON file %s buffer of length %d", filename, len);
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }
    if (fread(json_buf, sizeof(char), len, file) != len) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0, "loader_get_json: Failed to read JSON file %s.", filename);
        res = VK_ERROR_INITIALIZATION_FAILED;
        goto out;
    }
    json_buf[len] = '\0';

    // Can't be a valid json if the string is of length zero
    if (len == 0) {
        res = VK_ERROR_INITIALIZATION_FAILED;
        goto out;
    }
    // Parse text from file
    *json = cJSON_Parse(inst ? &inst->alloc_callbacks : NULL, json_buf);
    if (*json == NULL) {
        loader_log(inst, VULKAN_LOADER_ERROR_BIT, 0,
                   "loader_get_json: Failed to parse JSON file %s, this is usually because something ran out of memory.", filename);
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }

out:
    loader_instance_heap_free(inst, json_buf);
    if (NULL != file) {
        fclose(file);
    }
    if (res != VK_SUCCESS && *json != NULL) {
        cJSON_Delete(*json);
        *json = NULL;
    }

    return res;
}

VkResult loader_parse_json_string_to_existing_str(const struct loader_instance *inst, cJSON *object, const char *key,
                                                  size_t out_str_len, char *out_string) {
    cJSON *item = cJSON_GetObjectItem(object, key);
    if (NULL == item) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    char *str = cJSON_Print(item);
    if (str == NULL) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    if (NULL != out_string) {
        strncpy(out_string, str, out_str_len);
        if (out_str_len > 0) {
            out_string[out_str_len - 1] = '\0';
        }
    }
    loader_instance_heap_free(inst, str);
    return VK_SUCCESS;
}

VkResult loader_parse_json_string(cJSON *object, const char *key, char **out_string) {
    cJSON *item = cJSON_GetObjectItem(object, key);
    if (NULL == item) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    char *str = cJSON_Print(item);
    if (str == NULL) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    if (NULL != out_string) {
        *out_string = str;
    }
    return VK_SUCCESS;
}
VkResult loader_parse_json_array_of_strings(const struct loader_instance *inst, cJSON *object, const char *key,
                                            struct loader_string_list *string_list) {
    VkResult res = VK_SUCCESS;
    cJSON *item = cJSON_GetObjectItem(object, key);
    if (NULL == item) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    uint32_t count = cJSON_GetArraySize(item);
    if (count == 0) {
        return VK_SUCCESS;
    }

    res = create_string_list(inst, count, string_list);
    if (VK_ERROR_OUT_OF_HOST_MEMORY == res) {
        goto out;
    }
    for (uint32_t i = 0; i < count; i++) {
        cJSON *element = cJSON_GetArrayItem(item, i);
        if (element == NULL) {
            return VK_ERROR_INITIALIZATION_FAILED;
        }
        char *out_data = cJSON_Print(element);
        if (out_data == NULL) {
            res = VK_ERROR_OUT_OF_HOST_MEMORY;
            goto out;
        }
        res = append_str_to_string_list(inst, string_list, out_data);
        if (VK_ERROR_OUT_OF_HOST_MEMORY == res) {
            goto out;
        }
    }
out:
    if (res == VK_ERROR_OUT_OF_HOST_MEMORY && NULL != string_list->list) {
        free_string_list(inst, string_list);
    }

    return res;
}
