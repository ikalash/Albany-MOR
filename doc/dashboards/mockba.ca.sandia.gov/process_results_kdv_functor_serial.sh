
cd /home/ikalash/Trilinos_Albany/nightlyCDash
grep "Test   #" nightly_log_kdv_functor_serial.txt >& results0
grep "Test  #" nightly_log_kdv_functor_serial.txt >& results1 
cat results0 results1 >& results11
grep "Test #" nightly_log_kdv_functor_serial.txt >& results0
cat results11 results0 >& results1
grep " tests failed" nightly_log_kdv_functor_serial.txt >& results2 
cat results1 results2 >& results3
grep "Total Test" nightly_log_kdv_functor_serial.txt >& results4
cat results3 results4 >& results5
grep "(Failed)" nightly_log_kdv_functor_serial.txt >& results6 
cat results5 results6 >& results_kdv_functor_serial
rm results0 results1 results11 results2 results3 results4 results5 results6
