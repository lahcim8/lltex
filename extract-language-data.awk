# This script parses TL/tlpkg/tlpsrc/*.tlpsrc files from https://github.com/hyphenation/tex-hyphen.
# It creates two files:
# 1) language.def, which is used at etex format creation to register languages.
# 2) language.dat.lua, which is used at runtime by luatex based formats (through
#    luatex-hyphen.lua). That file contains names of files containing hyphenation
#    and exception patterns.

# Usage: awk -f extract-language-data.awk tex-hyphen/TL/tlpkg/tlpsrc/*.tlpsrc

BEGIN {
	FS = "[= ]"

	DEF = "mmtex-plain/files/language.def"
	DATLUA = "mmtex-base/files/language.dat.lua"

	print "%% e-TeX V2.1;0-luatex2" > DEF
	print "\\addlanguage{USenglish}{hyphen}{}{2}{3}" > DEF
	# these "synonyms" don't really work with Plain LuaTeX, due to weird
	# handling between luatex-hyphen and etex.src.
	print "%\\addlanguage{english}{}{}{2}{3}" > DEF
	print "%\\addlanguage{usenglish}{}{}{2}{3}" > DEF
	print "%\\addlanguage{american}{}{}{2}{3}" > DEF

	print "return {\n\
	['USenglish'] = {\n\
		synonyms = { 'english', 'usenglish', 'american', },\n\
		special = 'language0',\n\
	}," > DATLUA
}

END {
	print "\\uselanguage{USenglish}" > DEF

	print "}" > DATLUA
}

in_addhyphen {
	gsub(/^[[:space:]]+|[[:space:]]+\\$/,"")

	if ($1 == "runpattern" || $1 == "execute") {
		if (!disabled) {
			printf "\\addlanguage{%s}{}{}{%s}{%s}\n", lang["name"], lang["lefthyphenmin"], lang["righthyphenmin"] > DEF

			syn_list = " "
			for (i in synonyms) {
				syn_list = sprintf("%s'%s', ", syn_list, synonyms[i])
				printf "\\addlanguage{%s}{}{}{%s}{%s}\n", synonyms[i], lang["lefthyphenmin"], lang["righthyphenmin"] > DEF
			}

			printf "\
	['%s'] = {\n\
		synonyms = {%s},\n\
		patterns = '%s',\n\
		hyphenation = '%s',\n\
	},\n", lang["name"], syn_list, lang["file_patterns"], lang["file_exceptions"] > DATLUA
		}

		in_addhyphen = $1 == "execute"
	} else {
		if ($1 == "luaspecial" && $2 ~ /^"disabled:/) {
			disabled = 1
		} else if (($1 == "lefthyphenmin" || $1 == "righthyphenmin") && $2 == "") {
			# no value provided, keep default value
		} else {
			lang[$1] = $2
			if ($3 == "synonyms") {
				split($4, synonyms, ",")
			}
		}
	}
}

$1 == "execute" && $2 == "AddHyphen" {
	in_addhyphen = 1
	delete lang
	delete synonyms
	lang["lefthyphenmin"] = 2
	lang["righthyphenmin"] = 3
	disabled = 0
}
