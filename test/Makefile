
qtest1: qtest.c
	cc -o qtest1 -I ../fastcond qtest.c ../fastcond/fastcond.c -lpthread

strongtest_p: strongtest.c
	cc -o strongtest_p -I ../fastcond strongtest.c ../fastcond/fastcond.c -lpthread
strongtest_wc: strongtest.c
	cc -o strongtest_wc -DFASTCOND_PATCH_WCOND -I ../fastcond strongtest.c ../fastcond/fastcond.c -lpthread
strongtest_c: strongtest.c
	cc -o strongtest_c -DFASTCOND_PATCH_COND -I ../fastcond strongtest.c ../fastcond/fastcond.c -lpthread


all: qtest1 strongtest_p strongtest_wc strongtest_c
