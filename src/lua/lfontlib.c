/* lfontlib.c

   Copyright 2006-2014 Taco Hoekwater <taco@luatex.org>

   This file is part of LuaTeX.

   LuaTeX is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   LuaTeX is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   License for more details.

   You should have received a copy of the GNU General Public License along
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */


#include "ptexlib.h"
#include "lua/luatex-api.h"

#define TIMERS 0

#if TIMERS
#  include <sys/time.h>
#endif

static int get_fontid(void)
{
    if (font_tables == NULL || font_tables[0] == NULL) {
        create_null_font();
    }
    return new_font();
}

static int tex_current_font(lua_State * L)
{
    int i = luaL_optinteger(L, 1, 0);
    if (i > 0) {
        if (is_valid_font(i)) {
            zset_cur_font(i);
            return 0;
        } else {
            luaL_error(L, "expected a valid font id");
            return 2;           /* not reached */
        }
    } else {
        lua_pushinteger(L, get_cur_font());
        return 1;
    }
}

static int tex_max_font(lua_State * L)
{
    lua_pushinteger(L, max_font_id());
    return 1;
}


static int tex_each_font_next(lua_State * L)
{
    int m = lua_tointeger(L, 1);
    int i = lua_tointeger(L, 2);
    i++;
    while (i <= m && !is_valid_font(i))
        i++;
    if (i > m) {
        lua_pushnil(L);
        return 1;
    } else {
        lua_pushinteger(L, i);
        if (!font_to_lua(L, i, 1))
            lua_pushnil(L);
        return 2;
    }
}

static int tex_each_font(lua_State * L)
{
    lua_pushcclosure(L, tex_each_font_next, 0);
    lua_pushinteger(L, max_font_id());
    lua_pushinteger(L, 0);
    return 3;
}

static int frozenfont(lua_State * L)
{
    int i = luaL_checkinteger(L, 1);
    if (i) {
        if (is_valid_font(i)) {
            if (font_touched(i) || font_used(i)) {
                lua_pushboolean(L, 1);
            } else {
                lua_pushboolean(L, 0);
            }
        } else {
            lua_pushnil(L);
        }
        return 1;
    } else {
        luaL_error(L, "expected an integer argument");
    }
    return 0;                   /* not reached */
}


static int setfont(lua_State * L)
{
    int t = lua_gettop(L);
    int i = luaL_checkinteger(L,1);
    if (i) {
        luaL_checktype(L, t, LUA_TTABLE);
        if (is_valid_font(i)) {
            if (! (font_touched(i) || font_used(i))) {
                font_from_lua(L, i);
            } else {
                luaL_error(L, "that font has been accessed already, changing it is forbidden");
            }
        } else {
            luaL_error(L, "that integer id is not a valid font");
        }
    }
    return 0;
}

static int addcharacters(lua_State * L)
{
    int t = lua_gettop(L);
    int i = luaL_checkinteger(L,1);
    if (i) {
        luaL_checktype(L, t, LUA_TTABLE);
        if (is_valid_font(i)) {
            characters_from_lua(L, i);
        } else {
            luaL_error(L, "that integer id is not a valid font");
        }
    }
    return 0;
}

static int setexpansion(lua_State * L)
{
    int f = luaL_checkinteger(L,1);
    if (f) {
        if (is_valid_font(f)) {
            int fstretch = luaL_checkinteger(L,2);
            int fshrink = luaL_checkinteger(L,3);
            int fstep = luaL_checkinteger(L,4);
            set_expand_params(f, fstretch, fshrink, fstep);
        } else {
            luaL_error(L, "that integer id is not a valid font");
        }
    }
    return 0;
}

/* font.define(id,table) */
/* font.define(table) */

static int deffont(lua_State * L)
{
    int i = 0;
    int t = lua_gettop(L);
    if (t == 2) {
        i = lua_tointeger(L,1);
        if ((i <= 0) || ! is_valid_font(i)) {
            lua_pop(L, 1);          /* pop the broken table */
            luaL_error(L, "font creation failed, invalid id passed");
        }
    } else if (t == 1) {
        i = get_fontid();
    } else {
        luaL_error(L, "font creation failed, no table passed");
        return 0;
    }
    luaL_checktype(L, -1, LUA_TTABLE);
    if (font_from_lua(L, i)) {
        lua_pushinteger(L, i);
        return 1;
    } else {
        lua_pop(L, 1);          /* pop the broken table */
        delete_font(i);
        luaL_error(L, "font creation failed, error in table");
    }
    return 0;                   /* not reached */
}

/* this returns the expected (!) next fontid. */
/* first arg true will keep the id */

static int nextfontid(lua_State * L)
{
    int b = ((lua_gettop(L) == 1) && lua_toboolean(L,1));
    int i = get_fontid();
    lua_pushinteger(L, i);
    if (b == 0) {
        delete_font(i);
    }
    return 1;
}

static int getfont(lua_State * L)
{
    int i = luaL_checkinteger(L, -1);
    if (i && is_valid_font(i) && font_to_lua(L, i, 1))
        return 1;
    lua_pushnil(L);
    return 1;
}

static int getcopy(lua_State * L)
{
    int i = luaL_checkinteger(L, -1);
    if (i && is_valid_font(i) && font_to_lua(L, i, 0))
        return 1;
    lua_pushnil(L);
    return 1;
}

static int getparameters(lua_State * L)
{
    int i = luaL_checkinteger(L, -1);
    if (i && is_valid_font(i)) {
        return font_parameters_to_lua(L,i);
    }
    return 0;
}

static int getfontid(lua_State * L)
{
    if (lua_type(L, 1) == LUA_TSTRING) {
        size_t ff;
        const char *s = lua_tolstring(L, 1, &ff);
        int cs = string_lookup(s, ff);
        int f;
        if (cs == undefined_control_sequence || cs == undefined_cs_cmd || eq_type(cs) != set_font_cmd) {
            lua_pushstring(L, "not a valid font csname");
            f = -1;
        } else {
            f = equiv(cs);
        }
        lua_pushinteger(L, f);
    } else {
        luaL_error(L, "expected font csname string as argument");
    }
    return 1;
}

static const struct luaL_Reg fontlib[] = {
    {"current", tex_current_font},
    {"max", tex_max_font},
    {"each", tex_each_font},
    {"getfont", getfont},
    {"getcopy", getcopy},
    {"getparameters", getparameters},
    {"setfont", setfont},
    {"addcharacters", addcharacters},
    {"setexpansion", setexpansion},
    {"define", deffont},
    {"nextid", nextfontid},
    {"id", getfontid},
    {"frozen", frozenfont},
    {NULL, NULL}                /* sentinel */
};

int luaopen_font(lua_State * L)
{
    luaL_newlib(L, fontlib);
    make_table(L, "fonts", "tex.fonts", "getfont", "setfont");
    return 1;
}
