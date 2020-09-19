# This script parses TL/tlpkg/tlpsrc/*.tlpsrc files from https://github.com/hyphenation/tex-hyphen.
# It creates two files:
# 1) language.def, which is used at etex format creation to register languages.
# 2) language.dat.lua, which is used at runtime by luatex based formats (through
#    luatex-hyphen.lua). That file contains names of files containing hyphenation
#    and exception patterns.

# Usage: awk -f extract-language-data.awk tex-hyphen/TL/tlpkg/tlpsrc/*.tlpsrc

# Michal Vlasák, 2020, public domain

BEGIN {
	FS = "[= ]"

	DEF = "language.def"
	DATLUA = "language.dat.lua"

	print "%% e-TeX V2.1;0-luatex2" > DEF
	print "\\addlanguage{USenglish}{hyphen}{}{2}{3}" > DEF

	print "return {" > DATLUA
}

END {
	print "\\uselanguage{USenglish}" > DEF

	print "}" > DATLUA
}

in_addhyphen {
	gsub(/^[[:space:]]+|[[:space:]]+\\$/,"")

	if ($1 == "runpattern" || $1 == "execute") {
		if (!disabled) {
			printf "\\addlanguage{%s}{}{}{%s}{%s}\n", map["name"], map["lefthyphenmin"], map["righthyphenmin"] > DEF

			# "synonyms" functionality seems to be broken, because language.dat format
			# doesn't support it (see function TexLive::TLPOBJ::language_def_lines),
			# but all language need to by registered by \addlanguage.
			# Set "synonyms" to dummy value, so luatex-hyphen works.
			printf "\
	['%s'] = {\n\
		synonyms = {},\n\
		patterns = '%s',\n\
		hyphenation = '%s',\n\
	},\n", map["name"], map["file_patterns"], map["file_exceptions"] > DATLUA
		}

		in_addhyphen = $1 == "execute"
	} else {
		if ($1 == "luaspecial" && $2 ~ /^"disabled:/) {
			disabled = 1
		} else if (($1 == "lefthyphenmin" || $1 == "righthyphenmin") && $2 == "") {
			# no value provided, keep default value
		} else {
			map[$1] = $2
		}
	}
}

$1 == "execute" && $2 == "AddHyphen" {
	in_addhyphen = 1
	delete map
	map["lefthyphenmin"] = 2
	map["righthyphenmin"] = 3
	disabled = 0
}
