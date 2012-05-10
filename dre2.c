#include "dre2.h"

int
main( int argc, char *argv[] )
{
  if ( argc != 2 )
  {
    printf( "Usage: cat file.txt | %s <regex>\n", argv[0] );
    return 0;
  }
  struct dre2 graph;
  graph = dre2_parse( ( unsigned char * )argv[1] );
  if ( graph.v == NULL )
  {
    printf( "Failed to parse!\n" );
    return 0;
  }

  char *buf;
  buf = ( unsigned char * )calloc( 0x10000, 1 );
  int count = 0;
  while ( fgets( buf, 0x10000 - 1, stdin ) )
  {
    if ( dre2_match( &graph, buf ) )
    {
      printf( "%s", buf );
      count++;
    }
  }
  cleanup_dre2( &graph );
  free( buf );
  return 0;
}
