#! /bin/sh
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui" -o -name "*.kcfg" -o -name "*.h"` >> rc.cpp || exit 11
$XGETTEXT $(find -name "*.cpp" ) -o $podir/akregator.pot
