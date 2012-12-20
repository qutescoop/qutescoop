#! /bin/sh
## uploads files to Sourceforge. Adapt login and path as needed
if [ -z $1 ]; then
	echo "usage: $0 <files>"
	echo "	adapt script as needed with login and remote path"
else
	COMMAND="rsync -vz --progress $@ jonaseberle,qutescoop@frs.sourceforge.net:/home/frs/project/qutescoop/QuteScoop/2.1/"
	echo Calling \"$COMMAND\"
	$COMMAND
fi
