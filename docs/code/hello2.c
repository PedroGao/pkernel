#include <unistd.h>

int main()
{
    int stdout_fd = 1;
    char *str = "Hello World\n";
    int length = 13;
    write(stdout_fd, str, length);
    return 0;
}
