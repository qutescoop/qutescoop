#!/bin/bash
# Checks for errors.
# Exit code is set.
#
# @see .github/workflows/build.yaml for used crustify version

uncrustify -q -c uncrustify.cfg --check src/*.{h,cpp} src/*/*.{h,cpp} src/*/*/*.{h,cpp} \
	|| ( echo "There were linting issues."; false )
