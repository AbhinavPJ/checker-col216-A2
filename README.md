# Usage

Only two code files are used:

- main.cpp: core simulator + dump states
- check.cpp: build + run main.cpp + exact output compare

First clone the repo inside your project root, then:

## Build

From checker-col216-A2:

```bash
g++ -std=c++17 check.cpp -o check
```

## Check all tests

```bash
./check
```

This compares raw bytes of:

- output from ./main <code.txt>
- ans*.txt

## Regenerate ans files

```bash
./check --write
```

This rewrites ans*.txt using current output from main.cpp.

## Contribute Testcase

1. Create a new test folder inside checker-col216-A2 (for example: test10).
2. Add your code files as code1.txt, code2.txt, ...
3. Regenerate expected outputs:

```bash
./check --write
```

4. Verify everything passes:

```bash
./check
```

5. Create a PR.

DISCLAIMER: please do not accidentally leak YOUR assignment code, Abhinav is not responsible for this.
I do not claim ownership of all code in this repository.
