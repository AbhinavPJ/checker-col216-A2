from pathlib import Path
import re
import subprocess
import sys
import tempfile

THIS_DIR = Path(__file__).resolve().parent
ROOT = THIS_DIR.parents[1]
BASICS = ROOT / "Basics.h"
COMPILER = ROOT / "compiler.py"
BINARY = ROOT / "main"
CODE3 = THIS_DIR / "code3.txt"
ANS3 = THIS_DIR / "ans3.txt"

ROB_EXPECTED_CYCLES = {
    3: 19,
    4: 15,
    64: 15,
}


def read_text_auto(path: Path) -> str:
    data = path.read_bytes()
    if data.startswith(b"\xff\xfe") or data.startswith(b"\xfe\xff"):
        return data.decode("utf-16")
    return data.decode("utf-8", errors="ignore")


def normalize(text: str) -> str:
    lines = [ln for ln in text.splitlines() if not ln.startswith("=")]
    text = " ".join(lines)
    text = text.replace(".", " ").replace("|", " ").replace("+", " ")
    text = text.replace("[", " ").replace("]", " ")
    text = re.sub(r"\s+", " ", text).strip().lower()
    return text


def set_rob_size(size: int) -> None:
    content = BASICS.read_text(encoding="utf-8")
    updated, count = re.subn(
        r"(\brob_size\s*=\s*)\d+(\s*;)",
        rf"\g<1>{size}\g<2>",
        content,
        count=1,
    )
    if count != 1:
        raise RuntimeError("Could not find rob_size assignment in Basics.h")
    BASICS.write_text(updated, encoding="utf-8")


def compile_main() -> None:
    result = subprocess.run(
        ["make", "compile", "FILE=main.cpp"],
        cwd=ROOT,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        raise RuntimeError(f"Compilation failed:\n{result.stdout}\n{result.stderr}")


def run_code3() -> str:
    with tempfile.TemporaryDirectory() as td:
        tmp_code = Path(td) / CODE3.name
        tmp_code.write_bytes(CODE3.read_bytes())

        prep = subprocess.run(
            [sys.executable, str(COMPILER), str(tmp_code)],
            cwd=ROOT,
            capture_output=True,
            text=True,
        )
        if prep.returncode != 0:
            raise RuntimeError(f"Preprocess failed:\n{prep.stdout}\n{prep.stderr}")

        run = subprocess.run(
            [str(BINARY), str(tmp_code)],
            cwd=ROOT,
            capture_output=True,
            text=True,
        )
        if run.returncode != 0:
            raise RuntimeError(f"Execution failed:\n{run.stdout}\n{run.stderr}")
        return run.stdout


def patch_expected_cycles(ans_text: str, cycles: int) -> str:
    out = re.sub(
        r"after\s+\d+\s+cycles",
        f"after {cycles} cycles",
        ans_text,
        count=1,
        flags=re.IGNORECASE,
    )
    out = re.sub(
        r"\(CYCLE\s+\d+\)",
        f"(CYCLE {cycles})",
        out,
        count=1,
    )
    return out


def extract_cycles(output: str) -> int | None:
    m = re.search(r"after\s+(\d+)\s+cycles", output, flags=re.IGNORECASE)
    if m:
        return int(m.group(1))
    return None


def main() -> int:
    if not CODE3.exists() or not ANS3.exists():
        print("Missing code3.txt or ans3.txt in test7")
        return 2

    ans3_text = read_text_auto(ANS3)
    failures = 0

    for rob_size, expected_cycles in ROB_EXPECTED_CYCLES.items():
        try:
            set_rob_size(rob_size)
            compile_main()
            actual = run_code3()

            adjusted_expected = patch_expected_cycles(ans3_text, expected_cycles)
            actual_norm = normalize(actual)
            expected_norm = normalize(adjusted_expected)
            observed_cycles = extract_cycles(actual)

            cycles_ok = observed_cycles == expected_cycles
            match_ok = actual_norm == expected_norm
            ok = cycles_ok and match_ok

            status = "PASS" if ok else "FAIL"
            print(
                f"{status} rob_size={rob_size} "
                f"(observed_cycles={observed_cycles}, expected_cycles={expected_cycles})"
            )
            if not ok:
                failures += 1
        except Exception as exc:
            failures += 1
            print(f"FAIL rob_size={rob_size}: {exc}")

    set_rob_size(64)
    if failures:
        print(f"\nFailures: {failures}")
        return 1
    print("\nAll checks passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
