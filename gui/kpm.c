/* A small wrapper to call ksysguard --showprocesses */

#include <unistd.h>

int main()
{
  return execlp( "ksysguard", "ksysguard", "--showprocesses", 0 );
}
