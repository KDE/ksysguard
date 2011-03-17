#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.ui" -o -name "*.rc"` >> rc.cpp || exit 11
$EXTRACTATTR --attr="WorkSheet,title" --attr="display,title" `find . -name "*.sgrd"` >> rc.cpp || exit 13
$XGETTEXT `find . -name "*.cpp" -o $podir/ksysguard.pot
rm -f rc.cpp
