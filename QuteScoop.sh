#!/bin/sh
# Dist packages include all libs from the build environment in ./lib.
# If your OS libs are new enough or if you compiled yourself this is not needed.

dir="$(dirname "$0")"
app="$dir/$(basename "$0" .sh)"
lib="$dir/lib"

LD_LIBRARY_PATH="$lib" "$app" "$@"
