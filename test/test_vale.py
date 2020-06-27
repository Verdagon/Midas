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

    @staticmethod
    def extract_json_str(driver_output: str) -> str:
        capture = False
        json_lines = []
        for line in driver_output.splitlines():
            if not capture and \
                    re.match(
                        pattern="^\s*\{\s*\"\"\s*:\s*\"Program\"",
                        string=line,
                        flags=re.I
                    ):
                capture = True
            if capture:
                json_lines.append(line)
        return "\n".join(json_lines)

    @staticmethod
    def write_json_to_vir_file(obj: Dict[str, Any], fp: str) -> None:
        with open(fp, mode="w") as vir_f:
            json.dump(obj, fp=vir_f, indent=3)

    @staticmethod
    def run_driver(input_file: str) -> str:
        driver = "Driver3.jar"
        driver_class = "net.verdagon.vale.driver.Driver"
        driver_args = "run"
        proc = procrun(
            [
                "java",
                "-cp",
                driver,
                driver_class,
                driver_args,
                input_file
            ],
            check=True
        )
        return proc.stdout

    @classmethod
    def setUpClass(cls) -> None:
        input_file = "foo.vale"
        output_file = f"{os.path.splitext(input_file)[0]}.vir"
        output = cls.run_driver(input_file)
        vir_str = cls.extract_json_str(output)
        vir = json.loads(vir_str)
        cls.write_json_to_vir_file(vir, fp=output_file)

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
        proc = self.valec(['foo.vir'])
        self.assertEqual(proc.returncode, 0)
        proc = self.clang(["foo.o"])
        self.assertEqual(proc.returncode, 0)
        proc = self.aout()
        self.assertEqual(proc.returncode, 7)


if __name__ == '__main__':
    unittest.main()
