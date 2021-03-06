#!/bin/sh
# Run this to generate all the initial makefiles, etc.
# Ripped off from GNOME macros version

DIE=0

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

configure_mono=1
build_mono=1
use_sgen_in_mono=no
mono_flags=""
mono_path=../mono
mcs_path=../mono/mcs
mono_basic_path=../mono-basic
configure_gallium=1
build_gallium=1
gallium_path=../mesa
build_cairo=1
build_curl=1
for arg; do
  case "$arg" in
    --with-manual-mono=yes | --with-manual-mono )
      build_mono=0
      configure_mono=0
      ;;
    --with-manual-mono=build )
      build_mono=1
      configure_mono=0
      ;;
    --with-mcs-path* )
      mcs_path=$(echo $arg|sed -e 's,.*=,,')
      ;;
    --with-mono-path* )
      mono_path=$(echo $arg|sed -e 's,.*=,,')
      ;;
    --with-mono-basic-path* )
      mono_basic_path=$(echo $arg|sed -e 's,.*=,,')
      ;;
    --with-manual-gallium=yes | --with-manual-gallium )
      build_gallium=0
      configure_gallium=0
      ;;
    --with-manual-gallium=build )
      build_gallium=1
      configure_gallium=0
      ;;
    --with-gallium-path* )
      gallium_path=$(echo $arg|sed -e 's,.*=,,')
      ;;
    --with-sgen=yes | --with-sgen )
      use_sgen=yes
      ;;
    --with-cairo=system )
      build_cairo=0
      ;;
    --with-curl=system )
      build_curl=0
      ;;
    --host* )
      # pass the host arg on to mono
      mono_flags="$mono_flags $arg"
      ;;
    --build* )
      # pass the build arg on to mono
      mono_flags="$mono_flags $arg"
      ;;
  esac
done

if [ x"$use_sgen" = "xyes" ]; then
      mono_flags="$mono_flags --with-moon-gc=sgen"
else
      mono_flags="$mono_flags --with-sgen=no"
fi


if [ -n "$MONO_PATH" ]; then
	# from -> /mono/lib:/another/mono/lib
	# to -> /mono /another/mono
	for i in `echo ${MONO_PATH} | tr ":" " "`; do
		i=`dirname ${i}`
		if [ -n "{i}" -a -d "${i}/share/aclocal" ]; then
			ACLOCAL_FLAGS="-I ${i}/share/aclocal $ACLOCAL_FLAGS"
		fi
		if [ -n "{i}" -a -d "${i}/bin" ]; then
			PATH="${i}/bin:$PATH"
		fi
	done
	export PATH
fi

ACLOCAL_FLAGS="-I m4 $ACLOCAL_FLAGS"

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`autoconf' installed to compile Moonlight."
  echo "Download the appropriate package for your distribution,"
  echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
}

if [ -z "$LIBTOOL" ]; then
  LIBTOOL=`which glibtool 2>/dev/null` 
  if [ ! -x "$LIBTOOL" ]; then
    LIBTOOL=`which libtool`
  fi
fi

(grep "^AM_PROG_LIBTOOL" $srcdir/configure.ac >/dev/null) && {
  ($LIBTOOL --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`libtool' installed to compile Moonlight."
    echo "Get ftp://ftp.gnu.org/pub/gnu/libtool-1.2d.tar.gz"
    echo "(or a newer version if it is available)"
    DIE=1
  }
}

grep "^AM_GNU_GETTEXT" $srcdir/configure.ac >/dev/null && {
  grep "sed.*POTFILES" $srcdir/configure.ac >/dev/null || \
  (gettext --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`gettext' installed to compile Moonlight."
    echo "Get ftp://alpha.gnu.org/gnu/gettext-0.10.35.tar.gz"
    echo "(or a newer version if it is available)"
    DIE=1
  }
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`automake' installed to compile Moonlight."
  echo "Get ftp://ftp.gnu.org/pub/gnu/automake-1.3.tar.gz"
  echo "(or a newer version if it is available)"
  DIE=1
  NO_AUTOMAKE=yes
}


# if no automake, don't bother testing for aclocal
test -n "$NO_AUTOMAKE" || (aclocal --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: Missing \`aclocal'.  The version of \`automake'"
  echo "installed doesn't appear recent enough."
  echo "Get ftp://ftp.gnu.org/pub/gnu/automake-1.3.tar.gz"
  echo "(or a newer version if it is available)"
  DIE=1
}

if test "$DIE" -eq 1; then
  exit 1
fi

if test -z "$*"; then
  echo "**Warning**: I am going to run \`configure' with no arguments."
  echo "If you wish to pass any to it, please specify them on the"
  echo \`$0\'" command line."
  echo
fi

case $CC in
xlc )
  am_opt=--include-deps;;
esac


if grep "^AC_PROG_LIBTOOL" configure.ac >/dev/null; then
  if test -z "$NO_LIBTOOLIZE" ; then 
    echo "Running libtoolize..."
    ${LIBTOOL}ize --force --copy
  fi
fi

echo "Running aclocal $ACLOCAL_FLAGS ..."
aclocal $ACLOCAL_FLAGS || {
  echo
  echo "**Error**: aclocal failed. This may mean that you have not"
  echo "installed all of the packages you need, or you may need to"
  echo "set ACLOCAL_FLAGS to include \"-I \$prefix/share/aclocal\""
  echo "for the prefix where you installed the packages whose"
  echo "macros were not found"
  exit 1
}

if grep "^AM_CONFIG_HEADER" configure.ac >/dev/null; then
  echo "Running autoheader..."
  autoheader || { echo "**Error**: autoheader failed."; exit 1; }
fi

echo "Running automake --gnu $am_opt ..."
automake --add-missing --gnu $am_opt ||
  { echo "**Error**: automake failed."; exit 1; }
echo "Running autoconf ..."
autoconf || { echo "**Error**: autoconf failed."; exit 1; }

if test -d $srcdir/pixman; then
  echo Running pixman/autogen.sh ...
  (cd $srcdir/pixman ; NOCONFIGURE=1 ./autogen.sh "$@")
  echo Done running pixman/autogen.sh ...
fi

if [ $build_cairo -eq 1 ] ; then
	if test -d $srcdir/cairo; then
	  echo Running cairo/autogen.sh ...
	  pixmandir=`readlink -n $0`
	  pixmandir=`dirname $pixmandir`/pixman

	  old_pkg=$PKG_CONFIG_PATH
	  PKG_CONFIG_PATH=$pixmandir
	  export PKG_CONFIG_PATH

	  if test -n "$old_pkg"; then
	      PKG_CONFIG_PATH=$pixmandir:$old_pkg
	      export PKG_CONFIG_PATH
	  fi

	  (cd $srcdir/cairo ; NOCONFIGURE=1 ./autogen.sh "$@")

	  echo Done running cairo/autogen.sh ...
	fi
fi

if [ $build_curl -eq 1 ] ; then
  if test -d $srcdir/curl; then
    echo Running curl/buildconf ...
    (cd $srcdir/curl ; ./buildconf )
    echo Done running curl/buildconf ...
  fi
fi

if test ! -d $mono_path; then
	mono_path=$mcs_path/..
fi

if [ $configure_mono -eq 1 ] ; then
  if test -d $mono_basic_path; then
    echo Running $mono_basic_path/configure ...
    (cd $mono_basic_path/ ; ./configure "$@" --moonlight-sdk-location=$mcs_path/class/lib/moonlight_raw --with-moonlight=yes)
    echo Done running $mono_basic_path/configure ...
  fi
  if test -d $mono_path; then
    echo Running $mono_path/autogen.sh ...
    # we build --with-sgen=no to not build both boehm and sgen (and we build with boehm instead of sgen because sgen has a problem nobody has investigated much into yet)
    (cd $mono_path/ ; ./autogen.sh "$@" --with-moonlight=only --with-profile4=no --enable-minimal=aot,interpreter --with-ikvm-native=no --with-mcs-docs=no --disable-nls --disable-mono-debugger --with-shared_mono=no $mono_flags)
    echo Done running $mono_path/autogen.sh ...
  fi
fi

if [ $configure_gallium -eq 1 ] ; then
  if test -d $gallium_path; then
    echo Running $gallium_path/autogen.sh ...
    gallium_conf_flags=$(echo "$@"|sed -e 's,enable-llvm,enable-gallium-llvm,')
    (cd $gallium_path/ ; ./autogen.sh $gallium_conf_flags --with-driver=xlib)
    echo Done running $gallium_path/autogen.sh ...
  fi
fi

conf_flags="--enable-maintainer-mode --enable-compile-warnings --with-sanity-checks -C" #--enable-iso-c

if test x$NOCONFIGURE = x; then
  echo Running $srcdir/configure $conf_flags "$@" ...
  $srcdir/configure $conf_flags "$@" \
  && echo Now type \`make\' to compile $PKG_NAME || exit 1
else
  echo Skipping configure process.
fi
