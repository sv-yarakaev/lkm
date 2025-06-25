import time

MODULE = "ehlo"
BASE = f"/sys/module/{MODULE}/parameters"
PATH_STRING = f"{BASE}/my_str"
PATH_INDEX = f"{BASE}/idx"
PATH_CHAR = f"{BASE}/ch_val"

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

def replace_string(new_str):
    print(f"[>] Replacing string with: \"{new_str}\"")
    for i, c in enumerate(new_str):
        update_char_at(i + 1, c)
        current = read_string()
        assert current[i] == c, f"Mismatch at {i + 1}: expected '{c}', got '{current[i]}'"
        print(f"    [+] {i + 1}: '{c}' => {current}")
    print(f"[+] Replacement done: \"{read_string()}\"")

def test_string_modification():
    print("[+] Starting kernel module test...")

    original = read_string()
    print(f"[i] Original string: \"{original}\"")

    test_target = "Default string!"

    if len(test_target) > len(original):
        raise Exception("Target test string is longer than current kernel string")

    for i, char in enumerate(test_target):
        update_char_at(i + 1, char)
        current = read_string()
        assert current[i] == char, f"Mismatch at index {i}: expected {char}, got {current[i]}"
        print(f"[+] Changed index {i + 1} to '{char}' → {current}")

    final = read_string()
    assert final.startswith(test_target), "Final string mismatch"
    print(f"[+] Final string: \"{final}\"")
    replace_string("Changed string")
    print("[+] All tests passed.")

if __name__ == "__main__":
    test_string_modification()
