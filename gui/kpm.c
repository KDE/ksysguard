/* A small wrapper to call ksysguard --showprocesses */

#include <unistd.h>

int main(int argc, char **argv)
{
   char * const args[3] = { "ksysguard", "--showprocesses", 0 };
   return execvp("ksysguard", args);
}
