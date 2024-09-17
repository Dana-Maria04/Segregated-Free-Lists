CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

TARGET = sfl
SRCS = main.c dll.c command.c

build: $(TARGET)

run_sfl: build
	valgrind --leak-check=full --track-origins=yes ./sfl
$(TARGET):    $(SRCS)
	$(CC)    $(CFLAGS) $^ -o $@ -lm



pack:
	zip -r cs.zip README.md cs/cs.sh cs/checkpatch.pl cs/spelling.txt cs/const_structs.checkpatch cs/tests/

clean:
	rm -f $(TARGET)

.PHONY: pack clean