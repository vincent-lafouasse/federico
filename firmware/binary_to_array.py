SOURCE = "./busicom.bin"

with open(SOURCE, mode='rb') as file:
    data = file.read()

for b in data:
    print(f"    0x{b:02x},")
