# blade-project

This is a project build from blade. To use:

* init: `git submodule init; git submodule update`
* build: `./blade build ...`
* test: `./blade test`

# Benchmark

```
-------------------------------------------------------------------------------
threadpool - benchmark
-------------------------------------------------------------------------------
benchmark/threadpool.cc:30
...............................................................................

benchmark name                       samples       iterations    estimated
                                     mean          low mean      high mean
                                     std dev       low std dev   high std dev
-------------------------------------------------------------------------------
b_queue.async                                  100             1     12.5247 s
                                        124.142 ms    123.488 ms    125.001 ms
                                        3.78767 ms    3.04356 ms    5.83182 ms

ts_queue.async                                 100             1     12.3435 s
                                        122.545 ms    121.769 ms    123.718 ms
                                        4.79998 ms     3.5956 ms    7.88938 ms

std::async                                     100             1     1.53998 m
                                         1.13085 s     1.10487 s     1.16167 s
                                        143.835 ms    123.363 ms     189.34 ms


===============================================================================
```
