# #####################################################################
# This file is part of QuteScoop. See README for license
# #####################################################################
# This .pro file is just to allow easier cross-platform
# development with multiple instances of QtCreator on the same source-tree
# where QtCreator creates .pro.user-files for each platform which overwrite
# each other thus invalidating all project settings.
# This files does not have any functionality in its own.

include("QuteScoop.pro")

OTHER_FILES += \
    QuteScoop.sh
LIBS += -lGLU
