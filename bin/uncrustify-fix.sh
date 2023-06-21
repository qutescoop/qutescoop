#!/bin/bash
# Applies changes to files!
#
# @see .github/workflows/build.yaml for used crustify version

uncrustify -q -c uncrustify.cfg --replace --no-backup src/*.{h,cpp} src/*/*.{h,cpp} src/*/*/*.{h,cpp}
