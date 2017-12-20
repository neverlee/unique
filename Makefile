all: libutil unique

clean:
	$(MAKE) -C util clean
	$(MAKE) -C src clean

.PHONY: unique
unique: libutil
	$(MAKE) -C src

.PHONY: libutil
libutil:
	$(MAKE) -C util
