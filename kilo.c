/*
 * Copyright 2018, Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

/*** includes ***/
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/
struct termios orig_termios;

/*** terminal ***/

// Prints error message and exits program
void die(const char *s) {
	perror(s);
	exit(1);
}

// Return terminal to original settings on exit
void disableRawMode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
		die("tcsetattr");
	}
}

// Take control over terminal settings and flags
void enableRawMode() {
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
		die("tcgetattr");
	}
	atexit(disableRawMode);
	
	struct termios raw = orig_termios;
	// Flags to disable
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_iflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
		die("tcsetattr");
	}
}

// Wait for one keypresss and return it
char editorReadKey() {
	int nread;
	char c;
	while ((nread = read(STDIN_FILENO,  &c, 1)) != 1) {
		if (nread == -1 && errno != EAGAIN) {
			die("read");
		}
	}
	return c;
}

/*** output ***/

void editorRefreshScreen() {
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

// Wait for keypress, then handle it
void editorProcessKeypress() {
	char c = editorReadKey();
	
	switch (c) {
		case CTRL_KEY('q'):
			exit(0);
			break;
	}
}

/*** init ***/
int main() {
	enableRawMode();
	
	while (1) {
		editorRefreshScreen();
		editorProcessKeypress();
	}
	
	return 0;
}
