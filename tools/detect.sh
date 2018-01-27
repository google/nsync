#!/bin/sh
# Copyright 2018 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Detect platform properties based on the cc command given as first argument,
# then invoke the make command that follows on the command line with variables
# NSYNC_OS, NSYNC_OS_TYPE, NSYNC_CC, and NSYNC_ARCH indicating those
# properties.
#
# This script must use _only_ properties derived from the compiler supplied,
# becuase that may be a cross compiler, targeting a platform quite different
# from the host.
#
# detect.sh will work for most Unix-like platforms.  Uses the path to the detect.c
# header if specified; otherwise it assumes that it is called "detect.c" in the same
# directory as the detect.sh script.

usage="usage: detect.sh \\
	[-target_lang <target_lang_type>] \\
	[-target_arch <target_arch_type>] \\
	[-target_cc <target_cc_type>] \\
	[-target_os <target_os_type>] \\
	[-nsync_base <path_to_nsync_root>] \\
	[-detect_path <path_to_detect.c>] \\
	<cc_command> <make_command> [make_args...]

It is not usually necessary to specify options.   The target compiler, operating system, language, and architecture
types are normally deduced by running the supplied compiler.

The paths are normally inferred from the structure of the nsync tree and the
fact that this script is usually invoked from a child of the 'builds'
directory.
"

detect_base=detect.c

detect_path=
nsync_base=../..
target_lang=
target_arch=
target_cc=
target_os=
while
	arg="$1"
	case "$arg" in
	-*)	found=false
		for flag in target_lang target_arch target_cc target_os detect_path nsync_base; do
			case "$arg" in
			"-$flag")	eval $flag='"${2?"$usage"}"'; shift; found=true; break;;
			-"$flag"=*)	s=`expr "$arg" : '^[^=]*=\(.*\)$'`; eval $flag="'$s'"; found=true; break;;
			esac
		done
		case "$found" in false)	echo "$usage" >&2; exit 2;; esac
		;;
	*)	break;;
	esac
do
	shift
done

case $# in [01]) echo "$usage" >&2; exit 2;; esac
cc="$1"
make="$2"
shift
shift

case "$detect_path" in
"")	case "$0" in
	*/*)	detect_path=`expr "$0" : '^\(.*/\)[^/]*$'`"$detect_base";;
	*)	path="$PATH":.
		while [ ! -f "$detect_path" -a -n "$path" ]; do
			detect_path=`expr "$path" : '^\([^:]*\).*$'`"/$detect_base"
			path=`expr "$path" : '^[^:]*:\(.*\)$'`
		done;;
	esac
esac

if [ ! -f "$detect_path" ]; then
	echo "$0: can't find $detect_base; maybe use -detect_path flag?" >&2; exit 2
fi

# Set variables detected_* from detect.c
eval `$cc -E "$detect_path" | sed -n 's,^\(detected_.*=.*\)X,\1,p'`

case "$detected_cc.$detected_lang" in
pcc.c11)  # pcc lies when it claims to implement c11
	detected_lang=c;;
esac

# Allow overrides from the command line.
case "$target_lang" in ?*) detected_lang="$target_lang";; esac
case "$target_arch" in ?*) detected_arch="$target_arch";; esac
case "$target_cc" in   ?*) detected_cc="$target_cc";; esac
case "$target_os" in   ?*) detected_os="$target_os";; esac

extra_c="$DETECT_EXTRA_CFLAGS"
extra_objs=
extra_s=
extra_cpp_flags="$DETECT_EXTRA_CPPFLAGS"
extra_libs="$DETECT_EXTRA_LIBS"

# If we can't get atomics from the OS or the compiler, try to find an assembly
# language implementation based on the architecture, or one provided by the
# language library.
if [ ! -f "$nsync_base/platform/$detected_os/atomic.h" -a ! -f "$nsync_base/platform/$detected_cc/atomic.h" ]; then
	case "$detected_lang" in
	c11)	extra_cpp_flags="$extra_cpp_flags -DNSYNC_ATOMIC_C11 -I../../platform/c11";;
	c++11)	extra_cpp_flags="$extra_cpp_flags -DNSYNC_USE_CPP11_TIMEPOINT -DNSYNC_ATOMIC_CPP11 -I../../platform/c++11";;
	*)	# Try for assembly language.
		for x in "$nsync_base/platform/$detected_arch/src/nsync_atm_$detected_arch".[sS]; do
			if [ -f "$x" ]; then
				extra_cpp_flags="$extra_cpp_flags -I$nsync_base/platform/atomic_ind"
				extra_s="$extra_s $x"
				extra_objs="$extra_objs nsync_atm_$detected_arch.o"
				break
			fi
		done;;
	esac
fi

# Some systems are special in various ways.
case "$detected_arch.$detected_os.$detected_cc" in
*.macos.*)	# Some MacOS versions don't provide clock_gettime().
		extra_c="$extra_c $nsync_base/platform/posix/src/clock_gettime.c"
        	extra_obj="$extra_objs clock_gettime.o";;
*.linux.*)	# Some Linux boxes keep clock_gettime() in librt.
		extra_libs="$extra_libs -lrt";;
esac

# Some compilers support cc -M, but others do not.
case "$detected_cc" in
gcc|clang|decc)	mkdep="$cc -M";;
*)		mkdep="$nsync_base/tools/mkdep.sh $cc -E";;
esac
case "$detected_cc" in
lcc)	extra_c="$extra_c ../../platform/posix/src/time_rep.c"
	extra_objs="$extra_objs time_rep.o"
	pthread=
	cflags=
	ldflags=-lpthread
	opt=OPT=;;
tendracc)
	extra_c="$extra_c ../../platform/posix/src/time_rep.c"
	extra_objs="$extra_objs time_rep.o"
	pthread=
	cflags=
	ldflags=-lpthread
	opt=OPT=;;
pcc)	extra_c="$extra_c ../../platform/num_time/src/time_rep.c"
	extra_objs="$extra_objs time_rep.o"
	extra_cpp_flags="$extra_cpp_flags -I../../platform/num_time -DNSYNC_USE_INT_TIME=int64_t"
	pthread=-pthread
	cflags=
	ldflags=-pthread
	opt=;;
*)	extra_c="$extra_c ../../platform/posix/src/time_rep.c"
	extra_objs="$extra_objs time_rep.o"
	pthread=-pthread
	cflags=
	ldflags=-pthread
	opt=;;
esac

# Invoke command from command line, with various arguments inserted.
"$make" -e \
	"MKDEP=$mkdep" \
	"NSYNC_EXTRA_INC_DIR=$extra_cpp_flags" "NSYNC_EXTRA_PLATFORM_C=$extra_c" "PLATFORM_S=$extra_s" \
	"NSYNC_EXTRA_OBJS=$extra_objs" "NSYNC_OS=$detected_os" \
	"NSYNC_OS_TYPE=posix" "NSYNC_CC=$detected_cc" "NSYNC_ARCH=$detected_arch" \
	"NSYNC_PTHREAD=$pthread" \
	"PLATFORM_CFLAGS=$cflags" \
	"PLATFORM_LIBS=$extra_libs" \
	"PLATFORM_LDFLAGS=$ldflags" \
	$opt \
	${1+"$@"}
