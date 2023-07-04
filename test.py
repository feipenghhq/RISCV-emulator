#!/usr/bin/python3
# Test for the emulator using riscv-tests
# https://github.com/riscv-software-src/riscv-tests

import os
import subprocess

RV64UI_P_TEST = [
    "lui", "auipc",
    "jal", "jalr", "beq", "bne", "blt", "bge", "bltu", "bgeu",
    "lb", "lh", "lw", "lbu", "lhu", "sb", "sh", "sw",
    "addi", "slti", "sltiu", "xori", "ori", "andi", "slli", "srli", "srai",
    "add", "sub", "sll", "slt", "sltu", "xor", "srl", "sra", "or", "and",
    "fence_i", "ecall", "ebreak",
    "lwu", "ld", "sd", "addiw", "slliw", "srliw", "sraiw", "addw", "subw", "sllw", "srlw", "sraw",
]

RV64UM_P_TEST = [
    "mul", "mulh", "mulhsu", "mulhu", "div", "divu", "rem", "remu", "mulw", "divw", "divuw", "remw", "remuw",
]

RV64MI_P_TEST = [
    "mcsr", "csr",
]


class Tester:

    def __init__(self):
        self.path = os.getcwd()
        self.isa_test_dir = "test/riscv-tests/target/share/riscv-tests/isa"
        self.report_pass=False

    def run_test(self, prefix, name):
        self.test_path = self.isa_test_dir + "/" + prefix + name
        cmd = "./rvemu " + self.test_path
        proc = subprocess.Popen(cmd,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                            shell=True,
                            universal_newlines=True)
        std_out, std_err = proc.communicate()
        return proc.returncode

    def report_result(self, test_suite, name):
        print("Test Result for test suite: " + name)
        passed = 0
        failed = 0
        for i in range(len(test_suite)):
            if self.test_result[i]:
                if self.report_pass:
                    print(f"{test_suite[i]}: PASS")
                passed += 1
            else:
                print(f"{test_suite[i]}: FAIL")
                failed += 1
        print("Total Passed: " + str(passed))
        print("Total Failed: " + str(failed))

    def run_test_suite(self, prefix, suite, suite_name):
        self.test_result = []
        for test in suite:
            result = self.run_test(prefix, test)
            if result == 0:
                self.test_result.append(True)
            else:
                self.test_result.append(False)
        self.report_result(suite, suite_name)

    def run_rv64ui_p_test(self):
        self.run_test_suite('rv64ui-p-', RV64UI_P_TEST, "RV64UI_P_TEST")

    def run_rv64um_p_test(self):
        self.run_test_suite('rv64um-p-', RV64UM_P_TEST, "RV64UM_P_TEST")

    def run_rv64mi_p_test(self):
        self.run_test_suite('rv64mi-p-', RV64MI_P_TEST, "RV64MI_P_TEST")


if __name__ == '__main__':
    tester = Tester()
    tester.run_rv64ui_p_test()
    tester.run_rv64um_p_test()
    tester.run_rv64mi_p_test()