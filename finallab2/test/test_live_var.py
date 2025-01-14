#!/usr/bin/env python3
from typing import List

from collections import defaultdict
import dataclasses
import subprocess
import os
import argparse

ExeGen_ptn = '"clang" "-O0" "-o" "{}" "{}" "../lib/lib.c"'
Exe_ptn = '"{}"'

@dataclasses.dataclass
class TestCase:
    base_path: str
    name: str
    need_input: bool
    
    @property
    def sy(self):
        return os.path.join(self.base_path, self.name + ".sy")
    
    @property
    def in_file_path(self):
        return os.path.join(self.base_path, self.name + ".in")
    
    @property
    def out_file_path(self):
        return os.path.join(self.base_path, self.name + ".out")

    @property
    def path(self):
        return os.path.join(self.base_path, self.name)
    
    @property
    def live_var_out(self):
        return os.path.join(self.base_path, self.name + ".live_var.out")
    
    
    

def eval(EXE_PATH: str, TEST_BASE_PATH: str, opts: str, testcases: List[TestCase]):
    print("===========TEST START===========")
    print("now in {}".format(TEST_BASE_PATH))
    dir_succ = True
    max_name_len = max(len(case.name) for case in testcases)
    for testcase in testcases:
        print(f"Case \x1b[1m{testcase.name:<{max_name_len}}\x1b[0m", end="")

        cmd = f"{EXE_PATH} -emit-ir {opts} -o test.ll {testcase.sy}"
        IRBuild_result = subprocess.run(
            cmd,
            # IRBuild_ptn.format(EXE_PATH, opts, testcase.ll, testcase.sy),
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        if IRBuild_result.returncode == 0:
            with open(testcase.live_var_out, "r") as f:
                ans = f.read()
            with open("live_var.out", "r") as f:
                live_var_out = f.read()
                if live_var_out.strip() == ans.strip():
                    print("\t\x1b[1;32mPass\x1b[0m\n")
                else:
                    print("\t\x1b[1;31mWrong Answer\x1b[0m\n")
        else:
            dir_succ = False
            print("\t\033[31mIRBuild Fail\033[0m")
            subprocess.call(["rm", "-rf", testcase.path, testcase.path + ".ll"])
    if dir_succ:
        print("\t\033[32mSuccess\033[0m in dir {}".format(TEST_BASE_PATH))
    else:
        print("\t\033[31mFail\033[0m in dir {}".format(TEST_BASE_PATH))

    print("============TEST END============")


if __name__ == "__main__":
    # you can only modify this to add your testcase
    TEST_DIRS = [
        "./Test/Easy",
        "./Test/Medium",
        "./Test/Hard",
    ]
    # you can only modify this to add your testcase
    parser = argparse.ArgumentParser(description="Test LiveVar Script")
    opts: list[str] = ["-lv"]

    for TEST_BASE_PATH in TEST_DIRS:
        testcases = defaultdict(bool)
        EXE_PATH = os.path.abspath("../build/compiler")
        name_list = list(set(map(lambda x: x.split(".")[0], os.listdir(TEST_BASE_PATH)))) # extract name from name.sy/name.in/name.out
        testcases = [TestCase(TEST_BASE_PATH, name, os.path.exists(os.path.join(TEST_BASE_PATH, name+".in"))) for name in name_list]
        eval(EXE_PATH, TEST_BASE_PATH, " ".join(opts), testcases)
