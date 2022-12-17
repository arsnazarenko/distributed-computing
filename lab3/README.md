# Lab 3

1. Build app
```shell
$ make
```

2. Set the environment variable
```shell
$ export LD_LIBRARY_PATH=/path/to/dir/with/library
```
3. Run app
```shell
$ .pa3 -p [N] $1 $2 ... $N
```
```shell
$ ./pa3 --help

Usage: ./pa3 -p [N] [START_BALANCE]...

Options:
-h, --help                              Display this help text and exit
-p [N], --process-number[=N]            Required option. Define number of child processes created in the program. Value can be between 1 and 9 inclusive.

```

# Tests
Run
```shell
python3 test.py build/pa3
```
