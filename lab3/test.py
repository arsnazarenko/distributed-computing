import subprocess
import sys
def main():
    if len(sys.argv) < 2:
        print("Messing required argument: path to binary")
        exit(1)
    binary = sys.argv[1]
    args = " -p "
    for i in range(1, 10):
        current_args = args + str(i) + " "
        sum = 0
        for j in range(10, 10 * (i + 1), 10):
            current_args += str(j)
            sum += j
            if j != (i * 10):
            	current_args += " "
        print("Test " + str(i))
        print("-------------------------------------")
        print(binary + current_args)
        print("Sum: " + str(sum))
        with subprocess.Popen(binary + current_args, stdout=subprocess.PIPE, shell=True) as proc:
            output = proc.stdout.read()
            res = output.splitlines()[-2]
            int_res = list(map(int, list(filter(lambda w: w.isdigit(), res.split()))))
            result = all(elem == sum for elem in int_res)
            if result:
                print("PASS")
            else:
                print("ERROR")

if __name__ == "__main__":
    main()
