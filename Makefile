all: libutil unique

clean:
	$(MAKE) -C util clean
	$(MAKE) -C src clean

.PHONY: unique
redex:
	$(MAKE) -C src

.PHONY: libutil
libutil:
	$(MAKE) -C util
