import subprocess
import random
import sys

def main():
    if len(sys.argv) < 3:
        print("Missing required arguments: path to binary, number of test")
        exit(1)
    binary = sys.argv[1]
    one_test_count = int(sys.argv[2])
    if one_test_count < 1 or one_test_count > 50:
        print("Number of test can be between 1 and 50")
        exit(1)
    args = " -p "
    for i in range(1, 10):
        for c in range(one_test_count):
            current_args = args + str(i) + " "
            sum = 0
            for j in range(1, i + 1):
                r = random.randint(1,99)
                current_args += str(r)
                sum += r
                if j != i:
            	    current_args += " "
            print("TEST " + str(i) + "." + str(c + 1))
            print("-------------------------------------")
            print(binary + current_args)
            print("Sum: " + str(sum))
            with subprocess.Popen(binary + current_args, stdout=subprocess.PIPE, shell=True) as proc:
                output = proc.stdout.read()
                res = output.splitlines()[-2]
                int_res = list(map(int, list(filter(lambda w: w.isdigit(), res.split()))))
                result = all(elem == sum for elem in int_res)
                if result:
                    print("PASSED")
                else:
                    print("NOT PASSED")
                print()

if __name__ == "__main__":
    main()
