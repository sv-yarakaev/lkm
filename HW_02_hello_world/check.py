import time

MODULE = "ehlo"
BASE = f"/sys/module/{MODULE}/parameters"
PATH_STRING = f"{BASE}/string"
PATH_INDEX = f"{BASE}/index"
PATH_CHAR = f"{BASE}/char"

def read_string():
    with open(PATH_STRING, "r") as f:
        return f.read().strip()

def write_index(i):
    with open(PATH_INDEX, "w") as f:
        f.write(str(i))

def write_char(c):
    with open(PATH_CHAR, "w") as f:
        f.write(c)

def update_char_at(i, c):
    write_index(i)
    write_char(c)
    time.sleep(0.05)  

def test_string_modification():
    print("[+] Starting kernel module test...")

    original = read_string()
    print(f"[i] Original string: \"{original}\"")

    test_target = "Hello, MODULE test!"

    if len(test_target) > len(original):
        raise Exception("Target test string is longer than current kernel string")

    for i, char in enumerate(test_target):
        update_char_at(i + 1, char)
        current = read_string()
        assert current[i] == char, f"Mismatch at index {i}: expected {char}, got {current[i]}"
        print(f"[✓] Changed index {i + 1} to '{char}' → {current}")

    final = read_string()
    assert final.startswith(test_target), "Final string mismatch"
    print(f"[✓] Final string: \"{final}\"")
    print("[✓] All tests passed.")

if __name__ == "__main__":
    test_string_modification()
