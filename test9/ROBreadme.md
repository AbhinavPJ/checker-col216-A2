# ROB test helper: `rob_change.py`

`rob_change.py` automates ROB-size validation for `test7/code3.txt`.

It does the following for each ROB size in `{3, 4, 64}`:
1. Edits `../../Basics.h` and sets `rob_size` to that value.
2. Compiles the simulator (`make compile FILE=main.cpp`).
3. Runs `code3.txt` through `../../compiler.py` and then `../../main`.
4. Compares simulator output against `ans3.txt`, while allowing the cycle count difference:
   - ROB size `3` -> expected cycle count `19`
   - ROB size `4` -> expected cycle count `15`
   - ROB size `64` -> expected cycle count `15`

It prints `PASS`/`FAIL` per ROB size and exits non-zero on failure.
After completion, it restores `rob_size` back to `64`.

## How to run

From `Solution/checker-col216-A2/test7`:

```bash
python3 rob_change.py
```
