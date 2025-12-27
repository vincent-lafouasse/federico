TARGET = federico
SOURCE = ./main.c

CC = cc
CFLAGS += -Wall -Wextra -std=c11

OBJECTS := $(SOURCE:%=build/%.o)
DEPS := $(OBJECTS:.o=.d)

.PHONY: all
all: build

.PHONY: run
run: build
	exec ./$(NAME)

.PHONY: build
build: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@

build/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf build

.PHONY: mrproper
mrproper: clean
	rm -rf $(TARGET)

# LSP stuff, don't worry about it
.PHONY: update
update: clean
	mkdir -p build
	bear --output build/compile_commands.json -- make build

# aliases
.PHONY: b c u r t vt
b: build
c: clean
u: update
r: run

-include $(DEPS)
