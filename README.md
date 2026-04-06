# Checker Usage

## Run tests

Clone this repo inside your assignment root and run:

```bash
python checker-col216-A2/checker.py
```

What this does:
- Assumes that you have already generated a binary for your processor named 'main'.
- `checker.py` runs all pairs in folders named `test1`, `test2`, ...
- It matches `codeX.txt` with `ansX.txt`

## Contribute testcases

Rule: each contributed testcase set must be in a **new** test folder.

1. Create a new folder inside `checker-col216-A2`, for example `test67`.
2. Add one or more files named `code1.txt`, `code2.txt`, ...
3. Copy gen.py that is inside test2/ into test3/
4. Generate expected outputs using `gen.py` in that folder.

Example:

```bash
cd checker-col216-A2
mkdir test3
cd test3
#copy gen.py into test3,create code1.txt inside test3
python gen.py
```

This creates `ans1.txt`, `ans2.txt`, ... in UTF-16 format.

4. From repo root, verify everything:

```bash
python checker-col216-A2/checker.py
```

5. Commit and open a Pull Request with:
- new `testN/` folder
- all `code*.txt` files
- matching generated `ans*.txt` files
- a short note describing what the tests cover

DISCLAIMER: please do not accidentally leak YOUR assignment code, Abhinav is not responsible for this.
I do not claim ownership of all code in this repository.
