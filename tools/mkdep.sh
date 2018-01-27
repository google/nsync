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

# Generate make dependencies using C preprocessor.
#  Usage:
#      mkdep.sh cc cc_args_to_invoke_preprocessor... file.{[csh],cc} ...
#  Output is written to stdout.

# Implemented with sh, expr, sed, and sort.

c_cmd=
cpp_cmd=
for x in ${1+"$@"}; do
	case "$x" in
	*.[chsCHS]|*.cc|*.CC) ;;
	-c++=*)	cpp_cmd="$cpp_cmd '`expr "$x" : '^.....\(.*\)$'`'";;
	*)	c_cmd="$c_cmd '$x'"
		cpp_cmd="$cpp_cmd '$x'";;
	esac
done

# Write to stdout dependencies obtained by running each source file through the
# C preprocessor using the command prefix from the command line.
for file in ${1+"$@"}; do
	case "$file" in
	*.[chsCHS]|*.cc|*.CC)
		case "$file" in
		*.cc|*.CC)  cmd="$cpp_cmd";;
		*)	    cmd="$c_cmd";;
		esac

		ext=`expr "$file" : '^.*[.]\([^.]*\)$'`
		basename=`expr "$file" : '^\(.*\)[.][^.]*$'`  # remove extension
		case "$basename" in
		*/*)	basename=`expr "$basename" : '.*/\([^/]*\)$'`;;
		esac

		eval "$cmd" "'$file'" |
			sed '
				s/^#line 1 "/# 1 "/
				/^# 1 "[^<]/!d
				s/^# 1 \(".*"\).*$/\1/
				/ /!s/^"\(.*\)"$/\1/
				p
			' | 
			sort -u |
			sed '1i\
'"$basename.o"': \\
				s/^/ /
				$!s/$/ \\/
			'
		;;
	esac
done
