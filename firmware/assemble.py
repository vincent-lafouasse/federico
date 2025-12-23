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

for line in lines:
    print(line)
