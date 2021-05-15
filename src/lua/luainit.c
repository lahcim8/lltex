/*

luainit.w

Copyright 2006-2021 Taco Hoekwater <taco@@luatex.org>

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
with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

#include "ptexlib.h"

#include "lua/luatex-api.h"

#include <locale.h>

#include <sys/stat.h>

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

/*tex internalized strings: see luatex-api.h */

set_make_keys;

const_string LUATEX_IHELP[] = {
    "Usage: " my_name " --lua=FILE [OPTION]... [TEXNAME[.tex]] [COMMANDS]",
    "   or: " my_name " --lua=FILE [OPTION]... \\FIRST-LINE",
    "   or: " my_name " --lua=FILE [OPTION]... &FMT ARGS",
    "  Run " MyName " on TEXNAME, usually creating TEXNAME.pdf.",
    "  Any remaining COMMANDS are processed as luatex input, after TEXNAME is read.",
    "",
    "  Alternatively, if the first non-option argument begins with a backslash,",
    "  " my_name " interprets all non-option arguments as an input line.",
    "",
    "  Alternatively, if the first non-option argument begins with a &, the",
    "  next word is taken as the FMT to read, overriding all else.  Any",
    "  remaining arguments are processed as above.",
    "",
    "  If no arguments or options are specified, prompt for input.",
    "",
    "  The following regular options are understood: ",
    "",
    "   --credits                     display credits and exit",
    "   --[no-]file-line-error        disable/enable file:line:error style messages",
    "   --[no-]file-line-error-style  aliases of --[no-]file-line-error",
    "   --fmt=FORMAT                  load the format file FORMAT",
    "   --halt-on-error               stop processing at the first error",
    "   --help                        display help and exit",
    "   --ini                         be ini" my_name ", for dumping formats",
    "   --interaction=STRING          set interaction mode (STRING=batchmode/nonstopmode/scrollmode/errorstopmode)",
    "   --jobname=STRING              set the job name to STRING",
    "   --lua=FILE                    load and execute a lua initialization script",
    "   --[no-]shell-escape           disable/enable system commands",
    "   --utc                         init time to UTC",
    "   --version                     display version and exit",
    "",
    "Alternate behaviour models can be obtained by special switches",
    "",
    "  --luaonly                      run a lua file, then exit",
    "  --luaconly                     byte-compile a lua file, then exit",
    "  --luahashchars                 the bits used by current Lua interpreter for strings hashing",
    "",
    "See the reference manual for more information about the startup process.",
    NULL
};

/*tex

Later we will put on environment |LC_CTYPE|, |LC_COLLATE| and |LC_NUMERIC| set to
|C|, so we need a place where to store the old values.

*/

const char *lc_ctype;
const char *lc_collate;
const char *lc_numeric;

static void prepare_cmdline(lua_State * L, char **av, int ac, int zero_offset)
{
    int i;
    luaL_checkstack(L, ac + 3, "too many arguments to script");
    lua_createtable(L, 0, 0);
    for (i = 0; i < ac; i++) {
        lua_pushstring(L, av[i]);
        lua_rawseti(L, -2, (i - zero_offset));
    }
    lua_setglobal(L, "arg");
    lua_getglobal(L, "os");
    // TODO(mvlasak): absolute path to self directory?
        //s = ex_selfdir(argv[0]);
        //lua_pushstring(L, s);
        //xfree(s);
    lua_pushstring(L, argv[0]);
    lua_setfield(L, -2, "selfdir");
    return;
}


char *startup_filename = NULL;
int lua_only = 0;
int lua_offset = 0;
unsigned char show_luahashchars = 0;

int utc_option = 0;

extern int myoptind;

static struct optparse_long longopts[] = {
    {"fmt", 'f', OPTPARSE_REQUIRED},
    {"lua", 'l', OPTPARSE_REQUIRED},
    {"interaction", 'I', OPTPARSE_REQUIRED},
    {"jobname", 'j', OPTPARSE_REQUIRED},

    {"luaonly", 'L', OPTPARSE_NONE},
    {"luahashchars", 'Z', OPTPARSE_NONE},
    {"utc", 'u', OPTPARSE_REQUIRED},
    {"help", 'h', OPTPARSE_NONE},
    {"ini", 'i', OPTPARSE_NONE},
    {"halt-on-error", 'H', OPTPARSE_NONE},
    {"version", 'v', OPTPARSE_NONE},
    {"credits", 'c', OPTPARSE_NONE},
    {"file-line-error-style", 'f', OPTPARSE_NONE},
    {"no-file-line-error-style", 'F', OPTPARSE_NONE},
    {"parse-first-line", 'p', OPTPARSE_NONE},
    {"no-parse-first-line", 'P', OPTPARSE_NONE},

    {0, 0, 0, 0}
};

static void parse_options(int ac, char **av)
{
    /*tex The `getopt' return code. */
    int g;
    int option_index;
    if (strstr(argv[0], "texlua") != NULL) {
        lua_only = 1;
        luainit = 1;
    }

    char *arg;
    int option;
    struct optparse options;
    optparse_init(&options, argv);
    while ((option = optparse_long(&options, longopts, NULL)) != -1) {
        switch (option) {
        case 'f': // --fmt
            dump_name = options.optarg;
            break;
        case 'l': // --lua
            startup_filename = options.optarg;
            lua_offset = (options.optind - 1);
            luainit = 1;
            break;
        case 'I': // --interaction
            if (STREQ(options.optarg, "batchmode")) {
                interactionoption = 0;
            } else if (STREQ(options.optarg, "nonstopmode")) {
                interactionoption = 1;
            } else if (STREQ(options.optarg, "scrollmode")) {
                interactionoption = 2;
            } else if (STREQ(options.optarg, "errorstopmode")) {
                interactionoption = 3;
            } else {
                WARNING1("Ignoring unknown argument `%s' to --interaction", options.optarg);
            }
            break;
        case 'j': // --jobname
            c_job_name = options.optarg;
            break;

        case 'L': // --luaonly
            lua_offset = options.optind;
            lua_only = 1;
            luainit = 1;
            break;
        case 'Z': // --luahashchars
            show_luahashchars = 1;
            break;
        case 'u': // --utc
            utc_option = 1;
            break;
        case 'h': // --help
            usagehelp(LUATEX_IHELP, BUG_ADDRESS);
            break;
        case 'i': // --ini
            ini_version = 1;
            break;
        case 'H': // --halt-on-error
            haltonerrorp = 1;
            break;
        case 'v': // --version
            print_version_banner();
            /* *INDENT-OFF* */
            puts("\n\nExecute  '" my_name " --credits'  for credits and version details.\n\n"
                 "There is NO warranty. Redistribution of this software is covered by\n"
                 "the terms of the GNU General Public License, version 2 or (at your option)\n"
                 "any later version. For more information about these matters, see the file\n"
                 "named COPYING and the LuaTeX source.\n\n"
                 "LuaTeX is Copyright 2021 Taco Hoekwater and the LuaTeX Team.\n");
            /* *INDENT-ON* */
            uexit(0);
            break;
        case 'c': {// --credits
            //char *versions;
            //initversionstring(&versions);
            //print_version_banner();
            ///* *INDENT-OFF* */
            //puts("\n\nThe LuaTeX team is Hans Hagen, Hartmut Henkel, Taco Hoekwater, Luigi Scarso.\n\n"
            //     MyName " merges and builds upon (parts of) the code from these projects:\n\n"
            //     "tex       : Donald Knuth\n"
            //     "etex      : Peter Breitenlohner, Phil Taylor and friends\n"
            //     "omega     : John Plaice and Yannis Haralambous\n"
            //     "aleph     : Giuseppe Bilotta\n"
            //     "pdftex    : Han The Thanh and friends\n"
            //     "lua       : Roberto Ierusalimschy, Waldemar Celes and Luiz Henrique de Figueiredo\n"
            //);
            ///* *INDENT-ON* */
            //puts(versions);
            uexit(0);
            break;
	}
        case 'e': // --file-line-error-style
            filelineerrorstylep = 1;
            break;
        case 'E': // --no-file-line-error-style
            filelineerrorstylep = -1;
            break;
        case 'p': // --parse-first-line
            parsefirstlinep = 1;
            break;
        case 'P': // --no-parse-first-line
            parsefirstlinep = -1;
            break;
        case '?':
            /*tex Unknown option. */
            fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
            exit(EXIT_FAILURE);
	}
    }
    if (lua_only) {
        if (arg = optparse_arg(&options)) {
            startup_filename = xstrdup(arg);
            lua_offset = options.optind;
        }
    }
    myoptind = options.optind;
}

/*
    Get the job name to be used, which may have been set from the command line.
*/

str_number getjobname(str_number name)
{
    str_number ret = name;
    if (c_job_name != NULL)
        ret = maketexstring(c_job_name);
    return ret;
}

static void fix_dumpname(void)
{
    if (!dump_name && !ini_version) {
        /*tex For |dump_name| to be NULL is a bug. */
        fprintf(stdout, "no format given, quitting\n");
        exit(1);
    }
}

/*tex

    Helper variables for the safe keeping of table ids.

*/

int l_pack_type_index       [PACK_TYPE_SIZE] ;
int l_group_code_index      [GROUP_CODE_SIZE];
int l_local_par_index       [LOCAL_PAR_SIZE];
int l_math_style_name_index [MATH_STYLE_NAME_SIZE];
int l_dir_par_index         [DIR_PAR_SIZE];
int l_dir_text_index_normal [DIR_TEXT_SIZE];
int l_dir_text_index_cancel [DIR_TEXT_SIZE];

void lua_initialize(int ac, char **av)
{
    char *banner;
    size_t len;
    int starttime;
    int utc;
    static char LC_CTYPE_C[] = "LC_CTYPE=C";
    static char LC_COLLATE_C[] = "LC_COLLATE=C";
    static char LC_NUMERIC_C[] = "LC_NUMERIC=C";
    static char engine_luatex[] = "engine=" my_name;
    char *old_locale = NULL;
    char *env_locale = NULL;
    char *tmp = NULL;
    /*tex Save to pass along to topenin. */
    const char *fmt = "This is " MyName ", Version %s" WEB2CVERSION;
    argc = ac;
    argv = av;
    len = strlen(fmt) + strlen(luatex_version_string) ;
    banner = xmalloc(len);
    sprintf(banner, fmt, luatex_version_string);
    luatex_banner = banner;
    /*tex Must be initialized before options are parsed and might get adapted by config table.  */
    interactionoption = 4;
    filelineerrorstylep = false;
    haltonerrorp = false;
    tracefilenames = 1;
    dump_name = NULL;
    /*tex parse commandline */
    parse_options(ac, av);
    /*tex
        Get the current locale (it should be |C|) and save |LC_CTYPE|, |LC_COLLATE|
        and |LC_NUMERIC|. Later |luainterpreter()| will consciously use them.
    */
    old_locale = xstrdup(setlocale (LC_ALL, NULL));
    lc_ctype = NULL;
    lc_collate = NULL;
    lc_numeric = NULL;
    if (old_locale) {
        /*tex
            If |setlocale| fails here, then the state could be compromised, and
            we exit.
        */
        env_locale = setlocale (LC_ALL, "");
        if (!env_locale && !lua_only) {
            fprintf(stderr,"Unable to read environment locale: exit now.\n");
            exit(1);
        }
        tmp = setlocale (LC_CTYPE, NULL);
        if (tmp) {
            lc_ctype = xstrdup(tmp);
        }
        tmp = setlocale (LC_COLLATE, NULL);
        if (tmp) {
            lc_collate = xstrdup(tmp);
        }
        tmp = setlocale (LC_NUMERIC, NULL);
        if (tmp) {
            lc_numeric = xstrdup(tmp);
        }
        /*tex
            Return to the previous locale if possible, otherwise it's a serious
            error and we exit: we can't ensure a 'sane' locale for lua.
        */
        env_locale = setlocale (LC_ALL, old_locale);
        if (!env_locale) {
          fprintf(stderr,"Unable to restore original locale %s: exit now.\n",old_locale);
          exit(1);
        }
        xfree(old_locale);
    } else {
       fprintf(stderr,"Unable to store environment locale.\n");
    }
    /*tex make sure that the locale is 'sane' (for lua) */
    putenv(LC_CTYPE_C);
    putenv(LC_COLLATE_C);
    putenv(LC_NUMERIC_C);
    /*tex this is sometimes needed */
    putenv(engine_luatex);
    luainterpreter();
    /*tex init internalized strings */
    set_init_keys;
    lua_pushstring(Luas,"lua.functions");
    lua_newtable(Luas);
    lua_settable(Luas,LUA_REGISTRYINDEX);
    /*tex here start the key definitions */
    set_l_pack_type_index;
    set_l_group_code_index;
    set_l_local_par_index;
    set_l_math_style_name_index;
    set_l_dir_par_index;
    set_l_dir_text_index;
    l_set_node_data();
    l_set_whatsit_data();
    l_set_token_data();
    /*tex collect arguments */
    prepare_cmdline(Luas, argv, argc, lua_offset);
    /*tex now run the file */
    if (startup_filename != NULL) {
        int tex_table_id = hide_lua_table(Luas, "tex");
        int token_table_id = hide_lua_table(Luas, "token");
        int node_table_id = hide_lua_table(Luas, "node");
        int pdf_table_id = hide_lua_table(Luas, "pdf");
        if (lua_only) {
            /*tex hide the 'tex' and 'pdf' table */
            if (luaL_loadfile(Luas, startup_filename)) {
                fprintf(stdout, "%s\n", lua_tostring(Luas, -1));
                exit(1);
            }
            init_tex_table(Luas);
            if (lua_pcall(Luas, 0, 0, 0)) {
                fprintf(stdout, "%s\n", lua_tostring(Luas, -1));
                lua_traceback(Luas);
             /*tex lua_close(Luas); */
                exit(1);
            } else {
                /*tex lua_close(Luas); */
                exit(0);
            }
        }
        /*tex a normal tex run */
        init_tex_table(Luas);
        unhide_lua_table(Luas, "tex", tex_table_id);
        unhide_lua_table(Luas, "pdf", pdf_table_id);
        unhide_lua_table(Luas, "token", token_table_id);
        unhide_lua_table(Luas, "node", node_table_id);
        if (luaL_loadfile(Luas, startup_filename)) {
            fprintf(stdout, "%s\n", lua_tostring(Luas, -1));
            exit(1);
        }
        if (lua_pcall(Luas, 0, 0, 0)) {
            fprintf(stdout, "%s\n", lua_tostring(Luas, -1));
            lua_traceback(Luas);
            exit(1);
        }
        if (!dump_name) {
            get_lua_string("texconfig", "formatname", &dump_name);
        }
        /*tex |prohibit_file_trace| (boolean) */
        get_lua_boolean("texconfig", "trace_file_names", &tracefilenames);
        /*tex |file_line_error| */
        get_lua_boolean("texconfig", "file_line_error", &filelineerrorstylep);
        /*tex |halt_on_error| */
        get_lua_boolean("texconfig", "halt_on_error", &haltonerrorp);
        /*tex |interactionoption| */
        get_lua_number("texconfig", "interaction", &interactionoption);
        if ((interactionoption < 0) || (interactionoption > 4)) {
            interactionoption = 4;
        }
        /*tex |shell_escape| */
        starttime = -1 ;
        get_lua_number("texconfig", "start_time", &starttime);
        if (starttime < 0) {
            /*tex
                We provide this one for compatibility reasons and therefore also in
                uppercase.
            */
            get_lua_number("texconfig", "SOURCE_DATE_EPOCH", &starttime);
        }
        if (starttime >= 0) {
            set_start_time(starttime);
        }
        utc = -1 ;
        get_lua_boolean("texconfig", "use_utc_time", &utc);
        if (utc >= 0 && utc <= 1) {
            utc_option = utc;
        }
        fix_dumpname();
    } else if (luainit) {
        fprintf(stdout, "No %s file given\n", (lua_only ? "script" : "configuration"));
        exit(1);
    } else {
        fix_dumpname();
    }
}

void check_texconfig_init(void)
{
    if (Luas != NULL) {
        lua_getglobal(Luas, "texconfig");
        if (lua_istable(Luas, -1)) {
            lua_getfield(Luas, -1, "init");
            if (lua_isfunction(Luas, -1)) {
                int i = lua_pcall(Luas, 0, 0, 0);
                if (i != 0) {
                    /*tex
                        We can't be more precise hereas it's called before \TEX\
                        initialization happens.
                    */
                    fprintf(stderr, "This went wrong: %s\n", lua_tostring(Luas, -1));
                    error();
                }
            }
        }
    }
}
