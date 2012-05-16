#include "dre2.h"

// Recursive backtracking function - also sets group open/close positions.
int dre2_backtrack_recursive( struct dre2 *graph, unsigned char *input, int pos, int id, int **state, int **group_open, int **group_close )
{
  int i, j;
  int offset;
  int g_open, g_close;
  unsigned char *tp;

  tp = input + pos;

  // If we are at the regex close node, then we've matched as much as possible, return.
  if ( id == graph->count - 1 )
    return true;

  // If we are past the end of input, we can't match, return false.
  if ( pos > strlen( input ) )
    return false;

  // Make sure we haven't already tried this node.
  if ( ( *state )[id] )
    return false;
  ( *state )[id] = true;

  // Match the input character if it's not a group.
  if ( graph->v[id].c != DRE2_GROUP_OPEN && graph->v[id].c != DRE2_GROUP_CLOSE )
  {
    if ( !dre2_char_matches( graph, &graph->v[id], *tp ) )
      return false;
    offset = 1;
  } else
  {
    offset = 0;
  }

  // Set the group open and close positions, but also track where it was so we can revert later if needed.
  if ( graph->v[id].c == DRE2_GROUP_OPEN && graph->v[id].group_id > 0 )
  {
    g_open = ( *group_open )[graph->v[id].group_id - 1];
    ( *group_open )[graph->v[id].group_id - 1] = pos;
  } else if ( graph->v[id].c == DRE2_GROUP_CLOSE && graph->v[id].group_id > 0 )
  {
    g_close = ( *group_close )[graph->v[id].group_id - 1];
    ( *group_close )[graph->v[id].group_id - 1] = pos;
  }

  // If offset is not 0, then we matched an input character, so advance input pos and reset state table.
  if ( offset )
  {
    pos++;
    for ( i = 0; i < graph->count; i++ )
      ( *state )[i] = false;
  }

  // Check each of this nodes neighbors.
  for ( i = 0; i < graph->v[id].n_count; i++ )
  {
    // If the recursive function returns true, the regex matched.
    if ( dre2_backtrack_recursive( graph, input, pos, graph->v[id].n[i], state, group_open, group_close ) )
      return true;
  }

  // Revert group open/close on failure.
  if ( graph->v[id].c == DRE2_GROUP_OPEN && graph->v[id].group_id > 0 )
  {
    ( *group_open )[graph->v[id].group_id - 1] = g_open;
  } else if ( graph->v[id].c == DRE2_GROUP_CLOSE && graph->v[id].group_id > 0 )
  {
    ( *group_close )[graph->v[id].group_id - 1] = g_close;
  }
  return false;
}

// Perform backtracking match, returning submatch info (if any)
unsigned char **dre2_backtrack_match( struct dre2 *graph, unsigned char *input )
{
  int i, j;
  int matched, pos;
  int *state;
  int *group_open, *group_close;
  unsigned char **submatches;

  // Allocate some memory for the state table to ensure we don't ever get in an infinite loop.
  state = ( int * )calloc( graph->original->count, sizeof( int ) );

  // Allocate some memory for extracting submatches.
  if ( graph->original->group_count > 1 )
  {
    group_open = ( int * )calloc( graph->original->group_count, sizeof( int ) );
    group_close = ( int * )calloc( graph->original->group_count, sizeof( int ) );
    submatches = ( unsigned char ** )calloc( graph->original->group_count, sizeof( unsigned char * ) );
    for ( i = 1; i < graph->original->group_count; i++ )
      submatches[i - 1] = NULL;
  } else
  {
    submatches = NULL;
    group_open = NULL;
    group_close = NULL;
  }

  if ( ( matched = dre2_backtrack_recursive( graph->original, input, 0, 0, &state, &group_open, &group_close ) ) )
  {
    for ( i = 1; i < graph->original->group_count; i++ )
    {
      if ( group_close[i - 1] > group_open[i - 1] )
      {
        submatches[i - 1] = ( unsigned char * )calloc( group_close[i - 1] - group_open[i - 1] + 1, sizeof( unsigned char ) );
        sprintf( submatches[i - 1], "%.*s", group_close[i - 1] - group_open[i - 1], input + group_open[i - 1] );
      }
    }
  }

  // Free up memory used.
  if ( graph->original->group_count > 1 )
  {
    free( state ); state = NULL;
    free( group_open ); group_open = NULL;
    free( group_close ); group_close = NULL;
  }

  return submatches;
}
