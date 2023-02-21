import os
import subprocess
import time
from pathlib import *

my_timeout = int(15)


""" to run it in windows: wsl python3 test.py"""
if __name__ == "__main__":
    os.system("mv app.cpp app.cpp.cache")
    if os.path.exists("test_output"):
        os.system("rm -r test_output")
    if os.path.exists("cmp_output"):
        os.system("rm -r cmp_output")
    os.system("mkdir test_output")
    os.system("mkdir cmp_output")
    os.system("make clean >> make_log.txt")
    failed_tests = []
    for filename in os.listdir("testcase"):
        print("---------- testing: %s ----------" % filename)
        page_size = filename.split('.')[-2]
        os.system("cp testcase/%s app.cpp" % filename)
        os.system("make all>> make_log.txt")
        # & means it is a background task
        pager = "./pager -m {} > test_output/{}.txt &".format(page_size, filename)
        app = "./app > test_output/{}.txt &".format(filename + ".app")
        try:
            os.system(pager)
            time.sleep(1)
            os.system(app)

            if "Random" in filename:
                time.sleep(20)
            time.sleep(10)

            os.system("ps -C pager -o pid=|xargs kill -9")
            os.system("ps -C app -o pid=|xargs kill -9")

            cmp = "diff --strip-trailing-cr <(sed \'s/([0-9|, ]*)//g\' test_output/%s.txt) " % filename + \
                  " <(sed \'s/([0-9|, ]*)//g\' correct_output/%s.txt) " % filename + \
                  " | tee cmp_output/%s.txt " % filename
            # "sed -i 's/([0-9|, ]*)//g' test_output/testCopyOnWrite.4.cpp.txt"
            print(cmp)
            subprocess.call(['bash', '-c', cmp])

            cmp_file_path = "cmp_output/%s.txt" % filename
            # print("cmp file size: %s", Path(cmp_file_path).stat())
            if Path(cmp_file_path).exists() and Path(cmp_file_path).stat().st_size != 0:
                failed_tests.append(filename)

        except Exception as e:
            print(e)

    os.system("mv app.cpp.cache app.cpp")
    print("---------- test finished ----------")

    print("---------- test summary ----------")
    for test_name in failed_tests:
        print(test_name + " failed!")
