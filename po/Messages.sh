#!/bin/sh
# Rebuilds po/denkzettel.pot from the i18n() calls in src/ and merges it into
# every catalogue under po/<lang>/. Run it from the project root after adding,
# removing or rewording a source string; a catalogue that is not merged keeps
# translating a msgid the code no longer asks for, and the window silently
# falls back to English.
#
# The keyword list is KI18n's: every i18n family function, with the argument
# that carries the context marked with `c` so xgettext writes it as msgctxt.
set -e

xgettext --from-code=UTF-8 -C --kde \
    -ci18n \
    -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 \
    -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 \
    -kxi18n:1 -kxi18nc:1c,2 -kxi18np:1,2 -kxi18ncp:1c,2,3 \
    -kkxi18n:1 -kkxi18nc:1c,2 -kkxi18np:1,2 -kkxi18ncp:1c,2,3 \
    -kI18N_NOOP:1 -kI18NC_NOOP:1c,2 \
    --package-name=denkzettel \
    --msgid-bugs-address='https://github.com/hnsstrk/denkzettel/issues' \
    --copyright-holder='This file is copyright of the Denkzettel authors.' \
    -o po/denkzettel.pot \
    $(find src -name '*.cpp' -o -name '*.h' | sort)

for catalogue in po/*/denkzettel.po; do
    msgmerge --quiet --update --backup=none "$catalogue" po/denkzettel.pot
done
