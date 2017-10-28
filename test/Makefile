IDIR=../fastcond
LDLIBS=-lpthread -lm

INCLUDES=-I../fastcond
CFLAGS=-O3


_DEPS = fastcond.h fastcond_patch.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

PATCH=COND
DOPATCH=-DFASTCOND_PATCH_$(PATCH)

fastcond.o: ../fastcond/fastcond.c
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $^ 

qtest_pt: qtest.c fastcond.o
	$(CC) $(INCLUDES) $(CFLAGS) -o $@ $^ $(LDLIBS)

qtest_fc: qtest.c fastcond.o
	$(CC) $(INCLUDES) $(CFLAGS) $(DOPATCH) -o $@ $^ $(LDLIBS)

strongtest_pt: strongtest.c fastcond.o 
	$(CC) $(INCLUDES) $(CFLAGS) -o $@ $^ $(LDLIBS)
strongtest_fc: strongtest.c fastcond.o
	$(CC) $(INCLUDES) $(CFLAGS) $(DOPATCH) -o $@ $^ $(LDLIBS)


ALL=qtest_pt qtest_fc strongtest_pt strongtest_fc

.PHONY: all
all: $(ALL)

.PHONY: clean
clean:
	rm -f *.o $(ALL)