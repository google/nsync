/* Copyright 2018 Google Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License. */

/* Define macros 
	detected_cc     compiler type  (e.g., gcc)
	detected_os     OS type  (e.g., netbsd)
	detected_arch   CPU type (e.g., mips)
	detected_lang   language (e.g., c, c11, c++, c++11)
   Each value is suffixed with an "X" to prevent the strings used conflicting
   with other predefined macros. */

#if defined(__clang__)
detected_cc=clangX
#elif defined(__LCC__)
detected_cc=lccX
#elif defined(__TINYC__)
detected_cc=tccX
#elif defined(__DECC)
detected_cc=deccX
#elif defined(__PCC__)
detected_cc=pccX
#elif defined(__TenDRA__)
detected_cc=tendraccX
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6))
detected_cc=gccX
#elif defined(__GNUC__)
detected_cc=gcc_oldX
#else
detected_cc=posixX
#endif

#if defined(__linux__)
detected_os=linuxX
#elif defined(__NetBSD__)
detected_os=netbsdX
#elif defined(__OpenBSD__)
detected_os=openbsdX
#elif defined(__FreeBSD__)
detected_os=freebsdX
#elif defined(__osf__)
detected_os=osf1X
#elif defined(__hpux__)
detected_os=hpuxX
#elif __ANDROID__
detected_os=androidX
#elif __APPLE__
#if TARGET_OS_IPHONE || TARGET_OS_IOS || TARGET_IPHONE_SIMULATOR
detected_os=iosX
#else
detected_os=macosX
#endif
#elif defined(__CYGWIN__)
detected_os=cygwinX
#elif defined(_WIN64)
detected_os=win32X
#elif defined(_WIN32)
detected_os=win32X
#else /* defined(_POSIX_VERSION) || defined(__unix__) */
detected_os=posixX
#endif

#if defined(__x86_64__) || defined(__amd64__)
detected_arch=x86_64X
#elif defined(__x86_32__) || defined(__i686__) || defined(__i586__) || defined(__i486__) || defined(__i386__)
detected_arch=x86_32X
#elif defined(__alpha__)
detected_arch=alphaX
#elif defined(__mips64__) || defined(__mips32__) || defined(__mips__)
detected_arch=mipsX
#elif defined(__ppc64__) || defined(__powerpc64__) || defined(__ppc__) || defined(__powerpc__)
detected_arch=ppcX
#elif defined(__riscv__)
detected_arch=riscvX
#elif defined(__pnacl__)
detected_arch=pnaclX
#elif defined(__sparc64__)
detected_arch=sparc64X
#elif defined(__sparc__)
detected_arch=sparcX
#elif defined(__hppa64__)
detected_arch=parisc64X
#elif defined(__hppa__)
detected_arch=pariscX
#elif defined(__s390__)
detected_arch=s390X
#elif defined(__s390x__)
detected_arch=s390xX
#elif defined(__mc68000__)
detected_arch=mc68000X
#elif defined(__aarch64__) || defined(__arm64__)
detected_arch=aarch64X
#elif defined(__arm32__) || defined(__arm__)
detected_arch=armX
#elif defined(__ia64__)
detected_arch=ia64X
#else
detected_arch=posixX
#endif

#if __cplusplus >= 201103L 
detected_lang=c++11X
#elif defined(__cplusplus)
detected_lang=c++X
#elif __STDC_VERSION__ >= 201112L
detected_lang=c11X
#else
detected_lang=cX
#endif
