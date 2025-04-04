/* pthread-lecex3.c */

/* Lecture Exercise 3 -- Practice Problem 7
 *
 * How many distinct possible outputs are there?
 * Show all possible outputs.
 *
 * How many bytes are dynamically allocated on the heap after
 *  all threads are joined back in to the main thread?
 *
 * And fix the memory leaks!
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

void * pthread_function( void * y )
{
  int * x = (int *)y;
  char * s = calloc( 16, sizeof( char ) );
  *s = ' ';
  strcat( s, "LAKERS" );
  for ( int i = 0 ; i < 8 ; i++ )
  {
    int tmp = *(x + i);
    tmp += i;
    *(x + i) = tmp;
  }
  fprintf( stderr, "%s\n", s );
  return NULL;
}

int main()
{
  int * z = calloc( 8, sizeof( int ) );
  pthread_t t1, t2;
  pthread_create( &t1, NULL, pthread_function, z );
  pthread_create( &t2, NULL, pthread_function, z );
  fprintf( stderr, "LET'S GO" );
  pthread_join( t2, NULL );
  pthread_join( t1, NULL );
  for ( int i = 0 ; i < 8 ; i++ ) printf( "%d%s", *(z + i), i == 7 ? "" : "-" );
  printf( "\n" );
  return EXIT_SUCCESS;
}
