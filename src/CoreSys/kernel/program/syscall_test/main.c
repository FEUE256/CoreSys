#include <CoreSys.h>

static char buffer[128];
static int text_index = 0;

int syscall_test_main(void)
{
    CS_CORE core;
    cs_init(&core);

    core.sys.init();
    core.sys.clear();

    core.sys.write("Write what you want to print!\n");

    while (1)
    {
        char c = core.sys.read();

        if (c == '\r' || c == '\n')
        {
            buffer[text_index] = '\0';

            core.sys.write("\nYou entered: ");
            core.sys.write(buffer);
            core.sys.write("\n");

            core.sys.write("Write what you want to print!\n");

            text_index = 0;
        }
        else if (text_index < (int)sizeof(buffer) - 1)
        {
            buffer[text_index++] = c;

            char tmp[2] = {c, '\0'};
            core.sys.write(tmp);
        }
    }

    return 0;
}
