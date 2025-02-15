/* fork-multiple-children-v2.c */

/* CTRL-C sends a SIGINT to all processes in the group:
 *
 * bash$ kill -SIGINT -<parent>
 *
 * To only send the signal to the parent process:
 *
 * bash: kill -SIGINT <parent>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

int child_process();
void parent_process( int children, pid_t * pids );

int main( int argc, char ** argv )
{
  printf( "PARENT: my PID is %d\n", getpid() );

  if ( argc != 2 )
  {
    fprintf( stderr, "ERROR: invalid arguments\n" );
    fprintf( stderr, "USAGE: %s <#-of-children>\n", *(argv+0) );
    return EXIT_FAILURE;
  }

  int children = atoi( *(argv + 1) );

  pid_t * pids = calloc( children, sizeof( pid_t ) );
  if ( pids == NULL ) { perror( "calloc() failed" ); return EXIT_FAILURE; }

  /* create child processes */
  for ( int i = 0 ; i < children ; i++ )
  {
    pid_t p = fork();
    if ( p == -1 ) { perror( "fork() failed" ); return EXIT_FAILURE; }

    /* CHILD */
    if ( p == 0 )
    {
      free( pids );  /* child processes do not use this array... */
      srand( i * time( NULL ) );
      return child_process();
   /* ^^^^^^ be sure we EXIT the child process here... */
    }

    /* PARENT */
    *(pids + i) = p;
  }

  parent_process( children, pids );

  return EXIT_SUCCESS;
}

/* each child process will sleep for a short period of time... */
int child_process()
{
/*  srand( time( NULL ) ); */
  int t = 10 + ( rand() % 21 );   /* [10,30] seconds */
  printf( "CHILD %d: I'm gonna nap for %d seconds\n", getpid(), t );
  sleep( t );
  printf( "CHILD %d: I'm awake!\n", getpid() );
  return t;
}

/* TO DO: Replace the -1 argument in waitpid() with specific
 *         PID values from the pids array...
 */
void parent_process( int children, pid_t * pids )
{
  int status;

  printf( "PARENT: I'm waiting for my child processes to exit\n" );

  while ( children > 0 )
  {
    pid_t child_pid = waitpid( -1, &status, 0 );  /* BLOCKING */
    /*                         ^^
     *                     wait for any of my child processes...
     */

    children--;

    printf( "PARENT: child process %d terminated...\n", child_pid );

    if ( WIFSIGNALED( status ) )
    {
      printf( "PARENT: ...abnormally (killed by a signal)\n" );
    }
    else if ( WIFEXITED( status ) )
    {
      int exit_status = WEXITSTATUS( status );
      printf( "PARENT: ...normally with exit status %d\n", exit_status );
    }
  }

  free( pids );

  printf( "PARENT: All done\n" );
}
