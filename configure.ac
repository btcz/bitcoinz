AC_PREREQ([2.69])
define(_CLIENT_VERSION_MAJOR, 2)
define(_CLIENT_VERSION_MINOR, 1)
define(_CLIENT_VERSION_REVISION, 0)
define(_CLIENT_VERSION_BUILD, 50)
define(_ZC_BUILD_VAL, m4_if(m4_eval(_CLIENT_VERSION_BUILD < 25), 1, m4_incr(_CLIENT_VERSION_BUILD), m4_eval(_CLIENT_VERSION_BUILD < 50), 1, m4_eval(_CLIENT_VERSION_BUILD - 24), m4_eval(_CLIENT_VERSION_BUILD == 50), 1, , m4_eval(_CLIENT_VERSION_BUILD - 50)))
define(_CLIENT_VERSION_SUFFIX, m4_if(m4_eval(_CLIENT_VERSION_BUILD < 25), 1, _CLIENT_VERSION_REVISION-beta$1, m4_eval(_CLIENT_VERSION_BUILD < 50), 1, _CLIENT_VERSION_REVISION-rc$1, m4_eval(_CLIENT_VERSION_BUILD == 50), 1, _CLIENT_VERSION_REVISION, _CLIENT_VERSION_REVISION-$1)))
define(_CLIENT_VERSION_IS_RELEASE, true)
define(_COPYRIGHT_YEAR, 2024)
AC_INIT([BitcoinZ],[_CLIENT_VERSION_MAJOR._CLIENT_VERSION_MINOR._CLIENT_VERSION_SUFFIX(_ZC_BUILD_VAL)],[https://github.com/btcz/bitcoinz/issues],[BitcoinZ])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([src/config/bitcoin-config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([build-aux/m4])

m4_ifndef([PKG_PROG_PKG_CONFIG], [m4_fatal([PKG_PROG_PKG_CONFIG macro not found. Please install pkg-config and re-run autogen.sh])])
PKG_PROG_PKG_CONFIG
if test "$PKG_CONFIG" = ""; then
  AC_MSG_ERROR([pkg-config not found])
fi

# When compiling with depends, the `PKG_CONFIG_PATH` and `PKG_CONFIG_LIBDIR` variables,
# being set in a `config.site` file, are not exported to let the `--config-cache` option
# work properly.
if test -n "$PKG_CONFIG_PATH"; then
  PKG_CONFIG="env PKG_CONFIG_PATH=$PKG_CONFIG_PATH $PKG_CONFIG"
fi
if test -n "$PKG_CONFIG_LIBDIR"; then
  PKG_CONFIG="env PKG_CONFIG_LIBDIR=$PKG_CONFIG_LIBDIR $PKG_CONFIG"
fi

BITCOIN_DAEMON_NAME=bitcoinzd
BITCOIN_TEST_NAME=test_bitcoinz
BITCOIN_CLI_NAME=bitcoinz-cli
BITCOIN_TX_NAME=bitcoinz-tx

dnl Unless the user specified ARFLAGS, force it to be cr
dnl This is also the default as-of libtool 2.4.7
AC_ARG_VAR([ARFLAGS], [Flags for the archiver, defaults to <cr> if not set])
if test "${ARFLAGS+set}" != "set"; then
  ARFLAGS="cr"
fi

AC_CANONICAL_HOST

AH_TOP([#ifndef BITCOIN_CONFIG_H])
AH_TOP([#define BITCOIN_CONFIG_H])
AH_BOTTOM([#endif //BITCOIN_CONFIG_H])

dnl Automake init set-up and checks
AM_INIT_AUTOMAKE([1.13 no-define subdir-objects foreign])

AM_MAINTAINER_MODE([enable])

dnl make the compilation flags quiet unless V=1 is used
AM_SILENT_RULES([yes])

dnl Compiler checks (here before libtool).
if test "${CXXFLAGS+set}" = "set"; then
  CXXFLAGS_overridden=yes
else
  CXXFLAGS_overridden=no
fi
AC_PROG_CXX

dnl libtool overrides
case $host in
  *mingw*)
     dnl By default, libtool for mingw refuses to link static libs into a dll for
     dnl fear of mixing pic/non-pic objects, and import/export complications. Since
     dnl we have those under control, re-enable that functionality.
     lt_cv_deplibs_check_method="pass_all"

     dnl Remove unwanted -DDLL_EXPORT from these variables.
     dnl We do not use this macro, but system headers may export unwanted symbols
     dnl if it's set.
     lt_cv_prog_compiler_pic="-DPIC"
     lt_cv_prog_compiler_pic_CXX="-DPIC"
  ;;
  *darwin*)
     dnl Because it prints a verbose warning, lld fails the following check
     dnl for "-Wl,-single_module" from libtool.m4:
     dnl   # If there is a non-empty error log, and "single_module"
     dnl   # appears in it, assume the flag caused a linker warning
     dnl "-single_module" works fine on ld64 and lld, so just bypass the test.
     dnl Failure to set this to "yes" causes libtool to use a very broken
     dnl link-line for shared libs.
     lt_cv_apple_cc_single_mod="yes"
  ;;
esac

dnl Require C++17 compiler (no GNU extensions)
AX_CXX_COMPILE_STDCXX([17], [noext], [mandatory])

dnl Unless the user specified OBJCXX, force it to be the same as CXX. This ensures
dnl that we get the same -std flags for both.
m4_ifdef([AC_PROG_OBJCXX],[
if test "${OBJCXX+set}" = ""; then
  OBJCXX="${CXX}"
fi
AC_PROG_OBJCXX
])

dnl OpenBSD ships with 2.4.2
LT_PREREQ([2.4.2])
dnl Libtool init checks.
LT_INIT([pic-only win32-dll])

dnl Check/return PATH for base programs.
AC_PATH_TOOL([AR], [ar])
AC_PATH_TOOL([GCOV], [gcov])
AC_PATH_TOOL([LLVM_COV], [llvm-cov])
AC_PATH_PROG([LCOV], [lcov])
dnl The minimum supported version is specified in .python-version and should be used if available, see doc/dependencies.md
AC_PATH_PROGS([PYTHON], [python3.9 python3.10 python3.11 python3.12 python3 python])
AC_PATH_PROG([GENHTML], [genhtml])
AC_PATH_PROG([GIT], [git])
AC_PATH_PROG([CCACHE], [ccache])
AC_PATH_PROG([XGETTEXT], [xgettext])
AC_PATH_PROG([HEXDUMP], [hexdump])
AC_PATH_PROG([READELF], [readelf])
AC_PATH_TOOL([OBJDUMP], [objdump])
AC_PATH_TOOL([OBJCOPY], [objcopy])
AC_PATH_PROG([DOXYGEN], [doxygen])
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])

AC_ARG_ENABLE([wallet],
  [AS_HELP_STRING([--disable-wallet],
  [disable wallet (enabled by default)])],
  [enable_wallet=$enableval],
  [enable_wallet=yes])

AC_ARG_ENABLE(tests,
  AS_HELP_STRING([--disable-tests],[do not compile tests (default is to compile)]),
  [use_tests=$enableval],
  [use_tests=yes])

AC_ARG_ENABLE(bench,
  AS_HELP_STRING([--disable-bench],[do not compile benchmarks (default is to compile)]),
  [use_bench=$enableval],
  [use_bench=yes])

AC_ARG_ENABLE([fuzz-main],
  [AS_HELP_STRING([--enable-fuzz-main],
  [Replace main() with a stub for fuzzing (default is no)])],
  [use_fuzz_main=$enableval],
  [use_fuzz_main=no])

AC_ARG_ENABLE([hardening],
  [AS_HELP_STRING([--disable-hardening],
  [do not attempt to harden the resulting executables (default is to harden)])],
  [use_hardening=$enableval],
  [use_hardening=yes])

AC_ARG_ENABLE([reduce-exports],
  [AS_HELP_STRING([--enable-reduce-exports],
  [attempt to reduce exported symbols in the resulting executables (default is no)])],
  [use_reduce_exports=$enableval],
  [use_reduce_exports=no])

AC_ARG_ENABLE([ccache],
  [AS_HELP_STRING([--disable-ccache],
  [do not use ccache for building (default is to use if found)])],
  [use_ccache=$enableval],
  [use_ccache=auto])

dnl Suppress warnings from external headers (e.g. Boost, Qt).
dnl May be useful if warnings from external headers clutter the build output
dnl too much, so that it becomes difficult to spot Bitcoin Core warnings
dnl or if they cause a build failure with --enable-werror.
AC_ARG_ENABLE([suppress-external-warnings],
  [AS_HELP_STRING([--disable-suppress-external-warnings],
                  [Do not suppress warnings from external headers (default is to suppress)])],
  [suppress_external_warnings=$enableval],
  [suppress_external_warnings=yes])

AC_ARG_ENABLE([lcov],
  [AS_HELP_STRING([--enable-lcov],
  [enable lcov testing (default is no)])],
  [use_lcov=$enableval],
  [use_lcov=no])

AC_ARG_ENABLE([zmq],
  [AS_HELP_STRING([--disable-zmq],
  [disable ZMQ notifications])],
  [use_zmq=$enableval],
  [use_zmq=yes])

AC_ARG_ENABLE(man,
    [AS_HELP_STRING([--disable-man],
                    [do not install man pages (default is to install)])],,
    enable_man=yes)
AM_CONDITIONAL([ENABLE_MAN], [test "$enable_man" != "no"])

dnl Enable debug
AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug],
                    [use compiler flags and macros suited for debugging (default is no)])],
    [enable_debug=$enableval],
    [enable_debug=no])

dnl Enable different -fsanitize options
AC_ARG_WITH([sanitizers],
    [AS_HELP_STRING([--with-sanitizers],
                    [comma separated list of extra sanitizers to build with (default is none enabled)])],
    [use_sanitizers=$withval])

dnl Turn warnings into errors
AC_ARG_ENABLE([werror],
    [AS_HELP_STRING([--enable-werror],
                    [Treat compiler warnings as errors (default is no)])],
    [enable_werror=$enableval],
    [enable_werror=no])

AC_LANG_PUSH([C++])

dnl Always set -g -O2 in our CXXFLAGS. Autoconf will try and set CXXFLAGS to "-g -O2" by default,
dnl so we suppress that (if CXXFLAGS hasn't been overridden by the user), given we are adding it
dnl ourselves.
CXXFLAGS="$CXXFLAGS -g -O2"

if test "$CXXFLAGS_overridden" = "no"; then
  CXXFLAGS=""
fi

dnl Check for a flag to turn compiler warnings into errors. This is helpful for checks which may
dnl appear to succeed because by default they merely emit warnings when they fail.
dnl
dnl Note that this is not necessarily a check to see if -Werror is supported, but rather to see if
dnl a compile with -Werror can succeed. This is important because the compiler may already be
dnl warning about something unrelated, for example about some path issue. If that is the case,
dnl -Werror cannot be used because all of those warnings would be turned into errors.
AX_CHECK_COMPILE_FLAG([-Werror], [CXXFLAG_WERROR="-Werror"], [CXXFLAG_WERROR=""])

dnl Check for a flag to turn linker warnings into errors. When flags are passed to linkers via the
dnl compiler driver using a -Wl,-foo flag, linker warnings may be swallowed rather than bubbling up.
dnl See note above, the same applies here as well.
dnl
dnl LDFLAG_WERROR Should only be used when testing -Wl,*
case $host in
 *darwin*)
    AX_CHECK_LINK_FLAG([-Wl,-fatal_warnings], [LDFLAG_WERROR="-Wl,-fatal_warnings"], [LDFLAG_WERROR=""])
    ;;
  *)
    AX_CHECK_LINK_FLAG([-Wl,--fatal-warnings], [LDFLAG_WERROR="-Wl,--fatal-warnings"], [LDFLAG_WERROR=""])
    ;;
esac

if test "$enable_debug" = "yes"; then

  dnl Disable all optimizations
  AX_CHECK_COMPILE_FLAG([-O0], [DEBUG_CXXFLAGS="$DEBUG_CXXFLAGS -O0"], [], [$CXXFLAG_WERROR])

  dnl Prefer -g3, fall back to -g if that is unavailable.
  AX_CHECK_COMPILE_FLAG(
    [-g3],
    [DEBUG_CXXFLAGS="$DEBUG_CXXFLAGS -g3"],
    [AX_CHECK_COMPILE_FLAG([-g], [DEBUG_CXXFLAGS="$DEBUG_CXXFLAGS -g"], [], [$CXXFLAG_WERROR])],
    [$CXXFLAG_WERROR])

  AX_CHECK_PREPROC_FLAG([-DDEBUG], [DEBUG_CPPFLAGS="$DEBUG_CPPFLAGS -DDEBUG"], [], [$CXXFLAG_WERROR])
  AX_CHECK_PREPROC_FLAG([-DDEBUG_LOCKORDER], [DEBUG_CPPFLAGS="$DEBUG_CPPFLAGS -DDEBUG_LOCKORDER"], [], [$CXXFLAG_WERROR])
  AX_CHECK_PREPROC_FLAG([-DDEBUG_LOCKCONTENTION], [DEBUG_CPPFLAGS="$DEBUG_CPPFLAGS -DDEBUG_LOCKCONTENTION"], [], [$CXXFLAG_WERROR])
  AX_CHECK_PREPROC_FLAG([-DRPC_DOC_CHECK], [DEBUG_CPPFLAGS="$DEBUG_CPPFLAGS -DRPC_DOC_CHECK"], [], [$CXXFLAG_WERROR])
  AX_CHECK_PREPROC_FLAG([-DABORT_ON_FAILED_ASSUME], [DEBUG_CPPFLAGS="$DEBUG_CPPFLAGS -DABORT_ON_FAILED_ASSUME"], [], [$CXXFLAG_WERROR])
  AX_CHECK_COMPILE_FLAG([-ftrapv], [DEBUG_CXXFLAGS="$DEBUG_CXXFLAGS -ftrapv"], [], [$CXXFLAG_WERROR])
fi

if test "$use_sanitizers" != ""; then
  dnl First check if the compiler accepts flags. If an incompatible pair like
  dnl -fsanitize=address,thread is used here, this check will fail. This will also
  dnl fail if a bad argument is passed, e.g. -fsanitize=undfeined
  AX_CHECK_COMPILE_FLAG(
    [-fsanitize=$use_sanitizers],
    [SANITIZER_CXXFLAGS="-fsanitize=$use_sanitizers"
     SANITIZER_CFLAGS="-fsanitize=$use_sanitizers"],
    [AC_MSG_ERROR([compiler did not accept requested flags])])

  dnl Some compilers (e.g. GCC) require additional libraries like libasan,
  dnl libtsan, libubsan, etc. Make sure linking still works with the sanitize
  dnl flag. This is a separate check so we can give a better error message when
  dnl the sanitize flags are supported by the compiler but the actual sanitizer
  dnl libs are missing.
  AX_CHECK_LINK_FLAG(
    [-fsanitize=$use_sanitizers],
    [SANITIZER_LDFLAGS="-fsanitize=$use_sanitizers"],
    [AC_MSG_ERROR([linker did not accept requested flags, you are missing required libraries])],
    [],
    [AC_LANG_PROGRAM([[
    #include <cstdint>
    #include <cstddef>
    extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) { return 0; }
    __attribute__((weak)) // allow for libFuzzer linking
    ]],[[]])])
fi

ERROR_CXXFLAGS=
if test "$enable_werror" = "yes"; then
  if test "$CXXFLAG_WERROR" = ""; then
    AC_MSG_ERROR([enable-werror set but -Werror is not usable])
  fi
  ERROR_CXXFLAGS=$CXXFLAG_WERROR
fi

AX_CHECK_COMPILE_FLAG([-Wall], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wall"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wextra], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wextra"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wgnu], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wgnu"], [], [$CXXFLAG_WERROR])
dnl some compilers will ignore -Wformat-security without -Wformat, so just combine the two here.
AX_CHECK_COMPILE_FLAG([-Wformat -Wformat-security], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wformat -Wformat-security"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wvla], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wvla"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wshadow-field], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wshadow-field"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wthread-safety], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wthread-safety"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wloop-analysis], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wloop-analysis"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wredundant-decls], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wredundant-decls"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wunused-member-function], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wunused-member-function"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wdate-time], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wdate-time"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wconditional-uninitialized], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wconditional-uninitialized"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wduplicated-branches], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wduplicated-branches"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wduplicated-cond], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wduplicated-cond"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wlogical-op], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wlogical-op"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Woverloaded-virtual], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Woverloaded-virtual"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wsuggest-override], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wsuggest-override"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wimplicit-fallthrough], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wimplicit-fallthrough"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wunreachable-code], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wunreachable-code"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wdocumentation], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wdocumentation"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wself-assign], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wself-assign"], [], [$CXXFLAG_WERROR])
AX_CHECK_COMPILE_FLAG([-Wundef], [WARN_CXXFLAGS="$WARN_CXXFLAGS -Wundef"], [], [$CXXFLAG_WERROR])

dnl Some compilers (gcc) ignore unknown -Wno-* options, but warn about all
dnl unknown options if any other warning is produced. Test the -Wfoo case, and
dnl set the -Wno-foo case if it works.
AX_CHECK_COMPILE_FLAG([-Wunused-parameter], [NOWARN_CXXFLAGS="$NOWARN_CXXFLAGS -Wno-unused-parameter"], [], [$CXXFLAG_WERROR])

dnl Don't allow extended (non-ASCII) symbols in identifiers. This is easier for code review.
AX_CHECK_COMPILE_FLAG([-fno-extended-identifiers], [CXXFLAGS="$CXXFLAGS -fno-extended-identifiers"], [], [$CXXFLAG_WERROR])

dnl Currently all versions of gcc are subject to a class of bugs, see the
dnl gccbug_90348 test case (only reproduces on GCC 11 and earlier) and
dnl https://gcc.gnu.org/bugzilla/show_bug.cgi?id=111843. To work around that, set
dnl -fstack-reuse=none for all gcc builds. (Only gcc understands this flag)
AX_CHECK_COMPILE_FLAG([-fstack-reuse=none], [CXXFLAGS="$CXXFLAGS -fstack-reuse=none"])

enable_arm_crc=no
enable_sse42=no

dnl Check for optional instruction set support. Enabling these does _not_ imply that all code will
dnl be compiled with them, rather that specific objects/libs may use them after checking for runtime
dnl compatibility.

dnl x86
AX_CHECK_COMPILE_FLAG([-msse4.2], [SSE42_CXXFLAGS="-msse4.2"], [], [$CXXFLAG_WERROR])

TEMP_CXXFLAGS="$CXXFLAGS"
CXXFLAGS="$SSE42_CXXFLAGS $CXXFLAGS"
AC_MSG_CHECKING([for SSE4.2 intrinsics])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include <stdint.h>
    #if defined(_MSC_VER)
    #include <intrin.h>
    #elif defined(__GNUC__) && defined(__SSE4_2__)
    #include <nmmintrin.h>
    #endif
  ]],[[
    uint64_t l = 0;
    l = _mm_crc32_u8(l, 0);
    l = _mm_crc32_u32(l, 0);
    l = _mm_crc32_u64(l, 0);
    return l;
  ]])],
 [ AC_MSG_RESULT([yes]); enable_sse42=yes],
 [ AC_MSG_RESULT([no])]
)
CXXFLAGS="$TEMP_CXXFLAGS"

# ARM
AX_CHECK_COMPILE_FLAG([-march=armv8-a+crc+crypto], [ARM_CRC_CXXFLAGS="-march=armv8-a+crc+crypto"], [], [$CXXFLAG_WERROR])

TEMP_CXXFLAGS="$CXXFLAGS"
CXXFLAGS="$ARM_CRC_CXXFLAGS $CXXFLAGS"
AC_MSG_CHECKING([for ARMv8 CRC32 intrinsics])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include <arm_acle.h>
    #include <arm_neon.h>
  ]],[[
#ifdef __aarch64__
    __crc32cb(0, 0); __crc32ch(0, 0); __crc32cw(0, 0); __crc32cd(0, 0);
    vmull_p64(0, 0);
#else
#error "crc32c library does not support hardware acceleration on 32-bit ARM"
#endif
  ]])],
 [ AC_MSG_RESULT([yes]); enable_arm_crc=yes; ],
 [ AC_MSG_RESULT([no])]
)
CXXFLAGS="$TEMP_CXXFLAGS"

CPPFLAGS="$CPPFLAGS -DHAVE_BUILD_INFO"

AC_ARG_WITH([utils],
  [AS_HELP_STRING([--with-utils],
  [build bitcoinz-cli bitcoinz-tx (default=yes)])],
  [build_bitcoin_utils=$withval],
  [build_bitcoin_utils=yes])

AC_ARG_WITH([daemon],
  [AS_HELP_STRING([--with-daemon],
  [build bitcoinzd daemon (default=yes)])],
  [build_bitcoind=$withval],
  [build_bitcoind=yes])

AC_PATH_PROG([CARGO], [cargo], [none])
if test "$CARGO" = "none"; then
  AC_MSG_ERROR("cargo required to enable rust support.")
fi

AC_PATH_PROG([RUSTC], [rustc], [none])
if test "$RUSTC" = "none"; then
  AC_MSG_ERROR("rustc required to enable rust support.")
fi

CARGO_FLAGS="$CARGO_FLAGS --lib --verbose --release --locked"

case $host in
  ${host_cpu}-w64-mingw*) RUST_TARGET=${host_cpu}-pc-windows-gnu ;;
  ${host_cpu}-pc-linux*) RUST_TARGET=${host_cpu}-unknown-${host_os} ;;
  ${host_cpu}-apple-darwin*) RUST_TARGET=${host_cpu}-apple-darwin ;;
  *) RUST_TARGET=${host} ;;
esac

if test x$RUST_VENDORED_SOURCES != x; then
  AC_CONFIG_FILES([.cargo/config.toml:.cargo/config.toml.in])
  CARGO_FLAGS="$CARGO_FLAGS --offline"
fi

case $host in
  *mingw*)
     TARGET_OS=windows
     AC_CHECK_LIB([kernel32], [GetModuleFileNameA],      [], [AC_MSG_ERROR([libkernel32 missing])])
     AC_CHECK_LIB([user32],   [main],                    [], [AC_MSG_ERROR([libuser32 missing])])
     AC_CHECK_LIB([gdi32],    [main],                    [], [AC_MSG_ERROR([libgdi32 missing])])
     AC_CHECK_LIB([comdlg32], [main],                    [], [AC_MSG_ERROR([libcomdlg32 missing])])
     AC_CHECK_LIB([winmm],    [main],                    [], [AC_MSG_ERROR([libwinmm missing])])
     AC_CHECK_LIB([shell32],  [SHGetSpecialFolderPathW], [], [AC_MSG_ERROR([libshell32 missing])])
     AC_CHECK_LIB([comctl32], [main],                    [], [AC_MSG_ERROR([libcomctl32 missing])])
     AC_CHECK_LIB([ole32],    [CoCreateInstance],        [], [AC_MSG_ERROR([libole32 missing])])
     AC_CHECK_LIB([oleaut32], [main],                    [], [AC_MSG_ERROR([liboleaut32 missing])])
     AC_CHECK_LIB([uuid],     [main],                    [], [AC_MSG_ERROR([libuuid missing])])
     AC_CHECK_LIB([advapi32], [CryptAcquireContextW],    [], [AC_MSG_ERROR([libadvapi32 missing])])
     AC_CHECK_LIB([ws2_32],   [WSAStartup],              [], [AC_MSG_ERROR([libws2_32 missing])])
     AC_CHECK_LIB([shlwapi],  [PathRemoveFileSpecW],     [], [AC_MSG_ERROR([libshlwapi missing])])
     AC_CHECK_LIB([iphlpapi], [GetAdaptersAddresses],    [], [AC_MSG_ERROR([libiphlpapi missing])])
     AC_CHECK_LIB([ntdll],    [main],                    [], [AC_MSG_ERROR([libntdll missing])])
     AC_CHECK_LIB([bcrypt],   [main],                    [], [AC_MSG_ERROR([libbcrypt missing])])

     dnl -static is interpreted by libtool, where it has a different meaning.
     dnl In libtool-speak, it's -all-static.
     AX_CHECK_LINK_FLAG([-static], [LIBTOOL_APP_LDFLAGS="$LIBTOOL_APP_LDFLAGS -all-static"])

     AC_PATH_PROG([MAKENSIS], [makensis], [none])
     if test "$MAKENSIS" = "none"; then
       AC_MSG_WARN([makensis not found. Cannot create installer.])
     fi

     AC_PATH_TOOL([WINDRES], [windres], [none])
     if test "$WINDRES" = "none"; then
       AC_MSG_ERROR([windres not found])
     fi

     CPPFLAGS="$CPPFLAGS -DSECP256K1_STATIC"

     CPPFLAGS="$CPPFLAGS -D_MT -DWIN32 -D_WINDOWS -D_WIN32_WINNT=0x0601 -D_WIN32_IE=0x0501 -DWIN32_LEAN_AND_MEAN"
     dnl Prevent the definition of min/max macros.
     dnl We always want to use the standard library.
     CPPFLAGS="$CPPFLAGS -DNOMINMAX"

     dnl libtool insists upon adding -nostdlib and a list of objects/libs to link against.
     dnl That breaks our ability to build dll's with static libgcc/libstdc++/libssp. Override
     dnl its command here, with the predeps/postdeps removed, and -static inserted. Postdeps are
     dnl also overridden to prevent their insertion later.
     dnl This should only affect dll's.
     archive_cmds_CXX="\$CC -shared \$libobjs \$deplibs \$compiler_flags -static -o \$output_objdir/\$soname \${wl}--enable-auto-image-base -Xlinker --out-implib -Xlinker \$lib"
     postdeps_CXX=

     dnl We require Windows 7 (NT 6.1) or later
     AX_CHECK_LINK_FLAG([-Wl,--major-subsystem-version -Wl,6 -Wl,--minor-subsystem-version -Wl,1], [LDFLAGS="$LDFLAGS -Wl,--major-subsystem-version -Wl,6 -Wl,--minor-subsystem-version -Wl,1"], [], [$LDFLAG_WERROR])

     dnl Avoid the use of aligned vector instructions when building for Windows.
     dnl See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54412.
     AX_CHECK_COMPILE_FLAG([-Wa,-muse-unaligned-vector-move], [CXXFLAGS="$CXXFLAGS -Wa,-muse-unaligned-vector-move"], [], [$CXXFLAG_WERROR])
     ;;
  *darwin*)
     TARGET_OS=darwin
     if  test $cross_compiling != "yes"; then
       BUILD_OS=darwin

       AX_CHECK_LINK_FLAG([-Wl,-headerpad_max_install_names], [LDFLAGS="$LDFLAGS -Wl,-headerpad_max_install_names"], [], [$LDFLAG_WERROR])

       AC_CHECK_PROG([BREW], [brew], [brew])
       if test "$BREW" = "brew"; then
         dnl These Homebrew packages may be keg-only, meaning that they won't be found
         dnl in expected paths because they may conflict with system files. Ask
         dnl Homebrew where each one is located, then adjust paths accordingly.
         dnl It's safe to add these paths even if the functionality is disabled by
         dnl the user (--without-wallet or --without-gui for example).

         dnl Homebrew may create symlinks in /usr/local/include for some packages.
         dnl Because MacOS's clang internally adds "-I /usr/local/include" to its search
         dnl paths, this will negate efforts to use -isystem for those packages, as they
         dnl will be found first in /usr/local. Use the internal "-internal-isystem"
         dnl option to system-ify all /usr/local/include paths without adding it to the list
         dnl of search paths in case it's not already there.
         if test "$suppress_external_warnings" != "no"; then
           AX_CHECK_PREPROC_FLAG([-Xclang -internal-isystem -Xclang /usr/local/include/], [CPPFLAGS="$CPPFLAGS -Xclang -internal-isystem -Xclang /usr/local/include/"], [], [$CXXFLAG_WERROR])
         fi

         if test $BREW list --versions berkeley-db@4 >/dev/null && test "$BDB_CFLAGS" = "" && test "$BDB_LIBS" = ""; then
           bdb_prefix=$($BREW --prefix berkeley-db@4 2>/dev/null)
           dnl This must precede the call to BITCOIN_FIND_BDB62 below.
           BDB_CFLAGS="-I$bdb_prefix/include"
           BDB_LIBS="-L$bdb_prefix/lib -ldb_cxx-4.8"
         fi

         if $BREW list --versions qt@5 >/dev/null; then
           export PKG_CONFIG_PATH="$($BREW --prefix qt@5 2>/dev/null)/lib/pkgconfig:$PKG_CONFIG_PATH"
         fi
       fi
     else
       case $build_os in
         *darwin*)
           BUILD_OS=darwin
           ;;
         *)
           AC_PATH_TOOL([DSYMUTIL], [dsymutil], [dsymutil])
           AC_PATH_PROG([ZIP], [zip], [zip])

           dnl libtool will try to strip the static lib, which is a problem for
           dnl cross-builds because strip attempts to call a hard-coded ld,
           dnl which may not exist in the path. Stripping the .a is not
           dnl necessary, so just disable it.
           old_striplib=
           ;;
       esac
     fi

     CPPFLAGS="$CPPFLAGS -DMAC_OSX -DOBJC_OLD_DISPATCH_PROTOTYPES=0"

     dnl ignore deprecated-declarations warnings coming from objcxx code
     dnl "'NSUserNotificationCenter' is deprecated: first deprecated in macOS 11.0".
     OBJCXXFLAGS="$CXXFLAGS -Wno-deprecated-declarations"
     ;;
   *linux*)
     TARGET_OS=linux
     ;;
esac

if test "$use_lcov" = "yes"; then
  if test "$LCOV" = ""; then
    AC_MSG_ERROR([lcov testing requested but lcov not found])
  fi
  if test "$PYTHON" = ""; then
    AC_MSG_ERROR([lcov testing requested but python not found])
  fi
  if test "$GENHTML" = ""; then
    AC_MSG_ERROR([lcov testing requested but genhtml not found])
  fi

  AC_MSG_CHECKING([whether compiler is Clang])
  AC_PREPROC_IFELSE([AC_LANG_SOURCE([[
      #if defined(__clang__) && defined(__llvm__)
      // Compiler is Clang
      #else
      #  error Compiler is not Clang
      #endif
    ]])],[
      AC_MSG_RESULT([yes])
      if test "$LLVM_COV" = ""; then
        AC_MSG_ERROR([lcov testing requested but llvm-cov not found])
      fi
      COV_TOOL="$LLVM_COV gcov"
    ],[
      AC_MSG_RESULT([no])
      if test "$GCOV" = "x"; then
        AC_MSG_ERROR([lcov testing requested but gcov not found])
      fi
      COV_TOOL="$GCOV"
  ])
  AC_SUBST(COV_TOOL)
  AC_SUBST(COV_TOOL_WRAPPER, "cov_tool_wrapper.sh")
  LCOV="$LCOV --gcov-tool $(pwd)/$COV_TOOL_WRAPPER"

  AX_CHECK_LINK_FLAG([--coverage], [LDFLAGS="$LDFLAGS --coverage"],
    [AC_MSG_ERROR([lcov testing requested but --coverage linker flag does not work])])
  AX_CHECK_COMPILE_FLAG([--coverage],[CXXFLAGS="$CXXFLAGS --coverage"],
    [AC_MSG_ERROR([lcov testing requested but --coverage flag does not work])])
  CXXFLAGS="$CXXFLAGS -Og"

  AC_SUBST(LCOV_OPTS)
fi

dnl Check for endianness
AC_C_BIGENDIAN

dnl Check for pthread compile/link requirements
AX_PTHREAD

dnl Check if -latomic is required for <std::atomic>
CHECK_ATOMIC

dnl The following macro will add the necessary defines to bitcoin-config.h, but
dnl they also need to be passed down to any subprojects. Pull the results out of
dnl the cache and add them to CPPFLAGS.
AC_SYS_LARGEFILE
dnl detect POSIX or GNU variant of strerror_r
AC_FUNC_STRERROR_R

if test "$ac_cv_sys_file_offset_bits" != "" &&
   test "$ac_cv_sys_file_offset_bits" != "no" &&
   test "$ac_cv_sys_file_offset_bits" != "unknown"; then
  CPPFLAGS="$CPPFLAGS -D_FILE_OFFSET_BITS=$ac_cv_sys_file_offset_bits"
fi

if test "$ac_cv_sys_large_files" != "" &&
   test "$ac_cv_sys_large_files" != "no" &&
   test "$ac_cv_sys_large_files" != "unknown"; then
  CPPFLAGS="$CPPFLAGS -D_LARGE_FILES=$ac_cv_sys_large_files"
fi

if test "$TARGET_OS" != "windows"; then
  dnl All windows code is PIC, forcing it on just adds useless compile warnings
  AX_CHECK_COMPILE_FLAG([-fPIC], [PIC_FLAGS="-fPIC"])
fi

if test x$use_fuzz_main == xyes; then
  CFLAGS="$CFLAGS -DZCASH_FUZZ=1"
  CXXFLAGS="$CXXFLAGS -DZCASH_FUZZ=1"
fi

if test "$use_hardening" != "no"; then
  AX_CHECK_COMPILE_FLAG([-Wstack-protector], [HARDENED_CXXFLAGS="$HARDENED_CXXFLAGS -Wstack-protector"])
  AX_CHECK_COMPILE_FLAG([-fstack-protector-all], [HARDENED_CXXFLAGS="$HARDENED_CXXFLAGS -fstack-protector-all"])

  AX_CHECK_COMPILE_FLAG([-fcf-protection=full], [HARDENED_CXXFLAGS="$HARDENED_CXXFLAGS -fcf-protection=full"])

  case $host in
    *mingw*)
      dnl stack-clash-protection doesn't compile with GCC 10 and earlier.
      dnl In any case, it is a no-op for Windows.
      dnl See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=90458 for more details.
      ;;
    *)
      AX_CHECK_COMPILE_FLAG([-fstack-clash-protection], [HARDENED_CXXFLAGS="$HARDENED_CXXFLAGS -fstack-clash-protection"], [], [$CXXFLAG_WERROR])
      ;;
  esac

  case $host in
    *aarch64*)
      AX_CHECK_COMPILE_FLAG([-mbranch-protection=bti], [HARDENED_CXXFLAGS="$HARDENED_CXXFLAGS -mbranch-protection=bti"])
    ;;
  esac

  dnl When enable_debug is yes, all optimizations are disabled.
  dnl However, FORTIFY_SOURCE requires that there is some level of optimization, otherwise it does nothing and just creates a compiler warning.
  dnl Since FORTIFY_SOURCE is a no-op without optimizations, do not enable it when enable_debug is yes.
  if test "$enable_debug" != "yes"; then
    AX_CHECK_PREPROC_FLAG([-D_FORTIFY_SOURCE=3],[
      AX_CHECK_PREPROC_FLAG([-U_FORTIFY_SOURCE],[
        HARDENED_CPPFLAGS="$HARDENED_CPPFLAGS -U_FORTIFY_SOURCE"
      ])
      HARDENED_CPPFLAGS="$HARDENED_CPPFLAGS -D_FORTIFY_SOURCE=3"
    ])
  fi

  AX_CHECK_LINK_FLAG([-Wl,--enable-reloc-section], [HARDENED_LDFLAGS="$HARDENED_LDFLAGS -Wl,--enable-reloc-section"], [], [$LDFLAG_WERROR])
  AX_CHECK_LINK_FLAG([-Wl,--dynamicbase], [HARDENED_LDFLAGS="$HARDENED_LDFLAGS -Wl,--dynamicbase"], [], [$LDFLAG_WERROR])
  AX_CHECK_LINK_FLAG([-Wl,--nxcompat], [HARDENED_LDFLAGS="$HARDENED_LDFLAGS -Wl,--nxcompat"], [], [$LDFLAG_WERROR])
  AX_CHECK_LINK_FLAG([-Wl,--high-entropy-va], [HARDENED_LDFLAGS="$HARDENED_LDFLAGS -Wl,--high-entropy-va"], [], [$LDFLAG_WERROR])
  AX_CHECK_LINK_FLAG([-Wl,-z,relro], [HARDENED_LDFLAGS="$HARDENED_LDFLAGS -Wl,-z,relro"], [], [$LDFLAG_WERROR])
  AX_CHECK_LINK_FLAG([-Wl,-z,now], [HARDENED_LDFLAGS="$HARDENED_LDFLAGS -Wl,-z,now"], [], [$LDFLAG_WERROR])
  AX_CHECK_LINK_FLAG([-Wl,-z,separate-code], [HARDENED_LDFLAGS="$HARDENED_LDFLAGS -Wl,-z,separate-code"], [], [$LDFLAG_WERROR])
  AX_CHECK_LINK_FLAG([-fPIE -pie], [PIE_FLAGS="-fPIE"; HARDENED_LDFLAGS="$HARDENED_LDFLAGS -pie"], [], [$CXXFLAG_WERROR])
fi

dnl These flags are specific to ld64, and may cause issues with other linkers.
dnl For example: GNU ld will interpret -dead_strip as -de and then try and use
dnl "ad_strip" as the symbol for the entry point.
if test "$TARGET_OS" = "darwin"; then
  AX_CHECK_LINK_FLAG([-Wl,-dead_strip], [LDFLAGS="$LDFLAGS -Wl,-dead_strip"], [], [$LDFLAG_WERROR])
  AX_CHECK_LINK_FLAG([-Wl,-dead_strip_dylibs], [LDFLAGS="$LDFLAGS -Wl,-dead_strip_dylibs"], [], [$LDFLAG_WERROR])
  AX_CHECK_LINK_FLAG([-Wl,-fixup_chains], [HARDENED_LDFLAGS="$HARDENED_LDFLAGS -Wl,-fixup_chains"], [], [$LDFLAG_WERROR])
fi

AC_CHECK_HEADERS([sys/select.h sys/prctl.h vm/vm_param.h sys/vmmeter.h sys/resources.h])

AC_CHECK_DECLS([getifaddrs, freeifaddrs],[CHECK_SOCKET],,
    [#include <sys/types.h>
    #include <ifaddrs.h>]
)

dnl These are used for daemonization in bitcoind
AC_CHECK_DECLS([fork])
AC_CHECK_DECLS([setsid])

AC_CHECK_DECLS([pipe2])

AC_CHECK_HEADERS([endian.h sys/endian.h byteswap.h stdio.h stdlib.h unistd.h strings.h sys/types.h sys/stat.h sys/select.h sys/prctl.h])
AC_SEARCH_LIBS([getaddrinfo_a], [anl], [AC_DEFINE(HAVE_GETADDRINFO_A, 1, [Define this symbol if you have getaddrinfo_a])])
AC_SEARCH_LIBS([inet_pton], [nsl resolv], [AC_DEFINE(HAVE_INET_PTON, 1, [Define this symbol if you have inet_pton])])

AC_CHECK_DECLS([le16toh, le32toh, le64toh, htole16, htole32, htole64, be16toh, be32toh, be64toh, htobe16, htobe32, htobe64],,,
		[#if HAVE_ENDIAN_H
                 #include <endian.h>
                 #elif HAVE_SYS_ENDIAN_H
                 #include <sys/endian.h>
                 #endif])

AC_CHECK_DECLS([bswap_16, bswap_32, bswap_64],,,
		[#if HAVE_BYTESWAP_H
                 #include <byteswap.h>
                 #endif])

AC_MSG_CHECKING([for fdatasync])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <unistd.h>]],
 [[ fdatasync(0); ]])],
 [ AC_MSG_RESULT([yes]); HAVE_FDATASYNC=1 ],
 [ AC_MSG_RESULT([no]); HAVE_FDATASYNC=0 ]
)
AC_DEFINE_UNQUOTED([HAVE_FDATASYNC], [$HAVE_FDATASYNC], [Define to 1 if fdatasync is available.])

AC_MSG_CHECKING([for F_FULLFSYNC])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <fcntl.h>]],
 [[ fcntl(0, F_FULLFSYNC, 0); ]])],
 [ AC_MSG_RESULT([yes]); HAVE_FULLFSYNC=1 ],
 [ AC_MSG_RESULT([no]); HAVE_FULLFSYNC=0 ]
)

AC_MSG_CHECKING([for O_CLOEXEC])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <fcntl.h>]],
 [[ open("", O_CLOEXEC); ]])],
 [ AC_MSG_RESULT([yes]); HAVE_O_CLOEXEC=1 ],
 [ AC_MSG_RESULT([no]); HAVE_O_CLOEXEC=0 ]
)
AC_DEFINE_UNQUOTED([HAVE_O_CLOEXEC], [$HAVE_O_CLOEXEC], [Define to 1 if O_CLOEXEC flag is available.])

dnl crc32c platform checks
AC_MSG_CHECKING([for __builtin_prefetch])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[ ]], [[
  char data = 0;
  const char* address = &data;
  __builtin_prefetch(address, 0, 0);
  ]])],
 [ AC_MSG_RESULT([yes]); HAVE_BUILTIN_PREFETCH=1 ],
 [ AC_MSG_RESULT([no]); HAVE_BUILTIN_PREFETCH=0 ]
)

AC_MSG_CHECKING([for _mm_prefetch])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <xmmintrin.h>]], [[
  char data = 0;
  const char* address = &data;
  _mm_prefetch(address, _MM_HINT_NTA);
  ]])],
 [ AC_MSG_RESULT([yes]); HAVE_MM_PREFETCH=1 ],
 [ AC_MSG_RESULT([no]); HAVE_MM_PREFETCH=0 ]
)

AC_MSG_CHECKING([for strong getauxval support in the system headers])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include <sys/auxv.h>
  ]], [[
    getauxval(AT_HWCAP);
  ]])],
 [ AC_MSG_RESULT([yes]); HAVE_STRONG_GETAUXVAL=1; AC_DEFINE([HAVE_STRONG_GETAUXVAL], [1], [Define this symbol to build code that uses getauxval]) ],
 [ AC_MSG_RESULT([no]); HAVE_STRONG_GETAUXVAL=0 ]
)

dnl SUPPRESSED_CPPFLAGS=SUPPRESS_WARNINGS([$SOME_CPPFLAGS])
dnl Replace -I with -isystem in $SOME_CPPFLAGS to suppress warnings from
dnl headers from its include directories and return the result.
dnl See -isystem documentation:
dnl https://gcc.gnu.org/onlinedocs/gcc/Directory-Options.html
dnl https://clang.llvm.org/docs/ClangCommandLineReference.html#cmdoption-clang-isystem-directory
dnl Do not change "-I/usr/include" to "-isystem /usr/include" because that
dnl is not necessary (/usr/include is already a system directory) and because
dnl it would break GCC's #include_next.
AC_DEFUN([SUPPRESS_WARNINGS],
         [[$(echo $1 |${SED} -E -e 's/(^| )-I/\1-isystem /g' -e 's;-isystem /usr/include/*( |$);-I/usr/include\1;g')]])

if test "$enable_wallet" != "no"; then
    dnl Check for libdb_cxx only if wallet enabled
    BITCOIN_FIND_BDB62
    if test "$suppress_external_warnings" != "no" ; then
      BDB_CPPFLAGS=SUPPRESS_WARNINGS($BDB_CPPFLAGS)
    fi
fi

dnl Check to find the libsodium headers/libraries
PKG_CHECK_MODULES([SODIUM], [libsodium],,[AC_MSG_ERROR(libsodium not found.)])

AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
#include <sodium.h>
], [
#if SODIUM_LIBRARY_VERSION_MAJOR < 9 || SODIUM_LIBRARY_VERSION_MAJOR == 9 && SODIUM_LIBRARY_VERSION_MINOR < 1
# error
#endif
])],
[AC_MSG_RESULT([checking for version of libsodium... yes])],
[AC_MSG_ERROR([Wrong libsodium: version >= 1.0.8 required])])

if test "$build_bitcoin_utils$build_bitcoind$use_tests" = "nonono"; then
    use_boost=no
else
    use_boost=yes
fi

if test "$use_boost" = "yes"; then

  dnl Check for Boost headers
  AX_BOOST_BASE([1.73.0],[],[AC_MSG_ERROR([Boost is not available!])])
  AX_BOOST_SYSTEM
  AX_BOOST_FILESYSTEM
  AX_BOOST_PROGRAM_OPTIONS
  AX_BOOST_THREAD
  AX_BOOST_CHRONO

  if test "$want_boost" = "no"; then
    AC_MSG_ERROR([Boost is required])
  fi

  dnl we don't use multi_index serialization
  BOOST_CPPFLAGS="$BOOST_CPPFLAGS -DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION"

  dnl Prevent use of std::unary_function, which was removed in C++17,
  dnl and will generate warnings with newer compilers for Boost
  dnl older than 1.80.
  dnl See: https://github.com/boostorg/config/pull/430.
  AX_CHECK_PREPROC_FLAG([-DBOOST_NO_CXX98_FUNCTION_BASE], [BOOST_CPPFLAGS="$BOOST_CPPFLAGS -DBOOST_NO_CXX98_FUNCTION_BASE"], [], [$CXXFLAG_WERROR],
                        [AC_LANG_PROGRAM([[#include <boost/config.hpp>]])])

  if test "$suppress_external_warnings" != "no"; then
    BOOST_CPPFLAGS=SUPPRESS_WARNINGS($BOOST_CPPFLAGS)
  fi
fi

dnl Check for reduced exports
if test "$use_reduce_exports" = "yes"; then
  AX_CHECK_COMPILE_FLAG([-fvisibility=hidden], [CXXFLAGS="$CXXFLAGS -fvisibility=hidden"],
  [AC_MSG_ERROR([Cannot set hidden symbol visibility. Use --disable-reduce-exports.])], [$CXXFLAG_WERROR])
  AX_CHECK_LINK_FLAG([-Wl,--exclude-libs,ALL], [RELDFLAGS="-Wl,--exclude-libs,ALL"], [], [$LDFLAG_WERROR])
  AX_CHECK_LINK_FLAG([-Wl,-no_exported_symbols], [LIBTOOL_APP_LDFLAGS="$LIBTOOL_APP_LDFLAGS -Wl,-no_exported_symbols"], [], [$LDFLAG_WERROR])
fi

if test "$use_tests" = "yes"; then

  if test "$HEXDUMP" = ""; then
    AC_MSG_ERROR([hexdump is required for tests])
  fi

  if test "$use_boost" = "yes"; then

    AX_BOOST_UNIT_TEST_FRAMEWORK

    dnl Determine if -DBOOST_TEST_DYN_LINK is needed
    AC_MSG_CHECKING([for dynamic linked boost test])
    TEMP_LIBS="$LIBS"
    LIBS="$LIBS $BOOST_LDFLAGS $BOOST_UNIT_TEST_FRAMEWORK_LIB"
    TEMP_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
    AC_LINK_IFELSE([AC_LANG_SOURCE([
         #define BOOST_TEST_DYN_LINK
         #define BOOST_TEST_MAIN
          #include <boost/test/unit_test.hpp>

         ])],
      [AC_MSG_RESULT(yes)]
      [TESTDEFS="$TESTDEFS -DBOOST_TEST_DYN_LINK"],
      [AC_MSG_RESULT(no)])
    LIBS="$TEMP_LIBS"
    CPPFLAGS="$TEMP_CPPFLAGS"
  fi
fi

if test "$use_boost" = "yes"; then
  BOOST_LIBS="$BOOST_LDFLAGS $BOOST_SYSTEM_LIB $BOOST_FILESYSTEM_LIB $BOOST_PROGRAM_OPTIONS_LIB $BOOST_THREAD_LIB $BOOST_CHRONO_LIB"
fi

dnl libevent check

use_libevent=no
if test "$build_bitcoin_utils$build_bitcoind$use_tests$use_bench" != "nononono"; then
  PKG_CHECK_MODULES([EVENT], [libevent >= 2.1.8], [use_libevent=yes], [AC_MSG_ERROR([libevent version 2.1.8 or greater not found.])])
  if test "$TARGET_OS" != "windows"; then
    PKG_CHECK_MODULES([EVENT_PTHREADS], [libevent_pthreads >= 2.1.8], [], [AC_MSG_ERROR([libevent_pthreads version 2.1.8 or greater not found.])])
  fi

  if test "$suppress_external_warnings" != "no"; then
    EVENT_CFLAGS=SUPPRESS_WARNINGS($EVENT_CFLAGS)
  fi
fi

if test x$use_libevent = xyes; then
  TEMP_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS $EVENT_CFLAGS"
  AC_MSG_CHECKING([if evhttp_connection_get_peer expects const char**])
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
      #include <cstdint>
      #include <event2/http.h>
    ]], [[
      evhttp_connection *conn = (evhttp_connection *)1;
      const char *host;
      uint16_t port;

      evhttp_connection_get_peer(conn, &host, &port);
    ]])],
    [ AC_MSG_RESULT([yes]); AC_DEFINE([HAVE_EVHTTP_CONNECTION_GET_PEER_CONST_CHAR], [1], [Define this symbol if evhttp_connection_get_peer expects const char**]) ],
    [ AC_MSG_RESULT([no]) ]
  )
  CXXFLAGS="$TEMP_CXXFLAGS"
fi

dnl ZMQ check

if test "$use_zmq" = "yes"; then
  PKG_CHECK_MODULES([ZMQ], [libzmq >= 4],
    AC_DEFINE([ENABLE_ZMQ], [1], [Define this symbol to enable ZMQ functions]),
    [AC_MSG_WARN([libzmq version 4.x or greater not found, disabling])
    use_zmq=no])
fi

if test "$use_zmq" = "yes"; then
  dnl Assume libzmq was built for static linking
  case $host in
    *mingw*)
      ZMQ_CFLAGS="$ZMQ_CFLAGS -DZMQ_STATIC"
    ;;
  esac
fi

AM_CONDITIONAL([ENABLE_ZMQ], [test "$use_zmq" = "yes"])

AC_SEARCH_LIBS([dlsym],[dl])

RUST_LIBS=""
case $host in
  *mingw*)
    ;;
  *darwin*)
    ;;
  *)
    RUST_LIBS="$RUST_LIBS -ldl"
    ;;
esac

LIBZCASH_LIBS="$BOOST_SYSTEM_LIB -lsodium $RUST_LIBS"

AC_MSG_CHECKING([whether to build bitcoinzd])
AM_CONDITIONAL([BUILD_BITCOIND], [test $build_bitcoind = "yes"])
AC_MSG_RESULT($build_bitcoind)

AC_MSG_CHECKING([whether to build utils (bitcoinz-cli bitcoinz-tx)])
AM_CONDITIONAL([BUILD_BITCOIN_UTILS], [test $build_bitcoin_utils = "yes"])
AC_MSG_RESULT($build_bitcoin_utils)

AC_LANG_POP

if test "$use_ccache" != "no"; then
  AC_MSG_CHECKING([if ccache should be used])
  if test "$CCACHE" = ""; then
    if test "$use_ccache" = "yes"; then
      AC_MSG_ERROR([ccache not found.]);
    else
      use_ccache=no
    fi
  else
    use_ccache=yes
    CC="$ac_cv_path_CCACHE $CC"
    CXX="$ac_cv_path_CCACHE $CXX"
  fi
  AC_MSG_RESULT($use_ccache)
  if test "$use_ccache" = "yes"; then
    AX_CHECK_COMPILE_FLAG([-fdebug-prefix-map=A=B], [DEBUG_CXXFLAGS="$DEBUG_CXXFLAGS -fdebug-prefix-map=\$(abs_top_srcdir)=."], [], [$CXXFLAG_WERROR])
    AX_CHECK_PREPROC_FLAG([-fmacro-prefix-map=A=B], [DEBUG_CPPFLAGS="$DEBUG_CPPFLAGS -fmacro-prefix-map=\$(abs_top_srcdir)=."], [], [$CXXFLAG_WERROR])
  fi
fi

dnl enable wallet
AC_MSG_CHECKING([if wallet should be enabled])
if test "$enable_wallet" != "no"; then
  AC_MSG_RESULT([yes])
  AC_DEFINE_UNQUOTED([ENABLE_WALLET],[1],[Define to 1 to enable wallet functions])
  enable_wallet=yes

else
  AC_MSG_RESULT([no])
fi

dnl enable mining
AC_MSG_CHECKING([if mining should be enabled])
if test "$enable_mining" != "no"; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(ENABLE_MINING, 1, [Define to 1 to enable mining functions])
else
  AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING([whether to build test_bitcoin])
if test "$use_tests" = "yes"; then
  AC_MSG_RESULT([yes])
  BUILD_TEST="yes"
else
  AC_MSG_RESULT([no])
  BUILD_TEST=""
fi

AC_MSG_CHECKING([whether to reduce exports])
if test "$use_reduce_exports" = "yes"; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi

if test "$build_bitcoin_utils$build_bitcoind$use_tests$use_tests" = "nononono"; then
  AC_MSG_ERROR([No targets! Please specify at least one of: --with-utils --with-daemon --enable-bench or --enable-tests])
fi

AM_CONDITIONAL([TARGET_DARWIN], [test "$TARGET_OS" = "darwin"])
AM_CONDITIONAL([BUILD_DARWIN], [test "$BUILD_OS" = "darwin"])
AM_CONDITIONAL([TARGET_LINUX], [test "$TARGET_OS" = "linux"])
AM_CONDITIONAL([TARGET_WINDOWS], [test "$TARGET_OS" = "windows"])
AM_CONDITIONAL([ENABLE_WALLET], [test "$enable_wallet" = "yes"])
AM_CONDITIONAL([ENABLE_MINING],[test "$enable_mining" = "yes"])
AM_CONDITIONAL([ENABLE_TESTS], [test "$BUILD_TEST" = "yes"])
AM_CONDITIONAL([ENABLE_BENCH], [test "$use_bench" = "yes"])
AM_CONDITIONAL([USE_LCOV], [test "$use_lcov" = "yes"])
AM_CONDITIONAL([HARDEN], [test "$use_hardening" = "yes"])
AM_CONDITIONAL([ENABLE_SSE42], [test "$enable_sse42" = "yes"])
AM_CONDITIONAL([ENABLE_ARM_CRC], [test "$enable_arm_crc" = "yes"])
AM_CONDITIONAL([WORDS_BIGENDIAN], [test "$ac_cv_c_bigendian" = "yes"])

AC_DEFINE(CLIENT_VERSION_MAJOR, _CLIENT_VERSION_MAJOR, [Major version])
AC_DEFINE(CLIENT_VERSION_MINOR, _CLIENT_VERSION_MINOR, [Minor version])
AC_DEFINE(CLIENT_VERSION_REVISION, _CLIENT_VERSION_REVISION, [Build revision])
AC_DEFINE(CLIENT_VERSION_BUILD, _CLIENT_VERSION_BUILD, [Version Build])
AC_DEFINE(CLIENT_VERSION_IS_RELEASE, _CLIENT_VERSION_IS_RELEASE, [Version is release])
AC_DEFINE(COPYRIGHT_YEAR, _COPYRIGHT_YEAR, [Version is release])
AC_SUBST(CLIENT_VERSION_MAJOR, _CLIENT_VERSION_MAJOR)
AC_SUBST(CLIENT_VERSION_MINOR, _CLIENT_VERSION_MINOR)
AC_SUBST(CLIENT_VERSION_REVISION, _CLIENT_VERSION_REVISION)
AC_SUBST(CLIENT_VERSION_BUILD, _CLIENT_VERSION_BUILD)
AC_SUBST(CLIENT_VERSION_IS_RELEASE, _CLIENT_VERSION_IS_RELEASE)
AC_SUBST(COPYRIGHT_YEAR, _COPYRIGHT_YEAR)
AC_SUBST(BITCOIN_DAEMON_NAME)
AC_SUBST(BITCOIN_TEST_NAME)
AC_SUBST(BITCOIN_CLI_NAME)
AC_SUBST(BITCOIN_TX_NAME)

AC_SUBST(RELDFLAGS)
AC_SUBST(DEBUG_CPPFLAGS)
AC_SUBST(WARN_CXXFLAGS)
AC_SUBST(NOWARN_CXXFLAGS)
AC_SUBST(DEBUG_CXXFLAGS)
AC_SUBST(ERROR_CXXFLAGS)
AC_SUBST(HARDENED_CXXFLAGS)
AC_SUBST(HARDENED_CPPFLAGS)
AC_SUBST(HARDENED_LDFLAGS)
AC_SUBST(PIC_FLAGS)
AC_SUBST(PIE_FLAGS)
AC_SUBST(SANITIZER_CXXFLAGS)
AC_SUBST(SANITIZER_LDFLAGS)
AC_SUBST(SSE42_CXXFLAGS)
AC_SUBST(ARM_CRC_CXXFLAGS)
AC_SUBST(LIBTOOL_APP_LDFLAGS)
AC_SUBST(BOOST_LIBS)
AC_SUBST(TESTDEFS)
AC_SUBST(LIBZCASH_LIBS)
AC_SUBST(HAVE_FDATASYNC)
AC_SUBST(HAVE_FULLFSYNC)
AC_SUBST(HAVE_O_CLOEXEC)
AC_SUBST(HAVE_BUILTIN_PREFETCH)
AC_SUBST(HAVE_MM_PREFETCH)
AC_SUBST(HAVE_STRONG_GETAUXVAL)
AC_SUBST(CARGO_FLAGS)
AC_SUBST(RUST_TARGET)
AC_SUBST(RUST_VENDORED_SOURCES)
AC_CONFIG_FILES([Makefile src/Makefile doc/man/Makefile share/setup.nsi share/Info.plist src/test/buildenv.py])
AC_CONFIG_FILES([contrib/devtools/split-debug.sh],[chmod +x contrib/devtools/split-debug.sh])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([doc/Doxyfile])])
AC_CONFIG_LINKS([contrib/filter-lcov.py:contrib/filter-lcov.py])
AC_CONFIG_FILES([qa/pull-tester/run-bitcoind-for-test.sh],[chmod +x qa/pull-tester/run-bitcoind-for-test.sh])
AC_CONFIG_FILES([qa/pull-tester/tests-config.sh],[chmod +x qa/pull-tester/tests-config.sh])
AC_CONFIG_LINKS([src/test/Makefile:src/test/Makefile])

dnl boost's m4 checks do something really nasty: they export these vars. As a
dnl result, they leak into secp256k1's configure and crazy things happen.
dnl Until this is fixed upstream and we've synced, we'll just un-export them.
CPPFLAGS_TEMP="$CPPFLAGS"
unset CPPFLAGS
CPPFLAGS="$CPPFLAGS_TEMP"

if test -n "$use_sanitizers"; then
  export SECP_CFLAGS="$SECP_CFLAGS $SANITIZER_CFLAGS"
fi
ac_configure_args="${ac_configure_args} --disable-shared --with-pic --enable-benchmark=no --enable-module-recovery --disable-module-ecdh"
AC_CONFIG_SUBDIRS([src/secp256k1])

AC_OUTPUT

dnl Replace the BUILDDIR path with the correct Windows path if compiling on Native Windows
case ${OS} in
   *Windows*)
     sed  's/BUILDDIR="\/\([[a-z]]\)/BUILDDIR="\1:/'  test/config.ini > test/config-2.ini
     mv test/config-2.ini test/config.ini
   ;;
esac

dnl An old hack similar to a98356fee to remove hard-coded
dnl bind_at_load flag from libtool
case $host in
  *darwin*)
     AC_MSG_RESULT([Removing -Wl,bind_at_load from libtool.])
     sed < libtool > libtool-2 '/bind_at_load/d'
     mv libtool-2 libtool
     chmod 755 libtool
   ;;
esac

echo
echo "Options used to compile and link:"
echo "  with wallet     = $enable_wallet"
echo "  with zmq        = $use_zmq"
echo "  with test       = $use_tests"
echo "  sanitizers      = $use_sanitizers"
echo "  debug enabled   = $enable_debug"
echo "  werror          = $enable_werror"
echo
echo "  target os       = $host_os"
echo "  build os        = $build_os"
echo
echo "  CC              = $CC"
echo "  CFLAGS          = $PTHREAD_CFLAGS $SANITIZER_CFLAGS $CFLAGS"
echo "  CPPFLAGS        = $DEBUG_CPPFLAGS $HARDENED_CPPFLAGS $CPPFLAGS"
echo "  CXX             = $CXX"
echo "  CXXFLAGS        = $DEBUG_CXXFLAGS $HARDENED_CXXFLAGS $WARN_CXXFLAGS $NOWARN_CXXFLAGS $ERROR_CXXFLAGS $SANITIZER_CXXFLAGS $CXXFLAGS"
echo "  LDFLAGS         = $PTHREAD_LIBS $HARDENED_LDFLAGS $SANITIZER_LDFLAGS $LDFLAGS"
echo "  AR              = $AR"
echo "  ARFLAGS         = $ARFLAGS"
echo
echo "  CARGO           = $CARGO"
echo "  CARGO_FLAGS     = $CARGO_FLAGS"
echo
echo "  RUSTC           = $RUSTC"
echo "  RUST_TARGET     = $RUST_TARGET"
echo
