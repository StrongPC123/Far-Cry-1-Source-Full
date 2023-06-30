#!/bin/sh

GNUMake=gmake
MakeFile=build-tempinc-xlc.mk

PassNumber=0
echo "Building the AIX tempinc directory."
echo "Calling $MakeFile."
until  { $GNUMake -f $MakeFile -q ; }
do
    let PassNumber=$PassNumber+1
    echo "Pass Number: $PassNumber"
    if { ! $GNUMake -f $MakeFile ; } then
        echo "An error occurred.. aborting"
        exit 1;
    fi
done
echo "Finished the AIX tempinc directory."
