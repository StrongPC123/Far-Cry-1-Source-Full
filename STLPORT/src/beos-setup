#!/bin/sh

function intel_setup
{
files='alloca assert ctype dirent err fcntl float new inttypes libio limits locale malloc math memory signal stdarg stddef stdio stdlib string time va_list wchar wctype'

for file in $files; do
   echo '#include_next "'"$file"'.h"' > "../stlport/beos/$file"'.h'
   echo '#include_next "'"c$file"'"' > "../stlport/beos/c$file"''
done

files='fstream new exception typeinfo iomanip iosfwd iostream istream ostream stdexcept stdiostream stream streambuf strstream'

for file in $files; do
   echo '#include_next "'"$file"'.h"' > "../stlport/beos/$file"'.h'
   echo '#include_next "'"$file"'"' > "../stlport/beos/$file"''
done
}

function intel_uninstall
{
	rm -fr ~/config/include/stlport
	cd ../lib
	for file in libstlport_gcc*; do
		rm -f ~/config/lib/$file
	done
	rm -f ~/config/lib/libstlport_gcc*.so
}

function intel_install
{
	cp -R ../stlport ~/config/include

	cd ../lib
	for file in libstlport_gcc*.so.*; do
		basename=`echo $file | sed 's/\(.*\.so\).*/\1/'`
		cp $file ~/config/lib
		if test "$file" != "$basename"; then
			ln -s ~/config/lib/$file ~/config/lib/$basename
		fi
	done

	for file in libstlport_gcc*.a; do
		cp $file ~/config/lib
	done

	echo include files are in ~/config/include
	echo libraries are in ~/config/lib
}


if test $# -ne 1; then
echo usage:
echo "	beos-setup -setup       # sets up headers"
echo "	beos-setup -install     # installs STLPort"
echo "	beos-setup -uninstall   #uninstalls STLPort"

else

if test $1 = "-setup"; then
	intel_setup
	exit 0
fi

if test $1 = "-uninstall"; then
	intel_uninstall
	exit 0
fi

if test $1 = "-install"; then
	intel_install
	exit 0
fi

echo unknown option $*
exit 1



fi







