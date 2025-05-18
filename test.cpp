#include <unistd.h>

#include <cstring>

int main() {
    const char* msg = "1,2313\n";
    write(STDERR_FILENO, msg, strlen(msg));
    write(STDOUT_FILENO, msg, strlen(msg));
}