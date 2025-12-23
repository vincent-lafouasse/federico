SOURCE = "./busicom-141.s"


with open(SOURCE, "r") as file:
    lines = file.read().splitlines()


def trim_comment(line):
    comment_start = -1
    for i, c in enumerate(line):
        if c == ";":
            comment_start = i
            break

    if comment_start == -1:
        return line
    else:
        return line[0:comment_start]


lines = [line.lstrip() for line in lines]
lines = [trim_comment(line) for line in lines]
lines = [line for line in lines if line]


def ints_to_hexstr(numbers):
    out = "[ "
    for i in numbers:
        out += f"0x{i:02x}, "
    out += "]"
    return out


def is_hex_byte(token):
    return (
        len(token) == 2
        and all(c in "1234567890abcdef" for c in token.lower())
        and not token.endswith(":")
    )


def extract_instruction_data(tokens):
    instruction = []
    for token in tokens:
        if token.endswith(":"):
            continue

        if is_hex_byte(token):
            instruction.append(int(token, 16))
        else:
            break

    if instruction and instruction[0] == 0:
        assert all(code == 0 for code in instruction)
    else:
        assert len(instruction) <= 2

    return instruction


data = []
for line in lines:
    tokens = line.split()
    address = int(tokens[0], 16)
    print(address, tokens[1:])

    if address != len(data):
        print(f"Expected offset {hex(address)} was {hex(len(data))}")
        print(f"for line\n\t{line}")
        raise ValueError()

    instruction = extract_instruction_data(tokens[1:])
    print(f"instruction: {ints_to_hexstr(instruction)}")
    data.extend(instruction)

assert len(data) == 0x500
with open("busicom.bin", "wb") as f:
    f.write(bytes(data))
