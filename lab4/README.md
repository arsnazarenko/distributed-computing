# Lab 4

1. Build app
```shell
$ make
```

2. Set the environment variable
```shell
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/dir/with/library
```
Path to shared library: `./lib64/libruntime.so`

3. Run app
Help:
```shell
$ ./pa4 --help
Usage: ./pa4 -p [N] [OPTIONS]

Options:
-h, --help                              Display this help text and exit
-p [N], --process-number[=N]            Required option. Define number of child processes created in the program. Value can be between 1 and 9 inclusive.
-m, --mutexl                            Using the Lamport mutual exclusion algorithm

```
Execute:
```shell
$ ./pa4 -p [N] [OPTIONS]
```
