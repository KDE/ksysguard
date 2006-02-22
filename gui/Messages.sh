#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.ui"` >> rc.cpp || exit 11
$EXTRACTRC `find . -name "*.sgrd"` >> rc.cpp || exit 12
$XGETTEXT `find . -name "*.cpp" -o -name "*.cc"` -o $podir/ksysguard.pot
