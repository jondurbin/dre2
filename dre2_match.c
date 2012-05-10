#include "dre2.h"

// Match a single node.
int
dre2_char_matches( struct dre2_node *node, unsigned char c )
{
  if ( node->c >= 0 )
  {
    if ( c == node->c )
      return true;
    return false;
  }
  if ( node->c == DRE2_CHAR_CLASS )
    return node->possible[c];

  switch( node->c )
  {
    case DRE2_ALPHA:
      if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) )
        return true;
      return false;
    case DRE2_WORD:
      if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || ( c == '-' ) )
        return true;
      return false;
    case DRE2_DOMAIN:
      if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || ( c == '-' ) || ( c == '.' ) )
        return true;
      return false;
    case DRE2_URL:
      if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || ( c == '-' ) || ( c == '.' ) || ( c == '/' ) || ( c = ':' ) || ( c == '_' ) || ( c == '@' ) )
        return true;
      return false;
    case DRE2_DIGIT:
      if ( c >= '0' && c <= '9' )
        return true;
      return false;
    case DRE2_BORDER:
      if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || ( c == '-' ) )
        return false;
      return true;
    case DRE2_WHITE_SPACE:
      if ( c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\n' )
        return true;
      return false;
    case DRE2_SIM_DIGIT:
      if ( c >= '0' && c <= '9' )
        return true;
      if ( c == 215 || c == 225 || c == 226 || c == 231 || c == 238 || c == 239 || c == 247 || c == 250 || c == 254 )
        return true;
      return false;
    case DRE2_OBF_DIGIT:
      if ( c == 215 || c == 225 || c == 226 || c == 231 || c == 238 || c == 239 || c == 247 || c == 250 || c == 254 )
        return true;
      return false;
    case DRE2_HEX:
      if ( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ) )
        return true;
      return false;
  }
  return false;
}

// Matcher process.
unsigned char *
dre2_matcher( struct dre2 *graph, unsigned char *begin_ptr, unsigned char *input, int start, int direction, int length )
{
  int i, j, k;
  int *swapper, *next_nodes;
  int iter, l_iter;
  int matched;
  int r;
  int count;
  int *reachable, *r_temp;

  reachable = graph->reachable;
  r_temp = graph->r_temp;

  i = 0;
  next_nodes = direction == DRE2_LEFT ? graph->v[start].p : graph->v[start].n;
  count = direction == DRE2_LEFT ? graph->v[start].p_count : graph->v[start].n_count;

  if ( start == 0 || start == graph->count - 1 )
  {
    for ( i = 0; i < count; i++ )
      reachable[i] = next_nodes[i];
    l_iter = i;
  } else
  {
    reachable[0] = start;
    l_iter = 1;
  }

  struct dre2_node *node;
  int complete = false;
  int found;

  while ( input - begin_ptr <= length && input - begin_ptr >= 0 )
  {
    // Check for valid states.
    i = 0;
    iter = 0;
    matched = false;
    for ( i = 0; i < l_iter; i++ )
    {
      r = reachable[i];
      node = &graph->v[r];
      // Check if the character matches the node.
      if ( ( node->c == DRE2_EOF && ( ( *input == ' ' && input - begin_ptr + 1 == length - 1 ) || *input == '\0' ) ) || dre2_char_matches( node, *input ) )
      {
        matched = true;

        // Set the next state to include this node's neighbors/parents.
        next_nodes = direction == DRE2_LEFT ? node->p : node->n;
        count = direction == DRE2_LEFT ? node->p_count : node->n_count;
        for ( j = 0; j < count; j++ )
        {
          found = false;
          for ( k = 0; k < iter; k++ )
          {
            if ( r_temp[k] == next_nodes[j] )
              found = true;
              break;
          }
          if ( !found )
            r_temp[iter++] = next_nodes[j];
          if ( ( direction == DRE2_RIGHT && next_nodes[j] == graph->count - 1 ) || ( direction == DRE2_LEFT && next_nodes[j] == 0 ) )
          {
            complete = true;
            break;
          }
        }
        if ( complete)
          break;
      }
    }

    if ( complete )
      return input;

    l_iter = iter;

    swapper = reachable;
    reachable = r_temp;
    r_temp = swapper;

    // Increment/Decrement input.
    if ( direction == DRE2_LEFT )
      *input--;
    else
      *input++;

    // Check if there's a \z.
    for ( i = 0; i < l_iter; i++ )
    {
      if ( graph->v[reachable[i]].c == DRE2_EOF )
      {
        if ( input - begin_ptr + 1 == length )
          return input;
      }
    }

    // If none of the active states matched, the regex failed to match.
    if ( matched == false )
    {
      return NULL;
    }
  }
  return NULL;
}

// Match when there is a single character and single node, horspool.
int
dre2_sn_sc_horspool( struct dre2 *graph, unsigned char *input, int length )
{
  int i;
  unsigned char c, *pch;

  c = graph->c;
  pch = memchr( input + graph->min_length - 1, c, length - graph->min_length );
  while ( pch != NULL && pch - input <= length && *pch != '\0' )
  {
    if ( dre2_matcher( graph, input, pch, graph->starting_point, DRE2_LEFT, length ) != NULL )
      return true;
    pch = memchr( pch + graph->skip_table[*pch], c, length - ( pch - input ) );
  }
  return false;
}

// Match when there is a single character and single node.
int
dre2_sn_sc( struct dre2 *graph, unsigned char *input, int length )
{
  unsigned char c, *pch;
  c = graph->c;
  pch = memchr( input, c, length );
  while ( pch != NULL && pch - input <= length && *pch != '\0' )
  {
    if ( graph->starting_point == 0 )
    {
      if ( dre2_matcher( graph, input, pch, graph->starting_point, DRE2_RIGHT, length ) != NULL )
        return true;
    } else
    {
      if ( dre2_matcher( graph, input, pch, graph->starting_point, DRE2_LEFT, length ) != NULL )
      {
        if ( dre2_matcher( graph, input, pch, graph->starting_point, DRE2_RIGHT, length ) != NULL )
          return true;
      }
    }
    pch = memchr( pch + 1, c, length - ( pch - input ) );
  }
  return false;
}

// Match when there is a single node but multiple characters, horspool.
int
dre2_sn_mc_horspool( struct dre2 *graph, unsigned char *input, int length )
{
  unsigned char *pch;
  pch = input + graph->min_length - 1;
  while ( pch - input <= length && *pch != '\0' && !graph->starting_chars[*pch] )
    *pch++;
  while ( pch - input <= length && *pch != '\0' )
  {
    if ( dre2_matcher( graph, input, pch, graph->count - 1, DRE2_LEFT, length ) != NULL )
      return true;
    pch = pch + graph->skip_table[*pch];
    while ( pch - input <= length && *pch != '\0' && !graph->starting_chars[*pch] )
      *pch++;
  }
  return false;
}

// Match when there is a single node but multiple characters.
int
dre2_sn_mc( struct dre2 *graph, unsigned char *input, int length )
{
  unsigned char *pch;

  pch = input;
  while ( pch - input <= length && *pch != '\0' && !graph->starting_chars[*pch] )
    *pch++;
  while ( pch - input <= length && *pch != '\0' )
  {
    if ( graph->starting_point == 0 )
    {
       if ( dre2_matcher( graph, input, pch, graph->starting_point, DRE2_RIGHT, length ) != NULL )
        return true;
    } else
    {
      if ( dre2_matcher( graph, input, pch, graph->starting_point, DRE2_LEFT, length ) != NULL )
      {
        if ( dre2_matcher( graph, input, pch, graph->starting_point, DRE2_RIGHT, length ) != NULL )
          return true;
      }
    }
    *pch++;
    while ( pch - input <= length && *pch != 0 && !graph->starting_chars[*pch] )
      *pch++;
  }
  return false;
}

// Match when there are multiple nodes.
int
dre2_mn( struct dre2 *graph, unsigned char *input, int length )
{
  int i;
  unsigned char *pch;
  pch = input;
  while ( pch - input <= length && *pch != '\0' && !graph->starting_chars[*pch] )
    *pch++;
  while ( pch - input <= length && *pch != '\0' )
  {
    for ( i = 0; i < graph->starting_count; i++ )
    {
      if ( dre2_matcher( graph, input, pch, graph->starting_points[i], DRE2_LEFT, length ) != NULL )
      {
        if ( dre2_matcher( graph, input, pch, graph->starting_points[i], DRE2_RIGHT, length ) != NULL )
          return true;
      }
    }
    *pch++;
    while ( pch - input <= length && *pch != 0 && !graph->starting_chars[*pch] )
      *pch++;
  }
  return false;
}

// Regex match driver.
int
dre2_match( struct dre2 *graph, unsigned char *input )
{
  int length = strlen( input );

  if ( length < graph->min_length )
    return false;

  switch ( graph->match_method )
  {
    case DRE2_SN_SC_H:
      return dre2_sn_sc_horspool( graph, input, length );
    case DRE2_SN_SC:
      return dre2_sn_sc( graph, input, length );
    case DRE2_SN_MC_H:
      return dre2_sn_mc_horspool( graph, input, length );
    case DRE2_SN_MC:
      return dre2_sn_mc( graph, input, length );
    case DRE2_MN:
      return dre2_mn( graph, input, length );
  }
}
