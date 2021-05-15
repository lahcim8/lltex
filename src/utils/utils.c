/*

Copyright 1996-2006 Han The Thanh <thanh@pdftex.org>
Copyright 2006-2012 Taco Hoekwater <taco@luatex.org>

This file is part of LuaTeX.

LuaTeX is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

LuaTeX is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU General Public License along with
LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

#include "ptexlib.h"

#include <errno.h>

/*tex

    This is a trick to load mingw32's io.h early, using a macro redefinition of
    |eof()|.

*/

#include "sys/types.h"
#include <string.h>
#include <time.h>

/*tex For |DBL_EPSILON|: */

#include <float.h>

#include "lua/luatex-api.h"
#include "luatex_svnversion.h"



#define check_nprintf(size_get, size_want) \
    if ((unsigned)(size_get) >= (unsigned)(size_want)) \
        formatted_error("internal","snprintf failed: file %s, line %d", __FILE__, __LINE__);

char *cur_file_name = NULL;
static char print_buf[PRINTF_BUF_SIZE];
int epochseconds;
int microseconds;

__attribute__ ((format(printf, 1, 2)))
void tex_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(print_buf, PRINTF_BUF_SIZE, fmt, args);
    tprint(print_buf);
    if (fflush(stdout) != 0)
        formatted_error("file io","fflush() failed (%s)", strerror(errno));
    va_end(args);
}

scaled ext_xn_over_d(scaled x, scaled n, scaled d)
{
    double r = (((double) x) * ((double) n)) / ((double) d);
    if (r > DBL_EPSILON)
        r += 0.5;
    else
        r -= 0.5;
    if (r >= (double) max_integer || r <= -(double) max_integer)
        normal_warning("internal","arithmetic number too big");
    return (scaled) r;
}

/*tex

    The function |divide_scaled| divides |s| by |m| using |dd| decimal digits of
    precision.

*/
scaled
round_xn_over_d(scaled x, int n, unsigned int d)
{
    boolean positive = true;
    unsigned t, u, v;
    if (x < 0) {
        positive = !positive;
        x = -(x);
    }
    if (n < 0) {
        positive = !positive;
        n = -(n);
    }
    t = (unsigned) ((x % 0100000) * n);
    u = (unsigned) (((unsigned) (x) / 0100000) * (unsigned) n + (t / 0100000));
    v = (u % d) * 0100000 + (t % 0100000);
    if (u / d >= 0100000)
        arith_error = true;
    else
        u = 0100000 * (u / d) + (v / d);
    v = v % d;
    if (2 * v >= d)
        u++;
    if (positive)
        return (scaled) u;
    else
        return (-(scaled) u);
}

/*tex scaled value corresponds to 1truein (rounded!) */

scaled one_true_inch = (7227 * 65536 + 50) / 100;

/*tex scaled value corresponds to 1in (rounded to 4736287) */

scaled one_inch = (7227 * 65536 + 50) / 100;

/*tex

    This function strips trailing zeros in string with numbers; leading zeros are
    not stripped (as in real life), It's not used.

*/

#if 0
char *stripzeros(char *a)
{
    enum { NONUM, DOTNONUM, INT, DOT, LEADDOT, FRAC } s = NONUM, t = NONUM;
    char *p, *q, *r;
    for (p = q = r = a; *p != '\0';) {
        switch (s) {
        case NONUM:
            if (*p >= '0' && *p <= '9')
                s = INT;
            else if (*p == '.')
                s = LEADDOT;
            break;
        case DOTNONUM:
            if (*p != '.' && (*p < '0' || *p > '9'))
                s = NONUM;
            break;
        case INT:
            if (*p == '.')
                s = DOT;
            else if (*p < '0' || *p > '9')
                s = NONUM;
            break;
        case DOT:
        case LEADDOT:
            if (*p >= '0' && *p <= '9')
                s = FRAC;
            else if (*p == '.')
                s = DOTNONUM;
            else
                s = NONUM;
            break;
        case FRAC:
            if (*p == '.')
                s = DOTNONUM;
            else if (*p < '0' || *p > '9')
                s = NONUM;
            break;
        default:;
        }
        switch (s) {
        case DOT:
            r = q;
            break;
        case LEADDOT:
            r = q + 1;
            break;
        case FRAC:
            if (*p > '0')
                r = q + 1;
            break;
        case NONUM:
            if ((t == FRAC || t == DOT) && r != a) {
                q = r--;
                if (*r == '.')  /* was a LEADDOT */
                    *r = '0';
                r = a;
            }
            break;
        default:;
        }
        *q++ = *p++;
        t = s;
    }
    *q = '\0';
    return a;
}
#endif

void initversionstring(char **versions)
{

#define LUA_VER_STRING  "lua version " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR "." LUA_VERSION_RELEASE
#define STR(tok) STR2(tok)
#define STR2(tok) #tok

    const_string fmt =
        "Compiled with %s\n" /* Lua */
        "Compiled with zlib %s; using %s\n"
        "\nDevelopment id: %s\n";
    size_t len = strlen(fmt)
               + strlen(LUA_VER_STRING)
               + strlen(STR(luatex_svn_revision))
               + 1;

    /*tex
        The size of |len| will be more than enough, because of the placeholder
        chars in fmt that get replaced by the arguments.
    */
    *versions = xmalloc(len);
    sprintf(*versions, fmt,
                    LUA_VER_STRING,
                    STR(luatex_svn_revision));

#undef STR2
#undef STR
#undef LUA_VER_STRING

}

void check_buffer_overflow(int wsize)
{
    if (wsize > buf_size) {
        int nsize = buf_size + buf_size / 5 + 5;
        if (nsize < wsize) {
            nsize = wsize + 5;
        }
        buffer = (unsigned char *) xreallocarray(buffer, char, (unsigned) nsize);
        buf_size = nsize;
    }
}

/*tex

    The return value is a decimal number with the point |dd| places from the
    back, |scaled_out| is the number of scaled points corresponding to that.

*/

#define max_integer 0x7FFFFFFF

scaled divide_scaled(scaled s, scaled m, int dd)
{
    register scaled q;
    register scaled r;
    int i;
    int sign = 1;
    if (s < 0) {
        sign = -sign;
        s = -s;
    }
    if (m < 0) {
        sign = -sign;
        m = -m;
    }
    if (m == 0) {
        normal_error("arithmetic", "divided by zero");
    } else if (m >= (max_integer / 10)) {
        normal_error("arithmetic", "number too big");
    }
    q = s / m;
    r = s % m;
    for (i = 1; i <= (int) dd; i++) {
        q = 10 * q + (10 * r) / m;
        r = (10 * r) % m;
    }
    /*tex Rounding: */
    if (2 * r >= m) {
        q++;
    }
    return sign * q;
}

#ifdef _WIN32
#undef floor
#define floor win32_floor
#endif

/*tex

    The same function, but using doubles instead of integers (faster).

*/

scaled divide_scaled_n(double sd, double md, double n)
{
    double dd, di = 0.0;
    dd = sd / md * n;
    if (dd > 0.0)
        di = floor(dd + 0.5);
    else if (dd < 0.0)
        di = -floor((-dd) + 0.5);
    return (scaled) di;
}

int do_zround(double r)
{
    int i;
    if (r > 2147483647.0)
        i = 2147483647;
    else if (r < -2147483647.0)
        i = -2147483647;
    else if (r >= 0.0)
        i = (int) (r + 0.5);
    else
        i = (int) (r - 0.5);
    return i;
}


/*tex

    Old MSVC doesn't have |rint|.

*/

#if defined(_MSC_VER) && _MSC_VER <= 1600

#  include <math.h>

double rint(double x)
{
    return floor(x+0.5);
}

#endif
