######################################################################
#  This file is part of QuteScoop.
#  Copyright (C) 2007-2009 Martin Domig <martin@domig.net>
#
#  QuteScoop is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  QuteScoop is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with QuteScoop.  If not, see <http://www.gnu.org/licenses/>
######################################################################
#  This script creates a distributable mac bundle from a release
#  build. To do this, it has to add all required Qt libraries to
#  the bundle and do some magic to the link table
######################################################################
#!/bin/sh

QTDIR="/usr/local/Trolltech/Qt-4.3.2"
FRAMEWORKS="QtCore QtGui QtNetwork QtOpenGL"

rm -rf Qutescoop.app/Contents/Frameworks
mkdir QuteScoop.app/Contents/Frameworks 2>/dev/null

for i in $FRAMEWORKS; do
	echo "Framework $i..."
	cp -R $QTDIR/lib/$i.framework QuteScoop.app/Contents/Frameworks
	install_name_tool -id @executable_path/../Frameworks/$i.framework/Versions/4/$i \
		QuteScoop.app/Contents/Frameworks/$i.framework/Versions/4/$i
	install_name_tool -change $QTDIR/lib/$i.framework/Versions/4/$i \
		@executable_path/../Frameworks/$i.framework/Versions/4/$i \
		QuteScoop.app/Contents/MacOs/QuteScoop

	install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/4/QtCore \
		@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore \
		QuteScoop.app/Contents/Frameworks/$i.framework/Versions/4/$i
	install_name_tool -change $QTDIR/lib/QtGui.framework/Versions/4/QtGui \
		@executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui \
		QuteScoop.app/Contents/Frameworks/$i.framework/Versions/4/$i
done

echo -n "Cleaning up... "
rm -rf QuteScoop.app/Contents/Frameworks/*/*/*/*_debug
rm -rf Qutescoop.app/Contents/Frameworks/*/*/*/Headers
echo "done."
