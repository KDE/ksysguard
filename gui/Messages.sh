#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.ui"` >> rc.cpp || exit 11
$EXTRACTRC `find . -name "*.sgrd"` >> rc.cpp || exit 12
$EXTRACTATTR --attr="WorkSheet,title" `find . -name "*.sgrd"` >> rc.cpp || exit 13
$XGETTEXT `find . -name "*.cpp" -o -name "*.cc"` -o $podir/ksysguard.pot
