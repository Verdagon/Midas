import unittest
import subprocess
import re
import json
import os.path
import os
import sys

from typing import Dict, Any, List, Callable


def procrun(args: List[str], **kwargs) -> subprocess.CompletedProcess:
    return subprocess.run(args, capture_output=True, text=True, **kwargs)


class ValeTest(unittest.TestCase):
    GENPATH: str = os.environ.get('GENPATH', "cmake-build-debug")

    def valestrom(self, input_file: str, output_file: str) -> str:
        driver = "Driver20200627.jar"
        driver_class = "net.verdagon.vale.driver.Driver"
        return procrun(
            [
                "java",
                "-cp",
                driver,
                driver_class,
                "build",
                "-o",
                output_file,
                input_file
            ],
            check=True
        )

    def valec(self, args: List[str]) -> subprocess.CompletedProcess:
        assert self.GENPATH
        return procrun([f"../{self.GENPATH}/valec"] + args)

    def clang(self, args: List[str]) -> subprocess.CompletedProcess:
        return procrun(["clang"] + args)

    def aout(self) -> subprocess.CompletedProcess:
        return procrun(["./a.out"])

    def setUp(self) -> None:
        self.GENPATH: str = type(self).GENPATH
        print(
            f"Using valec from {self.GENPATH}. " +
            "Set GENPATH env var if this is incorrect",
            file=sys.stderr
        )

    def test_valec_blackbox(self) -> None:


        vale_file = "addret.vale"
        vir_file = f"{os.path.splitext(vale_file)[0]}.vir"

        proc = self.valestrom(vale_file, vir_file)
        self.assertEqual(proc.returncode, 0, "valestrom couldn't compile addret.vale!")

        proc = self.valec(['addret.vir'])
        self.assertEqual(proc.returncode, 0, "valec couldn't compile addret.vir!")
        proc = self.clang(["addret.o"])
        self.assertEqual(proc.returncode, 0, "clang couldn't compile addret.o")
        proc = self.aout()
        self.assertEqual(proc.returncode, 7)


if __name__ == '__main__':
    unittest.main()
