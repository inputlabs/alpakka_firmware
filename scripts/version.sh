HEADER_PATH="src/headers/version.h"

if ( git describe --tags ); then
    TAG=`git describe --tags`
else
    TAG=${GITHUB_REF}
fi

if [ -f $HEADER_PATH ]; then
    HEADER_OLD=`cat $HEADER_PATH`
else
    HEADER_OLD=''
fi

HEADER_NEW="#define VERSION \"${TAG}\""
if [ "$HEADER_NEW" != "$HEADER_OLD" ]; then
    echo "Overwriting version file"
    echo $HEADER_NEW > $HEADER_PATH
fi
