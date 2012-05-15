#include "dre2.h"

int
dre2_backtrack_recursive( struct dre2 *graph, unsigned char *input, int pos, int id, int **state, int **group_open, int **group_close )
{
  int i, j;
  int offset;
  unsigned char *tp;
  tp = input + pos;

  if ( id == graph->count - 1 )
    return true;

  if ( pos >= strlen( input ) )
    return false;

  if ( ( *state )[id] )
    return false;
  ( *state )[id] = true;

  if ( graph->v[id].c != DRE2_GROUP_OPEN && graph->v[id].c != DRE2_GROUP_CLOSE )
  {
    if ( !dre2_char_matches( graph, &graph->v[id], *tp ) )
      return false;
    offset = 1;
  } else
  {
    offset = 0;
  }

  if ( graph->v[id].c == DRE2_GROUP_OPEN && graph->v[id].group_id > 0 )
  {
    ( *group_open )[graph->v[id].group_id - 1] = pos;
  } else if ( graph->v[id].c == DRE2_GROUP_CLOSE )
  {
    ( *group_close )[graph->v[id].group_id - 1] = pos;
  }

  if ( offset )
  {
    for ( i = 0; i < graph->count; i++ )
      ( *state )[i] = false;
  }

  for ( i = 0; i < graph->v[id].n_count; i++ )
  {
    if ( dre2_backtrack_recursive( graph, input, pos + offset, graph->v[id].n[i], state, group_open, group_close ) )
      return true;
    for ( j = 0; j < graph->count; j++ )
      ( *state )[j] = false;
  }
  return false;
}

int
dre2_backtrack_match( struct dre2 *graph, unsigned char *input )
{
  int i, j;
  int matched, pos;
  int *state;
  int *group_open, *group_close;

  // Allocate some memory for the state table to ensure we don't ever get in an infinite loop.
  state = ( int * )calloc( graph->original->count, sizeof( int ) );
  group_open = ( int * )calloc( graph->original->group_count, sizeof( int ) );
  group_close = ( int * )calloc( graph->original->group_count, sizeof( int ) );

  pos = 0;
  if ( ( matched = dre2_backtrack_recursive( graph->original, input, pos, 0, &state, &group_open, &group_close ) ) )
  {
    for ( i = 1; i < graph->original->group_count; i++ )
    {
      printf( "Group: ", i );
      for ( j = group_open[i - 1]; j < group_close[i - 1]; j++ )
        printf( "%c", input[j] );
      printf( "\n" );
    }
  }
  free( state ); state = NULL;
  free( group_open ); group_open = NULL;
  free( group_close ); group_close = NULL;
  return matched;
}
