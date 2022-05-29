import os
import timeit
import filecmp

client_cmd = lambda trans, file_in, file_out: f"bin/sdstore proc-file {file_in} {file_out} {trans}"
expected_cmd = lambda trans, file_in, file_out: f"bin/sdstore-transformations/{trans} < {file_in} > {file_out}"

transfs = {
    "bcompress": ("samples/alan.txt", ("tmp/alan.txt.bz2", "tmp/alan_ex.txt.bz2")),
    "bdecompress": ("tmp/alan_ex.txt.bz2", ("tmp/alan.txt", "tmp/alan_ex.txt")),
    "gcompress": ("samples/alan.txt", ("tmp/gcomp.txt", "tmp/gcomp_ex.txt")),
    "gdecompress": ("tmp/gcomp_ex.txt", ("tmp/gdcomp.txt", "tmp/gdcomp_ex.txt")),
    "encrypt": ("samples/alan.txt", ("tmp/encr.txt", "tmp/encr_ex.txt")),
    "decrypt": ("tmp/encr_ex.txt", ("tmp/decr.txt", "tmp/decr_ex.txt")),
    "nop": ("samples/alan.txt", ("tmp/nop.txt", "tmp/nop_ex.txt")),
}

start_server = "bin/sdstored etc/sdstored.conf bin/sdstore-transformations"

total_tests = len(transfs)
passed_tests = 0

print("---- TESTING ----")
#os.system(start_server)

for i, (transf, (inp, (out, expc))) in enumerate(transfs.items(), 1):
    print(f"Testing {transf} - [{i}/{total_tests}]")
    execution_time = timeit.timeit(lambda : os.system(client_cmd(transf, inp, out)), number=1)
    print(f"Execution time: {execution_time}")
    os.system(expected_cmd(transf, inp, expc))

    if (filecmp.cmp(out, expc, shallow=True)):
        passed_tests += 1
        print("Test passed")
    else:
        print("Test failed")

print(f"Tests passed [{passed_tests}/{total_tests}]")