CC ?= gcc

all: prctl_disable_dumpability prctl_disable_dumpability.so

prctl_disable_dumpability.so: prctl_disable_dumpability.c
	$(CC) -Os -shared -Wall -o prctl_disable_dumpability.so prctl_disable_dumpability.c

.PHONY: clean
clean:
	rm -f *.so