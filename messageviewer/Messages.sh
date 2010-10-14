#! /bin/sh
$EXTRACTRC `find . -name '*.ui' -or -name '*.rc' -or -name '*.kcfg' -or -name '*.kcfg.cmake'` >> rc.cpp || exit 11
$XGETTEXT `find . -name '*.cpp'` -o $podir/libmessageviewer.pot
rm -f rc.cpp
