HEADER_PATH="src/headers/version.h"
TAG=`git describe --tags`
HEADER_NEW="#define VERSION \"${TAG}\""

if [ -f $HEADER_PATH ]; then
    HEADER_OLD=`cat $HEADER_PATH`
else
    HEADER_OLD=''
fi

if [ "$HEADER_NEW" != "$HEADER_OLD" ]; then
    echo "Overwriting version file"
    echo $HEADER_NEW > $HEADER_PATH
fi
