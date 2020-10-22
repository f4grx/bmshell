#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "shell.h"

#include <stdio.h>

/* ------------------------------------------------------------------------- */
static void convert(void (*write)(uint8_t ch), uint32_t val, int base, bool withsign, bool upper)
  {
    char numbuf[16];
    int index = 16;

    if(!val)
      {
        write('0');
        return;
      }

    if(withsign)
      {
        if(val >> 31)
          {
            write('-');
          }
        val &= ~(1LU<<31);
      }

    while(val > 0)
      {
        index -= 1;
        numbuf[index] = val % base;
        val = val / base;
      }
    while(index < 16)
      {
        if(numbuf[index] < 10)
          {
            write(numbuf[index] + '0');
          }
        else
          {
            if(upper)
              {
                write(numbuf[index] + 'A');
              }
            else
              {
                write(numbuf[index] + 'a');
              }
          }
        index += 1;
      }
  }

/* ------------------------------------------------------------------------- */
uint16_t cbvprintf(void (*write)(uint8_t ch), const char *fmt, va_list ap)
  {
    int index = 0;
    bool done = false;
    char ch;
    char *ptr;
    bool upper;
    bool sign;
    int base;
    bool islong;

    while(!done)
      {
        ch = fmt[index++];
        switch(ch)
          {
          case 0:  done = true; break;

norm:     default: write(ch); break;
          case '%':
            base = 10; upper = false; sign = false; islong = false;
again:
            ch = fmt[index++];
            switch(ch)
              {
                case 0: done = true; break;
                case '%': goto norm;
                case 'l': islong = true; goto again;
                case 's': ptr = va_arg(ap, char*); while(*ptr) {write(*ptr); ptr++;} break;
                case 'X': upper = true;
                case 'x': base = 16;
                case 'u': sign = true;
                case 'd':
                  {
                    uint32_t val;
                    if(islong) val = va_arg(ap, long); else val = va_arg(ap, int);
                    convert(write, val, base, sign, upper);
                    break;
                  }
              }
          }
      }
    return 0;
  }

/* ------------------------------------------------------------------------- */
void shell_printf(const struct shell *sh, const char *fmt, ...)
  {
    va_list ap;
    va_start(ap, fmt);
    cbvprintf(sh->write, fmt, ap);
    va_end(ap);
  }

/* ------------------------------------------------------------------------- */
static int readline(const struct shell *sh)
  {
    uint8_t ch;
    int index = 0;
    shell_printf(sh, "%s", sh->prompt);
    while(true)
      {
        ch = sh->read();
//printf("%02X",ch); fflush(stdout);
        if(ch==8 || ch==127)
          {
            if(index>0)
              {
                index -= 1;
                sh->write(' ');
                sh->write(ch);
              }
          }
        else if(ch==10 || ch==13)
          {
            break;
          }
        else if(ch<32 || ch>127)
          {
            continue;
          }
        else
          {
            if(index < sh->bufsize)
              {
                sh->buffer[index++] = ch;
                sh->write(ch);
              }
          }
      } //while
    return index;
  }

/* ------------------------------------------------------------------------- */
/* we split at spaces unless we are in quotes. Buffer contents is modified
   to insert nulls at proper places.
 */
static int split(const struct shell *sh)
  {
    int cur = 0;
    char quote_state = ' ';
    char *ptr = (char*)sh->buffer;
again:
    /* Check end of buffer */
    if(! *ptr)
      {
        return cur;
      }

    /* Check for quotes at beginning of arg */
    if( (*ptr == '\'') || (*ptr == '\"') )
      {
        //we are within simple or double quotes
        quote_state = *ptr;
        ptr += 1;

        /* Check end of buffer */
        if(! *ptr)
          {
            return cur;
          }
      }

    /* store current pointer into current arg */
    sh->argv[cur] = ptr;
    cur += 1;

    /* Now find end of arg: next similar quote/space */
    while(*ptr && *ptr != quote_state)
      {
        ptr += 1;
        /* Check end of buffer */
        if(! *ptr)
          {
            return cur;
          }
      }

    /* split arg at quote */
    *ptr = 0;
    ptr += 1;

    /* Check end of buffer */
    if(! *ptr)
      {
        return cur;
      }

    /* skip spaces after quote */
    quote_state = ' ';
    while(*ptr && *ptr == quote_state)
      {
        ptr += 1;

        /* Check end of buffer */
        if(! *ptr)
          {
            return cur;
          }
      }

    /* manage next arg */
    if(cur < sh->argvsize)
      {
        goto again;
      }

    return cur;
  }

/* ------------------------------------------------------------------------- */
static const struct shell_cmd *find(const struct shell *sh, const char *name)
  {
    const struct shell_cmd *cur = sh->commands;
    while(cur->name)
      {
        if(!strcmp(cur->name, name))
          {
            return cur;
          }
        cur = cur + 1;
      }
    return NULL;
  }

/* ------------------------------------------------------------------------- */
void shell_process(const struct shell *sh)
  {
    const struct shell_cmd *cmd;
    int argc;
    memset(sh->buffer, 0, sh->bufsize);
    readline(sh);
    sh->write('\n');
    argc = split(sh);
      {
        int i;
        shell_printf(sh, "argc: %d\r\n", argc);
        for(i=0;i<argc;i++)
            shell_printf(sh, "argv[%d]: {%s}\r\n", i, sh->argv[i]);
      }
    cmd = find(sh, sh->argv[0]);
    if(cmd)
      {
        cmd->runtime(sh, argc, sh->argv);
      }
    else
      {
        shell_printf(sh, "%s: command not found\n", sh->argv[0]);
      }
  }

/* ------------------------------------------------------------------------- */
int shell_help(const struct shell *sh, int argc, char **argv)
  {
    const struct shell_cmd *cur = sh->commands;
    while(cur->name)
      {
        shell_printf(sh,"%s - %s\n", cur->name, cur->desc);
        cur = cur + 1;
      }
    return 0;
  }

