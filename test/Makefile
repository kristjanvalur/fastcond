IDIR=../fastcond
LDLIBS=-lpthread -lm

INCLUDES=-I../fastcond
CFLAGS=-O3


_DEPS = fastcond.h fastcond_patch.h gil.h native_primitives.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

PATCH=COND
DOPATCH=-DFASTCOND_PATCH_$(PATCH)

fastcond.o: ../fastcond/fastcond.c
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $^ 

gil.o: ../fastcond/gil.c
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $^

gil_native.o: ../fastcond/gil.c
	$(CC) $(INCLUDES) $(CFLAGS) -DFASTCOND_GIL_USE_NATIVE_COND=1 -c -o $@ $^

# Unfair variants (fairness disabled)
gil_unfair.o: ../fastcond/gil.c
	$(CC) $(INCLUDES) $(CFLAGS) -DFASTCOND_GIL_DISABLE_FAIRNESS=1 -c -o $@ $^

# Naive variant (mutex-only, no condition variables)
gil_naive.o: ../fastcond/gil.c
	$(CC) $(INCLUDES) $(CFLAGS) -DFASTCOND_GIL_MODE_NAIVE=1 -DFASTCOND_GIL_DISABLE_FAIRNESS=1 -c -o $@ $^

gil_native_unfair.o: ../fastcond/gil.c
	$(CC) $(INCLUDES) $(CFLAGS) -DFASTCOND_GIL_USE_NATIVE_COND=1 -DFASTCOND_GIL_DISABLE_FAIRNESS=1 -c -o $@ $^

qtest_pt: qtest.c fastcond.o
	$(CC) $(INCLUDES) $(CFLAGS) -o $@ $^ $(LDLIBS)

qtest_fc: qtest.c fastcond.o
	$(CC) $(INCLUDES) $(CFLAGS) $(DOPATCH) -o $@ $^ $(LDLIBS)

qtest_wcond: qtest.c fastcond.o
	$(CC) $(INCLUDES) $(CFLAGS) -DFASTCOND_PATCH_WCOND -o $@ $^ $(LDLIBS)

strongtest_pt: strongtest.c fastcond.o 
	$(CC) $(INCLUDES) $(CFLAGS) -o $@ $^ $(LDLIBS)
strongtest_fc: strongtest.c fastcond.o
	$(CC) $(INCLUDES) $(CFLAGS) $(DOPATCH) -o $@ $^ $(LDLIBS)

# GIL tests with fastcond backend
gil_test_fc: gil_test.c gil.o fastcond.o
	$(CC) $(INCLUDES) $(CFLAGS) -o $@ $^ $(LDLIBS)

# GIL tests with native pthread backend  
gil_test_native: gil_test.c gil_native.o
	$(CC) $(INCLUDES) $(CFLAGS) -DFASTCOND_GIL_USE_NATIVE_COND=1 -o $@ $^ $(LDLIBS)

# GIL tests with fastcond backend (unfair)
gil_test_fc_unfair: gil_test.c gil_unfair.o fastcond.o
	$(CC) $(INCLUDES) $(CFLAGS) -DFASTCOND_GIL_DISABLE_FAIRNESS=1 -o $@ $^ $(LDLIBS)

# GIL tests with naive mutex-only implementation
gil_test_fc_naive: gil_test.c gil_naive.o fastcond.o
	$(CC) $(INCLUDES) $(CFLAGS) -DFASTCOND_GIL_MODE_NAIVE=1 -DFASTCOND_GIL_DISABLE_FAIRNESS=1 -o $@ $^ $(LDLIBS)

# GIL tests with native pthread backend (unfair)
gil_test_native_unfair: gil_test.c gil_native_unfair.o
	$(CC) $(INCLUDES) $(CFLAGS) -DFASTCOND_GIL_USE_NATIVE_COND=1 -DFASTCOND_GIL_DISABLE_FAIRNESS=1 -o $@ $^ $(LDLIBS)

# GIL benchmarks with fastcond backend
gil_benchmark_fc: gil_benchmark.c gil.o fastcond.o
	$(CC) $(INCLUDES) $(CFLAGS) -o $@ $^ $(LDLIBS)

# GIL benchmarks with native pthread backend
gil_benchmark_native: gil_benchmark.c gil_native.o
	$(CC) $(INCLUDES) $(CFLAGS) -DFASTCOND_GIL_USE_NATIVE_COND=1 -o $@ $^ $(LDLIBS)

# GIL benchmarks with fastcond backend (unfair)
gil_benchmark_fc_unfair: gil_benchmark.c gil_unfair.o fastcond.o
	$(CC) $(INCLUDES) $(CFLAGS) -DFASTCOND_GIL_DISABLE_FAIRNESS=1 -o $@ $^ $(LDLIBS)

# GIL benchmarks with native pthread backend (unfair)
gil_benchmark_native_unfair: gil_benchmark.c gil_native_unfair.o
	$(CC) $(INCLUDES) $(CFLAGS) -DFASTCOND_GIL_USE_NATIVE_COND=1 -DFASTCOND_GIL_DISABLE_FAIRNESS=1 -o $@ $^ $(LDLIBS)


ALL=qtest_pt qtest_fc qtest_wcond strongtest_pt strongtest_fc gil_test_fc gil_test_native gil_test_fc_unfair gil_test_fc_naive gil_test_native_unfair gil_benchmark_fc gil_benchmark_native gil_benchmark_fc_unfair gil_benchmark_native_unfair

.PHONY: all
all: $(ALL)

.PHONY: clean
clean:
	rm -f *.o $(ALL)