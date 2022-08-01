CC ?= gcc

all: prctl_disable_dumpability.so

prctl_disable_dumpability.so: prctl_disable_dumpability.c
	$(CC) -Os -fPIC -shared -Wall -o prctl_disable_dumpability.so prctl_disable_dumpability.c -ldl

.PHONY: clean
clean:
	rm -f *.so