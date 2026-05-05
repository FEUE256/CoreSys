#include <stdint.h>
#include <drivers/sys/main.h>

void sys_write(const char* s);
void sys_halt(void);
char sys_read(void);
void sys_init(void);
void sys_clear(void);

static char buffer[128];
static int text_index = 0;

int syscall_test_main(void)
{
    sys_init();
    sys_clear();

    sys_write("Write what you want to print!\n");

    while (1)
    {
        char c = sys_read();

        if (c == '\r' || c == '\n')
        {
            buffer[text_index] = '\0';

            sys_write("\nYou entered: ");
            sys_write(buffer);
            sys_write("\n");

            sys_write("Write what you want to print!\n");

            text_index = 0;
        }
        else if (text_index < (int)sizeof(buffer) - 1)
        {
            buffer[text_index++] = c;

            char tmp[2] = {c, '\0'};
            sys_write(tmp);
        }
    }

    return 0;
}
