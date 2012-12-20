# #####################################################################
# This file is part of QuteScoop. See README for license
# #####################################################################

# Defines 'make installer-*'-targets that build the installer using an
# existing BitRock InstallBuilder installation.
#
# You can use BitRockInstallBuilderQt-license.xml to use our free
# open source license (only for projects named QuteScoop!).
#
# Be sure to only deploy 32bit versions to not unnecessarily exclude
# any users! If you use 64bit for development, the easiest way is to set
# up a virtual 32bit machine for that.
#
# Check out ./upload.sh if you want to upload files to Sourceforge using
# the console (needs an SFTP-, e.g. rsync-client).

BITROCK_BUILDER_PATH="/opt/installbuilder-7.2.2/bin/builder"
BITROCK_PROJECT="BitRockInstallBuilderQt-QuteScoop.xml"

installer_linux.target = installer-linux
installer_linux.commands = "$${BITROCK_BUILDER_PATH} build $${BITROCK_PROJECT} linux --setvars product_version=$${VERSION}"
#installer_linux.depends = install
QMAKE_EXTRA_TARGETS += installer_linux

installer_windows.target = installer-windows
installer_windows.commands = "$${BITROCK_BUILDER_PATH} build $${BITROCK_PROJECT} windows --setvars product_version=$${VERSION}"
installer_windows.depends = install
QMAKE_EXTRA_TARGETS += installer_windows

!build_pass:message("If you have built 32bit (we only want to deploy 32bit to users!) and have \
BitrockInstallBuilder installed (check QuteScoop-makeInstaller.pri for correct paths), \
set the QuteScoop version number in QuteScoop.pro, run 'make install' and use \
the make-targets '$${installer_windows.target}' and '$${installer_linux.target}' to build the installers.")
