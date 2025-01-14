#!/usr/bin/env python3
import subprocess
import os
import sys
import argparse

def eprint(*objects: object, sep: str = " ", end: str = "\n", flush: bool = False):
    print(*objects, sep=sep, end=end, file=sys.stderr, flush=flush)


def run_command(cmd: str, input=None):
    return subprocess.run(
        cmd, shell=True, capture_output=True, input=input, text=True
    )


def task_eval(
    exe_path: str,
    test_base_path: str,
    testcases: dict[str, bool],
    opts: str,
    timeout: int,
):
    eprint("===========TEST START===========")
    eprint(f"now in {test_base_path}")
    dir_succ = True
    succ_count = 0
    max_len = max(len(case) for case in testcases)
    for case, need_input in testcases.items():
        eprint(f"Case \x1b[1m{case:<{max_len}}\x1b[0m", end="")
        test_path = os.path.join(test_base_path, case)
        sy_path = test_path + ".sy"
        ll_path = test_path + ".ll"
        input_path = test_path + ".in"
        output_path = test_path + ".out"

        result = run_command(f"{exe_path} {opts} -emit-ir -o {ll_path} {sy_path}")
        if result.returncode == 0:
            input_option = None
            if need_input:
                with open(input_path) as fin:
                    input_option = fin.read()
            try:
                result = run_command(
                    f"clang -O0 -o {test_path} {ll_path} ../lib/lib.c"
                )
                if result.returncode != 0:
                    dir_succ = False
                    eprint("\t\x1b[1;31mCompile Error\x1b[0m")
                    continue
                # Use the timeout command instead of subprocess's timeout option,
                # as the latter has too many bugs in its implementation.
                result = run_command(f"timeout {timeout} {test_path}", input=input_option)
                if result.returncode == 124: # timeout
                    dir_succ = False
                    eprint("\t\x1b[1;31mTime Out\x1b[0m")
                    continue
                outs = result.stdout.splitlines()
                outs.append(str(result.returncode))
                outs = [line for line in outs if line]  # Remove empty line
                answer = [
                    line for line in open(output_path).read().splitlines() if line
                ]
                case_succ = outs == answer
                dir_succ = dir_succ and case_succ
                if case_succ:
                    eprint("\t\x1b[1;32mPass\x1b[0m")
                    succ_count += 1
                else:
                    eprint("\t\x1b[1;31mWrong Answer\x1b[0m")
            except Exception as e:
                dir_succ = False
                eprint(e, end="")
                eprint("\t\x1b[31mCodeGen or CodeExecute Fail\x1b[0m")
            finally:
                _ = run_command(f"rm -rf {test_path} {test_path}.o {test_path}.ll")
        else:
            dir_succ = False
            print(result)
            eprint("\t\x1b[1;31mIRBuild Fail\x1b[0m")
    eprint(
        f"\t\x1b[1m{succ_count} Passed / {len(testcases) - succ_count} Failed\x1b[0m"
    )
    if dir_succ:
        eprint(f"\t\x1b[1;32mSuccess\x1b[0m in dir {test_base_path}")
    else:
        eprint(f"\t\x1b[1;31mFail\x1b[0m in dir {test_base_path}")

    eprint("============TEST END============")

if __name__ == "__main__":
    # you can only modify this to add your testcase
    TEST_DIRS = [
        "./Test/Easy",
        "./Test/Medium",
        "./Test/Hard",
        "./Opt/CSE"
    ]
    # you can only modify this to add your testcase
    parser = argparse.ArgumentParser(description="Test script")
    parser.add_argument("-O2", action="store_true", help="Enable all optimizations")
    parser.add_argument(
        "-O", action="store_true", help="Enable Mem2Reg and DominateTree"
    )
    parser.add_argument("-check", action="store_true", help="Enable checker")
    parser.add_argument(
        "-lv", action="store_true", help="Enable live variable analysis"
    )
    parser.add_argument(
        "-cse", action="store_true", help="Enable common subexpression elimination"
    )
    args = parser.parse_args()
    opts: list[str] = []

    if args.O2:
        opts.append("-O2")
    if args.O:
        opts.append("-O")
    if args.check:
        opts.append("-check")
    if args.lv:
        opts.append("-lv")
    if args.cse:
        opts.append("-cse")
    for TEST_BASE_PATH in TEST_DIRS:
        testcases: dict[str, bool] = {}  # { name: need_input }
        EXE_PATH = os.path.abspath("../build/compiler")
        testcase_list = list(map(lambda x: x.split("."), os.listdir(TEST_BASE_PATH)))
        testcase_list.sort()
        for i in range(len(testcase_list)):
            testcases[testcase_list[i][0]] = False
        for i in range(len(testcase_list)):
            testcases[testcase_list[i][0]] = testcases[testcase_list[i][0]] | (
                testcase_list[i][1] == "in"
            )
        task_eval(EXE_PATH, TEST_BASE_PATH, testcases, " ".join(opts), 30)
