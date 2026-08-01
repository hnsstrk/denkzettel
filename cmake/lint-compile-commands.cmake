# Copies the compile database and drops the flags clang does not know
# (issue #45). Driven by the lint-database target in the top level
# CMakeLists.txt, which passes INPUT and OUTPUT.
#
# Qt6 adds -mno-direct-extern-access for GNU compilers (Qt6Targets.cmake, the
# clang spelling is -fno-direct-access-external-data), ECM adds -Wlogical-op
# (KDECompilerSettings.cmake). clang-tidy and clazy do carry on analysing after
# them, but the unknown argument is reported as a compiler error for every file
# and leaves the exit status of a run useless as a lint verdict.
file(READ ${INPUT} DENKZETTEL_COMPILE_COMMANDS)
string(REPLACE " -mno-direct-extern-access" "" DENKZETTEL_COMPILE_COMMANDS "${DENKZETTEL_COMPILE_COMMANDS}")
string(REPLACE " -Wlogical-op" "" DENKZETTEL_COMPILE_COMMANDS "${DENKZETTEL_COMPILE_COMMANDS}")
file(WRITE ${OUTPUT} "${DENKZETTEL_COMPILE_COMMANDS}")
