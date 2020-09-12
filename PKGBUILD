pkgname=lltex
pkgver=0.1.1
pkgrel=1
pkgdesc='A leightweight LuaTeX distribution'
arch=('x86_64')
url='https://gitlab.com/lahcim8/lltex'
_ctan="http://mirrors.ctan.org"
_texlive="svn://tug.org/texlive"
_github="git+https://github.com"
source=("$_github/TeX-Live/luatex#branch=tags/1.12.0" # luatex source code

        "$_github/hyphenation/tex-hyphen#tag=CTAN-2020.03.25" # hyph-utf8, etex.src
        "$_ctan/obsolete/systems/e-tex/v2/src/etexdefs.lib" # etexdefs.lib
        "$_ctan/macros/generic/unicode-data.zip" # unicode-data

        "$_ctan/systems/knuth/dist/lib.zip" # plain.tex, hyphen.tex
        "$_ctan/fonts/cm/mf/cmbase.mf" # cmbase for generating rsfs .tfm files
        "$_ctan/fonts/amsfonts.zip" # computer modern .pfb files
        "$_ctan/fonts/cm/tfm.zip" # computer modern .tfm files
        "$_ctan/fonts/manual.zip" # manfnt font
        "$_ctan/fonts/ec.zip" # ecso, ecsx .tfm files
        "$_ctan/fonts/lm.zip" # ec-lm .pfb files
        "$_ctan/fonts/rsfs.zip" # rsfs .pfb files
        "$_ctan/fonts/ps-type1/cm-super.zip" # ecso, ecsx .pfb files
        "$_ctan/macros/cstex/base/cmexb.tar.gz" # cmexb font

        "$_ctan/macros/luatex/generic/lualibs.zip" # lualibs
        "$_ctan/macros/latex/base.zip" # ltluatex + docstrip
        "$_ctan/macros/latex/contrib/l3kernel.zip" # l3docstrip
        "$_ctan/macros/luatex/generic/luaotfload.zip" # luaotfload
        "$_ctan/macros/luatex/generic/luamplib.zip" # luamplib

        # metapost base files
        # "https://ctan.org/tex-archive/obsolete/graphics/metapost/base/texmf/metapost/base/"
        # "svn+https://serveur-svn.lri.fr/svn/modhel/metapost/trunk/texmf/metapost/base"
        "$_texlive/trunk/Master/texmf-dist/metapost/base/"

        "$_ctan/macros/generic/tex-ini-files.zip" # luatex.ini, etc

        "$_ctan/macros/luatex/optex.zip" # optex
        "$_ctan/macros/latex/contrib/lipsum.zip" # optex needs lipsum.ltd.tex

        "texmf.cnf" # custom kpathsea config file
        "pdftex.map" # mapping of used type1 fonts
        "luatexiniconfig.tex" # config for plain luatex format
        "mfplain.lua" # for creating .tfm from .mf using mplib
        "extract-language-data.awk") # creates language.def, language.dat.lua

sha256sums=('SKIP'
            'SKIP'
            '09af6198ac22a824b63ea4176fe584ef62802c1c7fb87f585fa97266d4142f03'
            'd38bc164d143a7d3cc1b99482509d7debaa7d2b12c8d652868264275014be5bd'
            'f697b665c835d4888b2417edbbf1c23322c722fc8163b1a231916f4e40dcd8fe'
            '607886890c697d0d1a827a3e329efcf56bb71e2ae4df6e2699ebc105357edf81'
            'aaab3930b952128b7ed65260144493eb3d88cb063b50da1f90272e23c7e08ba2'
            '2697459803c68db061217273ac7386efbfa17ed362aa3c7eafa24fb86cf1b4e2'
            '309e92d8ea932115b5775a418f0ef4ae373d088eefd70cf25d58757647c9901b'
            '364ea6dc4c05ca49833c31f8bb510bd7cd94142e8e934c59df48a950695c9ed4'
            '298744467b504c13a542ccfa0014f05e60367d0a84b4963f51036081980f4bd0'
            'd57d16ff8b438fad7212a822a394e099b427d1fb8434fd1f78decb09a4c84aaa'
            'f3f856a5006ae7d2bbc2d9a338ca5ae25b6fba19eebee3ba20892845aafa7987'
            '33f462ecbb4688a5015a7cd507918e96ab933059c9a40fb875a11819596f68d1'
            '6ad2cc39615dc090436e948e8a5c75adaedc422f30d62b2098d4f51d95cad097'
            'e4dfb17b355d8b7e653e9139ce7f65480e309b95fb2c04714df24d146c380a55'
            '65cba6eddb98e8b772f4f962f970535d3c22e1ea4304a8655966595e3fddaa30'
            'a599f5fa74fc3b68db2209da55d5232738751faf83e8cfc7e04a5278f9789104'
            '670c803c98eca5784e3cbba195d2b9ecef12b2ae1cd8f74b2e5e6408aa679c6a'
            'SKIP'
            'f0d1846073e7c8043750461af379a81e6498ebd38968f3fc5a8a996c61054fec'
            'd616171229a90dcfbf702177537105241813f0bf2cbbc3c0d7da4476a65a74bb'
            'c96aa4aa52a922ed7337794eb95093ae0726a825e430d870a4371fad3d479137'
            '1b72e8506889f43ea2a3b9ef57d3ceaa812d9c6c2c90dfd92823efddf400b9a8'
            'b8b107aae1eb98c0708f79dc72ffd6b78e33ffe9f0574ff661bc2857ed0f80d1'
            '9340df8c64b917773d2da54e7a24f5b0e9418834326c0df7b851e945859cbef3'
            '91fe52db40bb3fefadcc0e0ed6a6cdaa4015f1232a792296f404c2a1f235e9eb'
            '8e64c7f8963355ec0bd2744bed62890cec8f1e7e8728a6a78dc25f83ddbd8dfe')

build() {
    cd luatex

    # set our own TEXMFCNF value
    sed -i '1i TEXMFCNF={$XDG_CONFIG_HOME,~/.config,/etc}/lltex:$SELFAUTODIR/share/lltex/web2c' source/texk/kpathsea/texmf.cnf

    # build luatex binary
    ./build.sh --tlopt=--with-banner-add=/lltex
    cd -
}

package() {
    texmf="$pkgdir/usr/share/$pkgname"
    export TEXMF="$texmf"
    export TEXMFCNF=.

    # first install luatex binary as we will need it later and add it to PATH
    install -Dm755 luatex/build/texk/web2c/luatex "$pkgdir/usr/bin/luatex"
    PATH="$pkgdir/usr/bin:$PATH"

    # generate lipsum.ltd.tex (needs docstrip and l3docstrip)
    env TEXMFDOTDIR=.:base luatex -ini docstrip.ins # create docstrip.tex
    env TEXMFDOTDIR=.:l3kernel luatex -ini l3.ins # create l3docstrip.tex
    env TEXMFDOTDIR=.:lipsum luatex -ini '\let\obeyspaces=\relax \input lipsum.ins'

    # generate ltluatex.{lua,tex}
    env TEXMFDOTDIR=.:base luatex -ini format.ins

    # generate luamplib.{sty.lua}
    env TEXMFDOTDIR=.:luamplib luatex -ini '\catcode`\{=1 \catcode`\}=2 \let\obeyspaces\relax \input luamplib.dtx'

    # generate some lualibs files
    env TEXMFDOTDIR=.:lualibs luatex -ini '\catcode`\{=1 \catcode`\}=2 \let\obeyspaces\relax \input lualibs.dtx'

    # install luatex packages needed by both plain luatex and optex
    install -Dm644 tex-hyphen/hyph-utf8/tex/generic/hyph-utf8/patterns/txt/* -t "$texmf/tex/hyph-utf8"
    install -Dm644 unicode-data/{CaseFolding.txt,load-unicode-data.tex,PropList.txt,ScriptExtensions.txt,Scripts.txt,UnicodeData.txt} -t "$texmf/tex/unicode-data"
    install -Dm644 {.,lualibs}/lualibs*.lua -t "$texmf/tex/lualibs"
    install -Dm644 ltluatex.{lua,tex} -t "$texmf/tex/latex"
    install -Dm644 luaotfload/*.{lua,sty} -t "$texmf/tex/luaotfload"
    install -Dm644 luamplib.{lua,sty} -t "$texmf/tex/luamplib"
    install -Dm644 base/*.mp -t "$texmf/metapost"
    install -Dm644 tex-hyphen/hyph-utf8/tex/luatex/hyph-utf8/luatex-hyphen.lua -t "$texmf/tex/hyph-utf8"

    # install pdftex.map and needed .enc files
    install -Dm644 pdftex.map -t "$texmf/fonts/map"
    install -Dm644 lm/fonts/enc/dvips/lm/lm-ec.enc -t "$texmf/fonts/enc"
    install -Dm644 cm-super/dvips/cm-super-t1.enc -t "$texmf/fonts/enc"

    # extract language information to language.def and language.dat.lua
    awk -f extract-language-data.awk tex-hyphen/TL/tlpkg/tlpsrc/*.tlpsrc
    # language.dat.lua is needed at run time, language.def is needed only by
    # etex.src at luatex plain format creation
    install -Dm644 language.dat.lua -t "$texmf/tex/lltex"

    # generate rsfs*.tfm files
    env TEXMFDOTDIR=".:rsfs" luatex --luaonly mfplain.lua rsfs{5,7,10}

    # there are two manfnt.tfm files, one from cm-tfm package and one from manual package
    # use the one from manual package
    rm -f tfm/manfnt.tfm
    # delete comments and empty lines from .map file, prepare .tfm, .pfb file names
    sed -e 's/%.*//' -e '/^$/d' pdftex.map > pdftex.map.tmp
    awk '{print "/" $1 ".tfm$"}' pdftex.map.tmp > tfm_files
    awk -F'[[:blank:]]|<' '{print "/" $NF "$"}' pdftex.map.tmp > pfb_files
    # find all possible font files
    find -type f -regex '.*\.\(tfm\|pfb\)$' > available_files
    # install only needed font files
    grep -f tfm_files available_files | xargs install -Dm644 -t "$texmf/fonts/tfm"
    grep -f pfb_files available_files | xargs install -Dm644 -t "$texmf/fonts/type1"

    # need etex.src from depths of hyph-utf8
    cp tex-hyphen/hyph-utf8/tex/luatex/hyph-utf8/etex.src .

    # install optex files
    install -Dm644 optex/{base,pkg}/*.opm -t "$texmf/tex/optex"
    install -Dm644 lipsum.ltd.tex -t "$texmf/tex/lipsum"

    # run iniluatex to create format files
    env TEXMFDOTDIR=".:tex-ini-files:lib" luatex -ini tex-ini-files/luatex.ini
    env TEXMFDOTDIR=".:lib" luatex -ini optex/base/optex.ini

    # install format files
    install -Dm644 *.fmt -t "$texmf/web2c"

    # create optex -> luatex symlink
    ln -s luatex "$pkgdir/usr/bin/optex"

    # install configuration file
    install -Dm644 texmf.cnf "$texmf/web2c/texmf.cnf"
}
