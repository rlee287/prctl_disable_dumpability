# prctl_disable_dumpability

This is a `LD_PRELOAD` shared object for use with dynamic binaries that disables process dumpability using `PR_SET_DUMPABLE` using a `prctl` call. This disables the production of core dumps when they would normally be triggered by a signal, in addition to setting the ownership of many files under `/proc/$pid` to root (e.g. preventing the observation of environment variables via `/proc/$pid/environ`). For more details, see `prctl(2)` and `proc(5)`.

We do not provide a wrapper execuable because the dumpability status is reset on `execve` calls (as detailed in `execve(2)`). However, we shim `execve(3)` and related functions to preserve `LD_PRELOAD` across calls to `execve(2)` to make the `prctl` call again on init, again with the caveat that `LD_PRELOAD` only works with dynamic executables.

Usage: `LD_PRELOAD=/path/to/prctl_disable_dumpability.so dynamic_binary`