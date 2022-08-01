#include <sys/prctl.h>

int __attribute__((constructor)) disable_dumpability() {
    return prctl(PR_SET_DUMPABLE, 0); // 0=SUID_DUMP_DISABLE
}

