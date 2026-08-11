#!/usr/bin/env python3

import subprocess
import sys
import shutil

PROGRAM = "./codexion"

GREEN = "\033[92m"
RED = "\033[91m"
YELLOW = "\033[93m"
RESET = "\033[0m"

passed = 0
failed = 0


def have_program():
    return shutil.which(PROGRAM.replace("./", "")) is not None or shutil.which(PROGRAM)


def run(args, timeout=20):
    try:
        result = subprocess.run(
            [PROGRAM] + args,
            capture_output=True,
            text=True,
            timeout=timeout
        )
        return result.stdout + result.stderr
    except subprocess.TimeoutExpired:
        return "__TIMEOUT__"


def ok(name):
    global passed
    passed += 1
    print(f"{GREEN}✓{RESET} {name}")


def fail(name, reason=""):
    global failed
    failed += 1
    print(f"{RED}✗{RESET} {name}")
    if reason:
        print("   ", reason)


def count(output, text):
    return output.count(text)


print("=" * 45)
print(" Codexion Test Suite")
print("=" * 45)
print()

if not have_program():
    print("Could not find ./codexion")
    sys.exit(1)

#
# Invalid arguments
#

print("[1] Invalid arguments")

tests = [
    [],
    ["1"],
    ["0", "100", "100", "100", "100", "1", "0", "fifo"],
    ["5", "100", "100", "100", "100", "1", "0", "abc"],
]

for t in tests:
    out = run(t)
    if "Error" in out or out == "":
        ok(" ".join(t) if t else "no arguments")
    else:
        fail(" ".join(t), "expected parser error")

print()

#
# Single coder
#

print("[2] Single coder")

out = run(["1", "800", "200", "200", "200", "1", "0", "fifo"])

if count(out, "has taken a dongle") == 1:
    ok("one dongle taken")
else:
    fail("single coder", "wrong dongle count")

if "is compiling" not in out:
    ok("never compiles")
else:
    fail("compiled")

if "burned out" in out:
    ok("burnout")
else:
    fail("missing burnout")

print()

#
# Two coders
#

print("[3] Two coders")

out = run(["2", "3000", "200", "200", "200", "2", "0", "fifo"])

c = count(out, "is compiling")

if c == 4:
    ok("4 compiles")
else:
    fail("compile count", f"expected 4 got {c}")

if "burned out" not in out:
    ok("finished")
else:
    fail("unexpected burnout")

print()

#
# Three coders
#

print("[4] Three coders")

out = run(["3", "4000", "200", "200", "200", "2", "0", "fifo"])

c = count(out, "is compiling")

if c == 6:
    ok("6 compiles")
else:
    fail("compile count", f"expected 6 got {c}")

print()

#
# FIFO
#

print("[5] FIFO")

out = run(["5", "3000", "100", "100", "100", "3", "0", "fifo"])

if count(out, "is compiling") == 15:
    ok("compile count")
else:
    fail("fifo compile count")

print()

#
# EDF
#

print("[6] EDF")

out = run(["5", "3000", "100", "100", "100", "3", "0", "edf"])

if count(out, "is compiling") == 15:
    ok("compile count")
else:
    fail("edf compile count")

print()

#
# 20 coders one compile
#

print("[7] Twenty coders")

out = run(["20", "20000", "200", "200", "200", "1", "300", "fifo"], timeout=30)

c = count(out, "is compiling")

if c == 20:
    ok("exactly 20 compiles")
else:
    fail("compile count", f"expected 20 got {c}")

print()

#
# Impossible compile
#

print("[8] Impossible compile")

out = run(["2", "100", "200", "100", "100", "5", "0", "fifo"])

if "burned out" in out:
    ok("burnout")
else:
    fail("expected burnout")

print()

#
# Cooldown impossible
#

print("[9] Cooldown impossible")

out = run(["5", "500", "50", "50", "50", "10", "300", "fifo"])

if "burned out" in out:
    ok("burnout")
else:
    fail("expected burnout")

print()

#
# Long stress
#

print("[10] Stress")

out = run(["5", "1000", "100", "100", "100", "100", "0", "fifo"], timeout=60)

if "__TIMEOUT__" not in out:
    ok("completed")
else:
    fail("program hung")

print()

#
# Thread creation
#

print("[11] Large thread count")

out = run(["200", "10000", "100", "100", "100", "1", "0", "fifo"], timeout=60)

if "__TIMEOUT__" not in out:
    ok("200 coders")
else:
    fail("timeout")

print()

#
# Graceful failure
#

print("[12] Thread limit")

out = run(["20000", "2000", "200", "200", "200", "1", "0", "fifo"], timeout=30)

if "pthread_create" in out or "Error" in out:
    ok("graceful failure")
else:
    fail("expected thread creation failure")

print()

print("=" * 45)
print(f"Passed : {passed}")
print(f"Failed : {failed}")
print("=" * 45)

if failed == 0:
    print(f"{GREEN}ALL TESTS PASSED{RESET}")
else:
    print(f"{RED}SOME TESTS FAILED{RESET}")
