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
