SOURCES="input.c output.c"
MODULES="file_input file_output"

posix_memalign_test_c()
{
    cat <<EOF
#include <stdio.h>
#include <stdlib.h>
int main(void) { void *p = NULL; return posix_memalign(&p, 32, 128); }
EOF
}

check_posix_memalign()
{
    posix_memalign_test_c | $APP_C -Werror $CFLAGS $APP_CFLAGS -o /dev/null -x c - >/dev/null 2>&1
}

if check_posix_memalign ; then
    CFLAGS="-DHAVE_POSIX_MEMALIGN=1"
fi

libaio_test_c()
{
    cat <<EOF
#include <libaio.h>
int main(void) { return 0; }
EOF
}

check_libaio()
{
    libaio_test_c | $APP_C -Werror $CFLAGS $APP_CFLAGS -o /dev/null -x c - >/dev/null 2>&1
}

aio_test_c()
{
    cat <<EOF
#ifdef _WIN32
#   error Win32
#endif
#include <aio.h>
int main(void) { return 0; }
EOF
}

check_aio()
{
    aio_test_c | $APP_C -Werror $CFLAGS $APP_CFLAGS -o /dev/null -x c - >/dev/null 2>&1
}

if check_aio ; then
    CFLAGS="$CFLAGS -DHAVE_AIO=1"
    if [ "$OS" = "linux" ] ; then
        LDFLAGS="-lrt"
        if check_libaio ; then
            CFLAGS="$CFLAGS -DHAVE_LIBAIO=1"
            LDFLAGS="$LDFLAGS -laio"
        fi
    fi
fi
