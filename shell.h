#ifndef __SHELL_H__
#define __SHELL_H__

#include <stdint.h>

struct shell;

struct shell_cmd
  {
    const char *name;
    const char *desc;
    int (*runtime)(const struct shell *sh, int argc, char **argv);
  };

struct shell
  {
    void           (*write)(uint8_t ch);
    uint8_t        (*read) (void);
    uint8_t         *buffer;
    uint16_t         bufsize;
    char           **argv;
    char             argvsize;
    char            *prompt;
    struct shell_cmd commands[];
  };

void shell_process(const struct shell *sh);
void shell_printf (const struct shell *sh, const char *fmt, ...);

int shell_help   (const struct shell *sh, int argc, char **argv);

#endif /* __SHELL_H__ */
