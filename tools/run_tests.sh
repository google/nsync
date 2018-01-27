#!/bin/sh

usage="usage: run_tests [-p <N>] [-n <N>] [-bB] test...
  -p <N>             runs <N> tests in parallel
  -n <N>             runs <N> subtests in parallel
  -b                 benchmarks are run if tests pass
  -B                 only benchmarks are run
  -x <cross-runner>  runs each test as <cross-runner> <test> ...

The purpose of the cross-runner is to allow a test cross-compiled
for another environment to be run in that environment, perhaps via
an interpreter, or remotely.

Benchmarks are not run in parallel."

parallel=1
subparallel=1
run_tests=true
run_benchmarks=false
crossrunner=
rc=0
tests=
while
	arg="$1"
	case "$arg" in
	-*)
		case "$arg" in -*b*) run_benchmarks=true;; esac
		case "$arg" in -*B*) run_benchmarks=true; run_tests=false;; esac
		case "$arg" in -*n*) subparallel="${2?"$usage"}"; shift;; esac
		case "$arg" in -*p*) parallel="${2?"$usage"}"; shift;; esac
		case "$arg" in -*x*) crossrunner="${2?"$usage"}"; shift;; esac
		case "$arg" in -*[!bBnpx]*) echo "$usage" >&2; exit 2;; esac
		;;
	"")	break;;
	*)	tests="$tests $arg";;
	esac
do
	shift
done

if $run_tests; then
	case $parallel in
	1)
		for x in $tests; do
			$crossrunner $x -n $subparallel
			trc=$?
			case $rc in 0) rc=$trc;; esac
		done
		;;
	*)
		tmp=/tmp/run_tests.$$
		w=1000
		r=1000

		for x in $tests; do
			while 
				while [ -s $tmp.rc.$r ]; do
					cat $tmp.o.$r
					case $rc in 0) rc=`cat $tmp.rc.$r`;; esac
					rm -f $tmp.o.$r $tmp.rc.$r
					r=`expr $r + 1`
				done
				i=$r
				p=0
				while [ $i -lt $w ]; do
					i=`expr $i + 1`
					if [ ! -s $tmp.rc.$i ]; then p=`expr $p + 1`; fi
				done
				[ $parallel -le $p ]
			do
				sleep 1
			done
			($crossrunner $x -n $subparallel > $tmp.o.$w 2>&1; echo $? > $tmp.rc.$w) &
			w=`expr $w + 1`
		done
		while 
			while [ -s $tmp.rc.$r ]; do
				cat $tmp.o.$r
				case $rc in 0) rc=`cat $tmp.rc.$r`;; esac
				rm -f $tmp.o.$r $tmp.rc.$r
				r=`expr $r + 1`
			done
			[ $r -lt $w ]
		do
			sleep 1
		done
		;;
	esac
fi

case $rc in
0)	;;
*)	run_benchmarks=false;;
esac
if $run_benchmarks; then
	for x in $tests; do
		$crossrunner $x -B
		trc=$?
		case $rc in 0) rc=$trc;; esac
	done
fi
exit $rc
