#include <stdio.h>

int main( int argc, char *argv[] )
{
  int howMuchToSleep = 10; // in seconds
  if( argc == 2 )
    howMuchToSleep = atoi(argv[1]);

  printf("Will sleep for  %d seconds\n", howMuchToSleep);
  sleep(howMuchToSleep);

  printf("Enough is enough. I just slept for  %d seconds! \n", howMuchToSleep);
  return(howMuchToSleep);
}
