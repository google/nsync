#!/bin/sh
# Copyright 2016 Google Inc.
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

# Attempt to create a subdirectory in builds/ suitable for
# building nsync on this machine.   Works on many Unix-like systems,
# but for some you should create the Makefile by hand.
#
# Requires a Bourne shell, echo, expr, mkdir, uname, rm,
# and, if the -diff option is used, diff.

usage="usage: mkmakefile.sh [-diff|-out] [-dir <build_dir>]
			[-os <os>] [-arch <arch>] [-cc <cc>]
			[-sem futex|sem_t|mutex]
			[-atomic c++11|c11|asm|os]
			[-lrt]

	Create a build directory <build_dir> for the given operating system,
	machine architecture, and C compiler.
	Values not given explicitly are guessed.

	If -sem is given, it forces the use of futex, sem_t, or
	pthread_{mutex,cond}_t to implement a semaphore.

        If -atomic is given, it requests the use of C++11, C11, OS-supplied, or
        assembler-implemented atomics.  Otherwise, atomics are usually supplied
        by compiler intrinsics.

        The flag -lrt indicates that librt is needed to provide the
        clock_gettime call.

	If -out is given, the makefile is written to stdout.
	If -diff is given, the makefile is compared with the
	one present in the directory.
"

# Compilers this script knows about.
compilers="gcc clang g++ clang++ tcc lcc decc pcc tendracc"

os=
arch=
cc=
dir=
action=create
sem=
libs=
atomics=

while [ $# -gt 0 ]; do
	arg="${1-}"
	case "$arg" in
	-arch)		arch="${2?"$usage"}"; shift;;
	-atomic)	atomics="${2?"$usage"}"; shift;;
	-cc)		cc="${2?"$usage"}"; shift;;
	-diff)		action=diff;;
	-dir)		dir="${2?"$usage"}"; shift;;
	-lrt)		case "$libs" in "") libs="-lrt";; *) libs="$libs -lrt";; esac;;
	-os)		os="${2?"$usage"}"; shift;;
	-out)		action=out;;
	-sem)		sem="${2?"$usage"}"; shift;;
	*)		echo "$usage" >&2; exit 2;;
	esac
	shift
done

# Find the root of the nsync tree.
for root_dir in . .. ../.. ../../.. ../../../..; do
	if [ -f "$root_dir/public/nsync.h" ]; then
		break
	fi
done
if [ ! -f "$root_dir/public/nsync.h" ]; then
	echo "must be invoked within the nsync tree" >&2
	exit 2
fi

# Find the normalized operating system name.
case "$os" in
"")	os=`uname -s`
	case "$os" in
	[lL][iI][nN][uU][xX])		os=linux;;
	[nN][eE][tT][bB][sS][dD])	os=netbsd;;
	[fF][rR][eE][eE][bB][sS][dD])	os=freebsd;;
	[oO][pP][eE][nN][bB][sS][dD])	os=openbsd;;
	[dD][aA][rR][wW][iI][nN])	os=macos;;
	[cC][yY][gG][wW][iI][nN]*)	os=cygwin;;
	[oO][sS][fF]1)			os=osf1;;
	[iI][rR][iI][xX]64)		os=irix64;;
	[uU][lL][tT][rR][iI][xX])	os=ultrix;;
	esac;;
esac

# Find the normalized processor architecture name.
case "$arch" in
"")	arch=`uname -m`
	case "$arch" in
	[iI][3456]86|[xX]86|[xX]86_32)	arch=x86_32;;
	[aA][mM][dD]64|[xX]86_64)	arch=x86_64;;
	[iI][pP]35)			arch=mips;;
	[rR][iI][sS][cC])		arch=mips;;
	[vV][aA][xX])			arch=vax;;
	esac;;
esac

# Find a C compiler.
case "$cc" in
"")	for x in cc $compilers; do
		path="$PATH"
		while [ -n "$path" -a ! -n "$cc" ]; do
			d=`expr "$path" : '\([^:]*\)'`
			path=`expr "$path" : '[^:]*:\(.*\)'`
			if [ -x "$d/$x" ]; then
				cc="$x"
			fi
		done
	done
	case "$cc" in
	"")	echo "can't find C compiler: use -cc flag" >&2
		exit 2;;
	esac;;
esac

# Work out what sort of C compiler it is.
cplusplus=
case "$cc" in *++*) cplusplus=c++;; esac
cc_type2="$cc"
cc_type=
case "$cc_type2" in *" "*) cc_type2=`expr "$cc_type2" : '\([^ ]*\) .*$'`;; esac
case "$cc_type2" in */*)   cc_type2=`expr "$cc_type2" : '[^\([^ ]*\) .*$'`;; esac
case "$cc_type2" in
cc)			if version=`cc -v 2>&1 || cc -V 2>&1`; then
				cc_type2=`echo "$version" | while read line; do
					case "$line" in
					"gcc "*)		echo gcc; break;;
					"Apple LLVM "*) 	echo clang; break;;
					"clang "*|*" clang "*)	echo clang; break;;
					"tcc "*)		echo tcc; break;;
					"lcc "*)		echo lcc; break;;
					"Compaq C "*|"DEC C "*)	echo decc; break;;
					esac
				    done`
			fi
			cc_type="$cc_type2";;
g++)			cc_type=gcc;;
clang++)		cc_type=clang;;
*)			cc_type="$cc_type2";;
esac

# Find a build directory.
case "$dir" in
"")	dir="$root_dir/builds/$arch.$os.$cc_type2"
	case "$sem" in ?*) dir="$dir.sem-$sem";; esac
	case "$atomics" in ?*) dir="$dir.atm-$atomics";; esac
	case "$libs" in *-lrt*) dir="$dir.lrt";; esac
	;;
esac

case "$action" in
create)	if [ -d "$dir" ]; then echo "$dir exists" >&2; exit 2; fi;;
diff)	if [ ! -d "$dir" ]; then echo "$dir doesn't exist" >&2; exit 2; fi;;
esac

# Get C preprocessor flags.
cppflags="-D_POSIX_C_SOURCE=200809L"  # Sometimes needed; doesn't hurt.
# gcc's TLS doesn't work on OpenBSD or Irix.
case "$arch.$os.$cc_type" in
*.openbsd.gcc|*.openbsd.clang|*.irix64.gcc)
	cppflags="$cppflags -I../../platform/gcc_no_tls";;
esac
first="$cc_type"
second="$os"
case "$atomics" in
c11)	cppflags="-DNSYNC_ATOMIC_C11 -I../../platform/c11 $cppflags";;
c++11)	cppflags="-I../../platform/atomic_ind $cppflags";;
os)	first="$os" second="$cc_type";;
esac
for x in "$first" "$second" "$arch" posix; do
	if [ -d "$root_dir/platform/$x" ]; then
		cppflags="$cppflags -I../../platform/$x"
	fi
done

# If not using gcc/clang, or explcitly requested, attempt to find an assembly
# language set of atomic ops.
asm_atomics=false
case "$cc_type.$atomics.$os" in
*.c11.*|*.c++11.*)	;;
gcc..irix64)		asm_atomics=true;;
gcc..*|clang..*)	;;
*..*|*.asm.*)		asm_atomics=true;;
esac
atomic_ind=
atomic_c=
atomic_s=
case "$asm_atomics" in
true)	for x in `cd "$root_dir"; echo "platform/$arch/src/nsync_atm_$arch."[csS]`; do
		if [ -f "$root_dir/$x" ]; then
			atomic_ind="-I../../platform/atomic_ind "
			case "$x" in
			*.[sS]) atomic_s="../../$x";;
			*)	atomic_c="../../$x";;
			esac
			break
		fi
	done;;
esac

# Some platforms take -pthread flag, and others -pthread.
ldflags=
use_pthread=false
case "$cc_type.$os" in
*.irix64)			;;
gcc.*|clang.*|tcc.*|decc.*|pcc.*)	use_pthread=true;;
esac
case "$use_pthread" in
true)	cppflags="$cppflags -pthread"
	case "$ldflags" in
	"")	ldflags="-pthread";;
	*)	ldflags="$ldflags -pthread";;
	esac;;
*)	case "$libs" in
	"")	libs="-lpthread";;
	*)	libs="$libs -lpthread";;
	esac;;
esac

# Which semaphore implementation to use.
case "$os.$sem" in
linux.|linux.futex)	semfile="../../platform/linux/src/nsync_semaphore_futex.c ";;
*.|*.futex)		semfile="../../platform/posix/src/nsync_semaphore_mutex.c ";;
*)			semfile="../../platform/posix/src/nsync_semaphore_$sem.c ";;
esac

# Some platforms don't have clock_gettime.
clock_gettime_src=
case "$os" in
macos)	clock_gettime_src="../../platform/posix/src/clock_gettime.c ";;
esac

case "$cc_type" in
pcc)	# Compilers like pcc can't pass/return structs properly.  Use an integer for the time.
	time_rep_src="../../platform/num_time/src/time_rep.c"
	cppflags="-DNSYNC_USE_INT_TIME=int64_t -I../../platform/num_time $cppflags";;
*)	time_rep_src="../../platform/posix/src/time_rep.c";;
esac

# Platform-specific files.
platform_c="$atomic_c$clock_gettime_src$semfile../../platform/posix/src/per_thread_waiter.c ../../platform/posix/src/yield.c $time_rep_src ../../platform/posix/src/nsync_panic.c"
platform_s="$atomic_s"
sp=
platform_o=
for x in $platform_s $platform_c; do
	o=`expr "$x" : '.*/\([^/]*\)[.][^/]*$'`.o
	platform_o="$platform_o$sp$o"
	sp=" "
done
test_platform_c="../../platform/posix/src/start_thread.c"
sp=
test_platform_o=
for x in $test_platform_c; do
	o=`expr "$x" : '.*/\([^/]*\)[.][^/]*$'`.o
	test_platform_o="$test_platform_o$sp$o"
	sp=" "
done

makefile=`
	case "$cc" in
	cc) ;;
	*)  echo "CC=$cc";;
	esac
	case "$cc_type.$cplusplus" in
	gcc.)		echo "PLATFORM_CPPFLAGS=$atomic_ind$cppflags"
			echo "PLATFORM_CFLAGS=-Werror -Wall -Wextra -ansi -pedantic"
			case "$ldflags" in ?*) echo "PLATFORM_LDFLAGS=$ldflags";; esac
			case "$libs" in ?*) echo "PLATFORM_LIBS=$libs";; esac
			case "$atomics" in
			c++11)	echo "PLATFORM_CXXFLAGS=-Werror -Wall -Wextra -std=c++11 -pedantic"
				echo "PLATFORM_CXX=../../platform/c_from_c++11/src/nsync_atm_c++.cc"
				echo "MKDEP_DEPEND=mkdep"
				echo 'MKDEP=./mkdep ${CC} -E -c++=-std=c++11'
				platform_o="nsync_atm_c++.o $platform_o";;
			*)	echo 'MKDEP=${CC} -M';;
			esac
			echo "PLATFORM_C=$platform_c"
			case "$platform_s" in ?*) echo "PLATFORM_S=$platform_s";; esac
			echo "PLATFORM_OBJS=$platform_o"
			echo "TEST_PLATFORM_C=$test_platform_c"
			echo "TEST_PLATFORM_OBJS=$test_platform_o"
			;;
	clang.)		echo "PLATFORM_CPPFLAGS=$atomic_ind$cppflags"
			echo "PLATFORM_CFLAGS=-Werror -Wall -Wextra -ansi -pedantic -Wno-unneeded-internal-declaration"
			case "$ldflags" in ?*) echo "PLATFORM_LDFLAGS=$ldflags";; esac
			case "$libs" in ?*) echo "PLATFORM_LIBS=$libs";; esac
			case "$atomics" in
			c++11)	echo "PLATFORM_CXXFLAGS=-Werror -Wall -Wextra -std=c++11 -pedantic"
				echo "PLATFORM_CXX=../../platform/c_from_c++11/src/nsync_atm_c++.cc"
				echo "MKDEP_DEPEND=mkdep"
				echo 'MKDEP=./mkdep ${CC} -E -c++=-std=c++11'
				platform_o="nsync_atm_c++.o $platform_o";;
			*)	echo 'MKDEP=${CC} -M';;
			esac
			echo "PLATFORM_C=$platform_c"
			case "$platform_s" in ?*) echo "PLATFORM_S=$platform_s";; esac
			echo "PLATFORM_OBJS=$platform_o"
			echo "TEST_PLATFORM_C=$test_platform_c"
			echo "TEST_PLATFORM_OBJS=$test_platform_o"
			;;
	gcc.c++)	echo "PLATFORM_CPPFLAGS=-DNSYNC_ATOMIC_CPP11 -I../../platform/c++11 $cppflags"
			echo "PLATFORM_CFLAGS=-x c++ -std=c++11 -Werror -Wall -Wextra -pedantic"
			case "$ldflags" in ?*) echo "PLATFORM_LDFLAGS=$ldflags";; esac
			case "$libs" in ?*) echo "PLATFORM_LIBS=$libs";; esac
			echo 'MKDEP=${CC} -M -x c++ -std=c++11'
			echo "PLATFORM_C=$platform_c"
			case "$platform_s" in ?*) echo "PLATFORM_S=$platform_s";; esac
			echo "PLATFORM_OBJS=$platform_o"
			echo "TEST_PLATFORM_C=$test_platform_c"
			echo "TEST_PLATFORM_OBJS=$test_platform_o"
			;;
	clang.c++)	echo "PLATFORM_CPPFLAGS=-DNSYNC_ATOMIC_CPP11 -I../../platform/c++11 $cppflags"
			echo "PLATFORM_CFLAGS=-x c++ -std=c++11 -Werror -Wall -Wextra -pedantic -Wno-unneeded-internal-declaration"
			case "$ldflags" in ?*) echo "PLATFORM_LDFLAGS=$ldflags";; esac
			case "$libs" in ?*) echo "PLATFORM_LIBS=$libs";; esac
			echo 'MKDEP=${CC} -M -x c++ -std=c++11'
			echo "PLATFORM_C=$platform_c"
			case "$platform_s" in ?*) echo "PLATFORM_S=$platform_s";; esac
			echo "PLATFORM_OBJS=$platform_o"
			echo "TEST_PLATFORM_C=$test_platform_c"
			echo "TEST_PLATFORM_OBJS=$test_platform_o"
			;;
	tcc.)		echo "PLATFORM_CPPFLAGS=$atomic_ind$cppflags"
			echo "PLATFORM_CFLAGS=-Werror -Wall"
			case "$ldflags" in ?*) echo "PLATFORM_LDFLAGS=$ldflags";; esac
			case "$libs" in ?*) echo "PLATFORM_LIBS=$libs";; esac
			echo "MKDEP_DEPEND=mkdep"
			echo 'MKDEP=./mkdep ${CC} -E'
			echo "PLATFORM_C=$platform_c"
			case "$platform_s" in ?*) echo "PLATFORM_S=$platform_s";; esac
			echo "PLATFORM_OBJS=$platform_o"
			echo "TEST_PLATFORM_C=$test_platform_c"
			echo "TEST_PLATFORM_OBJS=$test_platform_o"
			;;
	lcc.)		echo "PLATFORM_CPPFLAGS=$atomic_ind$cppflags"
			case "$libs" in ?*) echo "PLATFORM_LIBS=$libs";; esac
			echo "MKDEP_DEPEND=mkdep"
			echo 'MKDEP=./mkdep ${CC} -E'
			echo "PLATFORM_C=$platform_c"
			case "$platform_s" in ?*) echo "PLATFORM_S=$platform_s";; esac
			echo "PLATFORM_OBJS=$platform_o"
			echo "TEST_PLATFORM_C=$test_platform_c"
			echo "TEST_PLATFORM_OBJS=$test_platform_o"
			;;
	decc.)		echo "PLATFORM_CPPFLAGS=$atomic_ind$cppflags -Duintmax_t=uint64_t"
			echo "PLATFORM_CFLAGS=-std1"
			case "$ldflags" in ?*) echo "PLATFORM_LDFLAGS=$ldflags";; esac
			case "$libs" in
			?*) echo "PLATFORM_LIBS=$libs -lrt";;
			*)  echo "PLATFORM_LIBS=-lrt";;
			esac
			echo 'MKDEP=${CC} -M'
			echo "PLATFORM_C=$platform_c"
			case "$platform_s" in ?*) echo "PLATFORM_S=$platform_s";; esac
			echo "PLATFORM_OBJS=$platform_o"
			echo "TEST_PLATFORM_C=$test_platform_c"
			echo "TEST_PLATFORM_OBJS=$test_platform_o"
			;;
	pcc.)		echo "PLATFORM_CPPFLAGS=$atomic_ind$cppflags"
			case "$ldflags" in ?*) echo "PLATFORM_LDFLAGS=$ldflags";; esac
			case "$libs" in ?*) echo "PLATFORM_LIBS=$libs";; esac
			echo 'MKDEP=${CC} -M'
			echo "PLATFORM_C=$platform_c"
			case "$platform_s" in ?*) echo "PLATFORM_S=$platform_s";; esac
			echo "PLATFORM_OBJS=$platform_o"
			echo "TEST_PLATFORM_C=$test_platform_c"
			echo "TEST_PLATFORM_OBJS=$test_platform_o"
			;;
	*)		echo "PLATFORM_CPPFLAGS=$atomic_ind$cppflags"
			echo "PLATFORM_LIBS=$libs"
			echo "MKDEP_DEPEND=mkdep"
			echo 'MKDEP=./mkdep ${CC} -E'
			echo "PLATFORM_C=$platform_c"
			case "$platform_s" in ?*) echo "PLATFORM_S=$platform_s";; esac
			echo "PLATFORM_OBJS=$platform_o"
			echo "TEST_PLATFORM_C=$test_platform_c"
			echo "TEST_PLATFORM_OBJS=$test_platform_o"
			;;
	esac
	echo
	echo "include ../../platform/posix/make.common"
	echo "include dependfile"
`

case "$action" in
out)	echo "$makefile";;
diff)	echo "$makefile" | diff - "$dir/Makefile";;
create)	mkdir -p "$dir" || exit 2
	> "$dir/dependfile"
	echo "$makefile" > "$dir/Makefile"
	echo "Created $dir/Makefile";;
esac
