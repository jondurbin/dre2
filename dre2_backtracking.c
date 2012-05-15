#include "dre2.h"

int
dre2_backtrack_recursive( struct dre2 *graph, unsigned char *input, unsigned char *original, int id )
{
  int i, j;
  int offset;

  printf( "Pos: %d\n", input - original );
  printf( "Len: %d\n", strlen( original ) - 1 );
  if ( id == graph->count - 1 && input - original == strlen( original ) - 1 )
    return true;

  if ( graph->v[id].c != DRE2_GROUP_OPEN && graph->v[id].c != DRE2_GROUP_CLOSE )
  {
    if ( !dre2_char_matches( graph, &graph->v[id], *input ) )
    {
      return false;
    } else
    {
      printf( "Match: %c\n", *input );
    }
    offset = 1;
  } else
  {
    offset = 0;
  }

  for ( i = 0; i < graph->v[id].n_count; i++ )
  {
    printf( "Checking node %d\n", graph->v[id].n[i] );
    if ( dre2_backtrack_recursive( graph, input + offset, original, graph->v[id].n[i] ) )
      return true;
  }
  return false;
}

int
dre2_backtrack_match( struct dre2 *graph, unsigned char *input )
{
  int i, j;
  print_dre2( graph->original );

  if ( dre2_backtrack_recursive( graph->original, input, input, 0 ) )
    printf( "Success!\n" );
}
