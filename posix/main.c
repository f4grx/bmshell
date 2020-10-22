#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "shell.h"

void shwrite(uint8_t ch)
  {
    if(ch == '\n') putchar('\r');
    putchar((int)ch);
  }

uint8_t shread(void)
  {
    return (uint8_t)getchar();
  }

uint8_t shbuf[128];
char* shargv[16];

const struct shell sh =
  {
    .write    = shwrite,
    .read     = shread,
    .buffer   = shbuf,
    .bufsize  = sizeof(shbuf),
    .argv     = shargv,
    .argvsize = sizeof(shargv)/sizeof(shargv[0]),
    .prompt   = "pc_shell $ ",
    .commands = {
      {"help", "list commands", shell_help},
      {NULL, NULL, NULL}
    }
  };

struct termios term;

int main(int argc, char **argv)
  {
    tcgetattr(fileno(stdin), &term);
    term.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    term.c_oflag &= ~OPOST;
    term.c_lflag &= ~(ECHO | ECHONL | ICANON | /*ISIG |*/ IEXTEN);
    term.c_cflag &= ~(CSIZE | PARENB);
    term.c_cflag |= CS8;

    tcsetattr(fileno(stdin), 0, &term);

    while(true)
      {
        shell_process(&sh);
      }
  }
