PREFIX?=/usr/local
BSDCONV_PATH?=${PREFIX}
CFLAGS+=-Wall -DPREFIX='"${PREFIX}"' -DBSDCONV_PATH='"${BSDCONV_PATH}"'
SHLIBVER=10

UNAME_S=$(shell uname -s)
ifeq (${UNAME_S}, Darwin)
SHLIBNAME=libbsdconv.so
else
SHLIBNAME=libbsdconv.so.${SHLIBVER}
endif

LIBS?=
ifeq (${UNAME_S}, Linux)
LIBS+=-ldl
endif

TODO_CODECS_BASIC=
TODO_CODECS_BASIC+=from/00
TODO_CODECS_BASIC+=from/ANSI-CONTROL
TODO_CODECS_BASIC+=from/ANY
TODO_CODECS_BASIC+=from/ASCII-NAMED-HTML-ENTITY
TODO_CODECS_BASIC+=from/ASCII-NUMERIC-HTML-ENTITY
TODO_CODECS_BASIC+=from/ASCII
TODO_CODECS_BASIC+=from/BIG5-5C
TODO_CODECS_BASIC+=from/BSDCONV
TODO_CODECS_BASIC+=from/BSDCONV_KEYWORD
TODO_CODECS_BASIC+=from/BSDCONV_LOG
TODO_CODECS_BASIC+=from/BYTE
TODO_CODECS_BASIC+=from/ESCAPE
TODO_CODECS_BASIC+=from/PASS
TODO_CODECS_BASIC+=from/UTF-16BE
TODO_CODECS_BASIC+=from/UTF-16LE
TODO_CODECS_BASIC+=from/UTF-32BE
TODO_CODECS_BASIC+=from/UTF-32LE
TODO_CODECS_BASIC+=from/_CP1251
TODO_CODECS_BASIC+=from/_CP1252
TODO_CODECS_BASIC+=from/_CP1253
TODO_CODECS_BASIC+=from/_CP874
TODO_CODECS_BASIC+=from/_CP949
TODO_CODECS_BASIC+=from/_GB18030
TODO_CODECS_BASIC+=from/_GBK
TODO_CODECS_BASIC+=from/_ISO-8859-1
TODO_CODECS_BASIC+=from/_JIS0212
TODO_CODECS_BASIC+=from/_SHIFT-JIS
TODO_CODECS_BASIC+=from/_UAO250
TODO_CODECS_BASIC+=from/_UTF-8
TODO_CODECS_BASIC+=inter/AMBIGUOUS-PAD
TODO_CODECS_BASIC+=inter/AMBIGUOUS-UNPAD
TODO_CODECS_BASIC+=inter/BIG5-DEFRAG
TODO_CODECS_BASIC+=inter/FROM_ALIAS
TODO_CODECS_BASIC+=inter/ERROR
TODO_CODECS_BASIC+=inter/FULL
TODO_CODECS_BASIC+=inter/HALF
TODO_CODECS_BASIC+=inter/INTER_ALIAS
TODO_CODECS_BASIC+=inter/KANA_PHONETIC
TODO_CODECS_BASIC+=inter/LOWER
TODO_CODECS_BASIC+=inter/MAC
TODO_CODECS_BASIC+=inter/NOBOM
TODO_CODECS_BASIC+=inter/NFC
TODO_CODECS_BASIC+=inter/NFD
TODO_CODECS_BASIC+=inter/NFKC
TODO_CODECS_BASIC+=inter/NFKD
TODO_CODECS_BASIC+=inter/NL2BR
TODO_CODECS_BASIC+=inter/SCORE
TODO_CODECS_BASIC+=inter/SCORE_TRAIN
TODO_CODECS_BASIC+=inter/SPLIT
TODO_CODECS_BASIC+=inter/TO_ALIAS
TODO_CODECS_BASIC+=inter/TRIM-WIDTH
TODO_CODECS_BASIC+=inter/UNIX
TODO_CODECS_BASIC+=inter/UPPER
TODO_CODECS_BASIC+=inter/UPSIDEDOWN
TODO_CODECS_BASIC+=inter/WHITESPACE-DERAIL
TODO_CODECS_BASIC+=inter/WHITESPACE-RERAIL
TODO_CODECS_BASIC+=inter/WIDTH
TODO_CODECS_BASIC+=inter/WIN
TODO_CODECS_BASIC+=inter/ZH-STRINGS
TODO_CODECS_BASIC+=inter/ZHCN
TODO_CODECS_BASIC+=inter/ZHTW
TODO_CODECS_BASIC+=inter/ZHTW_WORDS
TODO_CODECS_BASIC+=inter/ZH_FUZZY_TW
TODO_CODECS_BASIC+=inter/ZH_FUZZY_CN
TODO_CODECS_BASIC+=to/00
TODO_CODECS_BASIC+=to/ANY
TODO_CODECS_BASIC+=to/ASCII-HTML-CNS11643-IMG
TODO_CODECS_BASIC+=to/ASCII-HTML-INFO
TODO_CODECS_BASIC+=to/ASCII-HTML-UNICODE-IMG
TODO_CODECS_BASIC+=to/ASCII-NAMED-HTML-ENTITY
TODO_CODECS_BASIC+=to/ASCII
TODO_CODECS_BASIC+=to/BIG5-5C
TODO_CODECS_BASIC+=to/BSDCONV
TODO_CODECS_BASIC+=to/BSDCONV_KEYWORD
TODO_CODECS_BASIC+=to/BSDCONV_LOG
TODO_CODECS_BASIC+=to/BSDCONV_STDOUT
TODO_CODECS_BASIC+=to/BYTE
TODO_CODECS_BASIC+=to/_CP1251
TODO_CODECS_BASIC+=to/_CP1252
TODO_CODECS_BASIC+=to/_CP1253
TODO_CODECS_BASIC+=to/_CP874
TODO_CODECS_BASIC+=to/_CP936
TODO_CODECS_BASIC+=to/CP936_TRANS
TODO_CODECS_BASIC+=to/_CP949
TODO_CODECS_BASIC+=to/_CP950
TODO_CODECS_BASIC+=to/CP950_TRANS
TODO_CODECS_BASIC+=to/ESCAPE
TODO_CODECS_BASIC+=to/_GB18030
TODO_CODECS_BASIC+=to/_GBK
TODO_CODECS_BASIC+=to/_ISO-8859-1
TODO_CODECS_BASIC+=to/_JIS0212
TODO_CODECS_BASIC+=to/_SHIFT-JIS
TODO_CODECS_BASIC+=to/NULL
TODO_CODECS_BASIC+=to/PASS
TODO_CODECS_BASIC+=to/RAW
TODO_CODECS_BASIC+=to/_UAO250
TODO_CODECS_BASIC+=to/UCS-2BE
TODO_CODECS_BASIC+=to/UCS-2LE
TODO_CODECS_BASIC+=to/UTF-16BE
TODO_CODECS_BASIC+=to/UTF-16LE
TODO_CODECS_BASIC+=to/UTF-32BE
TODO_CODECS_BASIC+=to/UTF-32LE
TODO_CODECS_BASIC+=to/_UTF-8

TODO_CODECS_CHINESE=
TODO_CODECS_CHINESE+=from/CCCII
TODO_CODECS_CHINESE+=from/_CNS11643
TODO_CODECS_CHINESE+=from/_CP936
TODO_CODECS_CHINESE+=from/_CP950
TODO_CODECS_CHINESE+=from/_GB2312
TODO_CODECS_CHINESE+=from/_UAO241
TODO_CODECS_CHINESE+=inter/CHEWING
TODO_CODECS_CHINESE+=inter/CNS11643
TODO_CODECS_CHINESE+=inter/HAN_PINYIN
TODO_CODECS_CHINESE+=inter/UNICODE
TODO_CODECS_CHINESE+=inter/ZH_COMP
TODO_CODECS_CHINESE+=inter/ZH_DECOMP
TODO_CODECS_CHINESE+=to/CCCII
TODO_CODECS_CHINESE+=to/_CNS11643
TODO_CODECS_CHINESE+=to/_GB2312
TODO_CODECS_CHINESE+=to/_UAO241

TODO_CODECS_EBCDIC=
TODO_CODECS_EBCDIC+=from/IBM-37
TODO_CODECS_EBCDIC+=from/IBM-930
TODO_CODECS_EBCDIC+=from/IBM-933
TODO_CODECS_EBCDIC+=from/IBM-935
TODO_CODECS_EBCDIC+=from/IBM-937
TODO_CODECS_EBCDIC+=from/IBM-939
TODO_CODECS_EBCDIC+=to/IBM-37
TODO_CODECS_EBCDIC+=to/IBM-930
TODO_CODECS_EBCDIC+=to/IBM-933
TODO_CODECS_EBCDIC+=to/IBM-935
TODO_CODECS_EBCDIC+=to/IBM-937
TODO_CODECS_EBCDIC+=to/IBM-939

all: libbsdconv bsdconv-mktable meta bsdconv-man bsdconv-completion bsdconv codecs

alias:
	python tools/mkalias.py codecs/from/alias codecs/inter/FROM_ALIAS.txt
	@printf "014C,014F,0143,0141,014C,0145\t?\n" >> codecs/inter/FROM_ALIAS.txt
	python tools/mkalias.py codecs/inter/alias codecs/inter/INTER_ALIAS.txt
	python tools/mkalias.py codecs/to/alias codecs/inter/TO_ALIAS.txt
	@printf "014C,014F,0143,0141,014C,0145\t?\n" >> codecs/inter/TO_ALIAS.txt

builddir:
	mkdir -p build/bin
	mkdir -p build/lib
	mkdir -p build/include
	mkdir -p build/share/bsdconv/from
	mkdir -p build/share/bsdconv/inter
	mkdir -p build/share/bsdconv/to

installdir:
	mkdir -p ${PREFIX}/bin
	mkdir -p ${PREFIX}/lib
	mkdir -p ${PREFIX}/include
	mkdir -p ${PREFIX}/share/bsdconv/from
	mkdir -p ${PREFIX}/share/bsdconv/inter
	mkdir -p ${PREFIX}/share/bsdconv/to

libbsdconv: builddir src/libbsdconv.c src/bsdconv.h
	$(CC) ${CFLAGS} src/libbsdconv.c -fPIC -shared -o build/lib/${SHLIBNAME} ${LIBS}

bsdconv: builddir libbsdconv meta src/bsdconv.h src/bsdconv.c
	$(CC) ${CFLAGS} src/bsdconv.c -L./build/lib/ -o build/bin/bsdconv -lbsdconv ${LIBS}

bsdconv-mktable: builddir src/bsdconv.h src/bsdconv-mktable.c
	$(CC) ${CFLAGS} -DUSE_FMALLOC src/libfmalloc.c src/bsdconv-mktable.c -o build/bin/bsdconv-mktable

bsdconv-man: builddir libbsdconv meta src/bsdconv.h src/bsdconv.c
	$(CC) ${CFLAGS} src/bsdconv-man.c -L./build/lib/ -o build/bin/bsdconv-man -lbsdconv ${LIBS}

bsdconv-completion: builddir libbsdconv src/bsdconv.h src/bsdconv-completion.c
	$(CC) ${CFLAGS} src/bsdconv-completion.c -L./build/lib -o build/bin/bsdconv-completion -lbsdconv ${LIBS}

bsdconv_dbg: builddir libbsdconv src/libbsdconv.c src/bsdconv.h src/bsdconv_dbg.c
	$(CC) ${CFLAGS} src/libbsdconv.c src/bsdconv_dbg.c -o build/bin/bsdconv_dbg ${LIBS}

codecs_basic: builddir bsdconv-mktable
	for item in ${TODO_CODECS_BASIC} ; do \
		./build/bin/bsdconv-mktable codecs/$${item}.txt ./build/share/bsdconv/$${item} ; \
		if [ -e codecs/$${item}.man ]; then cp codecs/$${item}.man ./build/share/bsdconv/$${item}.man ; fi ; \
		if [ -e codecs/$${item}.c ]; then $(CC) ${CFLAGS} codecs/$${item}.c -L./build/lib/ -fPIC -shared -o ./build/share/bsdconv/$${item}.so -lbsdconv ${LIBS} ; fi ; \
	done

codecs_chinese: builddir bsdconv-mktable
	for item in ${TODO_CODECS_CHINESE} ; do \
		./build/bin/bsdconv-mktable codecs/$${item}.txt ./build/share/bsdconv/$${item} ; \
		if [ -e codecs/$${item}.man ]; then cp codecs/$${item}.man ./build/share/bsdconv/$${item}.man ; fi ; \
		if [ -e codecs/$${item}.c ]; then $(CC) ${CFLAGS} codecs/$${item}.c -L./build/lib/ -fPIC -shared -o ./build/share/bsdconv/$${item}.so -lbsdconv ${LIBS} ; fi ; \
	done

codecs_ebcdic: builddir bsdconv-mktable
	for item in ${TODO_CODECS_EBCDIC} ; do \
		./build/bin/bsdconv-mktable codecs/$${item}.txt ./build/share/bsdconv/$${item} ; \
		if [ -e codecs/$${item}.man ]; then cp codecs/$${item}.man ./build/share/bsdconv/$${item}.man ; fi ; \
		if [ -e codecs/$${item}.c ]; then $(CC) ${CFLAGS} codecs/$${item}.c -L./build/lib/ -fPIC -shared -o ./build/share/bsdconv/$${item}.so -lbsdconv ${LIBS} ; fi ; \
	done

codecs: codecs_basic codecs_chinese codecs_ebcdic

meta: libbsdconv
	if [ ${SHLIBNAME} != libbsdconv.so ]; then \
		ln -sf libbsdconv.so.${SHLIBVER} build/lib/libbsdconv.so ; \
	fi
	cp src/bsdconv.h build/include
	cp codecs/from/alias build/share/bsdconv/from/alias
	cp codecs/inter/alias build/share/bsdconv/inter/alias
	cp codecs/to/alias build/share/bsdconv/to/alias

clean:
	rm -rf build
	rm -rf testsuite/api

install: installdir install_main install_basic install_chinese install_ebcdic

install_main:
	install -m 555 build/bin/bsdconv ${PREFIX}/bin
	install -m 555 build/bin/bsdconv-man ${PREFIX}/bin
	install -m 555 build/bin/bsdconv-mktable ${PREFIX}/bin
	install -m 555 build/bin/bsdconv-completion ${PREFIX}/bin
	install -m 444 build/include/bsdconv.h ${PREFIX}/include
	install -m 444 build/lib/${SHLIBNAME} ${PREFIX}/lib
	install -m 444 build/share/bsdconv/from/alias ${PREFIX}/share/bsdconv/from/alias
	install -m 444 build/share/bsdconv/inter/alias ${PREFIX}/share/bsdconv/inter/alias
	install -m 444 build/share/bsdconv/to/alias ${PREFIX}/share/bsdconv/to/alias
	if [ ${SHLIBNAME} != libbsdconv.so ]; then \
		ln -sf libbsdconv.so.${SHLIBVER} ${PREFIX}/lib/libbsdconv.so ; \
	fi

install_doc:
	install -m 555 build/bin/bsdconv-man ${PREFIX}/bin

install_basic:
	for item in ${TODO_CODECS_BASIC} ; do \
		install -m 444 build/share/bsdconv/$${item} ${PREFIX}/share/bsdconv/$${item} ; \
		if [ -e build/share/bsdconv/$${item}.man ]; then install -m 444 build/share/bsdconv/$${item}.man ${PREFIX}/share/bsdconv/$${item}.man ; fi ; \
		if [ -e build/share/bsdconv/$${item}.so ]; then install -m 444 build/share/bsdconv/$${item}.so ${PREFIX}/share/bsdconv/$${item}.so ; fi ; \
	done

install_chinese:
	for item in ${TODO_CODECS_CHINESE} ; do \
		install -m 444 build/share/bsdconv/$${item} ${PREFIX}/share/bsdconv/$${item} ; \
		if [ -e build/share/bsdconv/$${item}.man ]; then install -m 444 build/share/bsdconv/$${item}.man ${PREFIX}/share/bsdconv/$${item}.man ; fi ; \
		if [ -e build/share/bsdconv/$${item}.so ]; then install -m 444 build/share/bsdconv/$${item}.so ${PREFIX}/share/bsdconv/$${item}.so ; fi ; \
	done

install_ebcdic:
	for item in ${TODO_CODECS_EBCDIC} ; do \
		install -m 444 build/share/bsdconv/$${item} ${PREFIX}/share/bsdconv/$${item} ; \
		if [ -e build/share/bsdconv/$${item}.man ]; then install -m 444 build/share/bsdconv/$${item}.man ${PREFIX}/share/bsdconv/$${item}.man ; fi ; \
		if [ -e build/share/bsdconv/$${item}.so ]; then install -m 444 build/share/bsdconv/$${item}.so ${PREFIX}/share/bsdconv/$${item}.so ; fi ; \
	done

test:
	@python testsuite/conversion.py
	@$(CC) ${CFLAGS} testsuite/api.c -L./build/lib/ -o testsuite/api -lbsdconv ${LIBS}
	@./testsuite/api

plist:
	@echo bin/bsdconv
	@echo bin/bsdconv-completion
	@echo bin/bsdconv-man
	@echo bin/bsdconv-mktable
	@echo include/bsdconv.h
	@echo lib/libbsdconv.so
	@echo lib/${SHLIBNAME}
	@echo %%DATADIR%%/from/alias
	@echo %%DATADIR%%/inter/alias
	@echo %%DATADIR%%/to/alias
	@for item in ${TODO_CODECS_BASIC} ; do \
		echo %%DATADIR%%/$${item} ; \
		if [ -e codecs/$${item}.man ]; then echo %%DATADIR%%/$${item}.man ; fi ; \
		if [ -e codecs/$${item}.c ]; then echo %%DATADIR%%/$${item}.so ; fi ; \
	done
	@for item in ${TODO_CODECS_CHINESE} ; do \
		echo %%CHINESE%%%%DATADIR%%/$${item} ; \
		if [ -e codecs/$${item}.man ]; then echo %%CHINESE%%%%DATADIR%%/$${item}.man ; fi ; \
		if [ -e codecs/$${item}.c ]; then echo %%CHINESE%%%%DATADIR%%/$${item}.so ; fi ; \
	done
	@for item in ${TODO_CODECS_EBCDIC} ; do \
		echo %%EBCDIC%%%%DATADIR%%/$${item} ; \
		if [ -e codecs/$${item}.man ]; then echo %%EBCDIC%%%%DATADIR%%/$${item}.man ; fi ; \
		if [ -e codecs/$${item}.c ]; then echo %%EBCDIC%%%%DATADIR%%/$${item}.so ; fi ; \
	done
	@echo @dirrmtry %%DATADIR%%/to
	@echo @dirrmtry %%DATADIR%%/inter
	@echo @dirrmtry %%DATADIR%%/from
	@echo @dirrmtry %%DATADIR%%

URL=	http://cnmc.tw/~buganini/chvar/engine.php?action=dump
chvar:
	wget -O codecs/inter/ZHTW.txt "${URL}&mode=norml&for=tw"
	wget -O codecs/inter/ZHCN.txt "${URL}&mode=norml&for=cn"
	wget -O codecs/inter/ZH_FUZZY_TW.txt "${URL}&mode=fuzzy&for=tw"
	wget -O codecs/inter/ZH_FUZZY_CN.txt "${URL}&mode=fuzzy&for=cn"
	@for file in ZHTW ZHCN ZH_FUZZY_TW ZH_FUZZY_CN; do \
		sed -i '' -e 's|^|01|g' "codecs/inter/$${file}.txt" ; \
		sed -i '' -e 's|	|	01|g' "codecs/inter/$${file}.txt" ; \
	done
	wget -O codecs/to/CP950_TRANS.txt "${URL}&mode=trans&for=cp950"
	wget -O codecs/to/CP936_TRANS.txt "${URL}&mode=trans&for=cp936"
	@for file in CP950_TRANS CP936_TRANS ; do \
		sed -i '' -e 's|^|01|g' "codecs/to/$${file}.txt" ; \
	done

