FLAGS= -Wall -fsanitize=address -g -O0 -lreadline

emulator: emulator.c
	gcc -o $@ $< ${FLAGS}
