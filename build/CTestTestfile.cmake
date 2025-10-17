# CMake generated Testfile for 
# Source directory: /home/kristjan/git/fastcond
# Build directory: /home/kristjan/git/fastcond/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(qtest_pthread_smoke "/home/kristjan/git/fastcond/build/qtest_pt" "100" "2" "5")
set_tests_properties(qtest_pthread_smoke PROPERTIES  PASS_REGULAR_EXPRESSION "sender.*sent|receiver.*got" _BACKTRACE_TRIPLES "/home/kristjan/git/fastcond/CMakeLists.txt;77;add_test;/home/kristjan/git/fastcond/CMakeLists.txt;0;")
add_test(qtest_fastcond_smoke "/home/kristjan/git/fastcond/build/qtest_fc" "100" "2" "5")
set_tests_properties(qtest_fastcond_smoke PROPERTIES  PASS_REGULAR_EXPRESSION "sender.*sent|receiver.*got" _BACKTRACE_TRIPLES "/home/kristjan/git/fastcond/CMakeLists.txt;79;add_test;/home/kristjan/git/fastcond/CMakeLists.txt;0;")
add_test(qtest_weak_smoke "/home/kristjan/git/fastcond/build/qtest_wcond" "100" "2" "5")
set_tests_properties(qtest_weak_smoke PROPERTIES  _BACKTRACE_TRIPLES "/home/kristjan/git/fastcond/CMakeLists.txt;81;add_test;/home/kristjan/git/fastcond/CMakeLists.txt;0;")
add_test(strongtest_pthread_smoke "/home/kristjan/git/fastcond/build/strongtest_pt" "100" "5")
set_tests_properties(strongtest_pthread_smoke PROPERTIES  PASS_REGULAR_EXPRESSION "sender.*sent|receiver.*got" _BACKTRACE_TRIPLES "/home/kristjan/git/fastcond/CMakeLists.txt;84;add_test;/home/kristjan/git/fastcond/CMakeLists.txt;0;")
add_test(strongtest_fastcond_smoke "/home/kristjan/git/fastcond/build/strongtest_fc" "100" "5")
set_tests_properties(strongtest_fastcond_smoke PROPERTIES  PASS_REGULAR_EXPRESSION "sender.*sent|receiver.*got" _BACKTRACE_TRIPLES "/home/kristjan/git/fastcond/CMakeLists.txt;86;add_test;/home/kristjan/git/fastcond/CMakeLists.txt;0;")
add_test(strongtest_weak_smoke "/home/kristjan/git/fastcond/build/strongtest_wcond" "100" "5")
set_tests_properties(strongtest_weak_smoke PROPERTIES  _BACKTRACE_TRIPLES "/home/kristjan/git/fastcond/CMakeLists.txt;88;add_test;/home/kristjan/git/fastcond/CMakeLists.txt;0;")
