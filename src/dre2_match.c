#include "dre2.h"

// Match a single node.
int dre2_char_matches( struct dre2 *graph, struct dre2_node *node, unsigned char c )
{
  if ( node->c >= 0 )
  {
    if ( c == node->c )
      return true;
    if ( graph->options & DRE2_NO_CASE )
    {
      if ( node->c >= 'a' && node->c <= 'z' && c == node->c - ( 'a' - 'A' ) )
        return true;
      else if ( node->c >= 'A' && node->c <= 'Z' && c == node->c + ( 'a' - 'A' ) )
        return true;
    }
    return false;
  }

  if ( node->c == DRE2_CHAR_CLASS )
    return node->possible[c];

  switch( node->c )
  {
    case DRE2_DOT:
      if ( c != '\0' && c != '\n' && c != '\r' )
        return true;
      return false;
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
struct dre2_single_match dre2_matcher( struct dre2 *graph, unsigned char *begin_ptr, unsigned char *input, int start, int direction, int length, int *r_temp, int *reachable, int *state )
{
  int i, j, k;
  int *swapper, *next_nodes;
  int iter, l_iter;
  int matched;
  int r;
  int count;
  int found, complete, matched_bol, matched_eolf;
  struct dre2_node *node;
  struct dre2_single_match ret;

  ret.match = NULL;
  ret.active_states = 0;

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

  while ( input - begin_ptr <= length && input - begin_ptr >= 0 )
  {
    // Check for valid states.
    i = 0;
    iter = 0;
    matched = false;
    complete = false;
    matched_bol = false;
    matched_eolf = false;
    for ( i = 0; i < l_iter; i++ )
    {
      r = reachable[i];
      node = &graph->v[r];

      // Check if the character matches the node.
      if ( ( node->c == DRE2_BOL && input - begin_ptr == -1 ) ||
           ( node->c == DRE2_EOL && ( ( *input == '\n' && input - begin_ptr + 1 == length - 1 ) ) || *input == '\0' ) ||
           ( node->c == DRE2_EOF && ( ( *input == ' ' && input - begin_ptr + 1 == length - 1 ) || *input == '\0' ) ) ||
           ( dre2_char_matches( graph, node, *input ) ) )
      {
        if ( node->c == DRE2_BOL )
          matched_bol = true;
        else if ( node->c == DRE2_EOF || node->c == DRE2_EOL )
          matched_eolf = true;
        matched = true;

        // Set the next state to include this node's neighbors/parents.
        next_nodes = direction == DRE2_LEFT ? node->p : node->n;
        count = direction == DRE2_LEFT ? node->p_count : node->n_count;
        for ( j = 0; j < count; j++ )
        {
          if ( !state[next_nodes[j]] )
          {
            state[next_nodes[j]] = true;
            r_temp[iter++] = next_nodes[j];
            if ( ( direction == DRE2_RIGHT && next_nodes[j] == graph->count - 1 ) || ( direction == DRE2_LEFT && next_nodes[j] == 0 ) )
              complete = true;
          }
        }
      }
    }

    if ( complete )
    {
      ret.match = input;
      if ( !graph->options & DRE2_GREEDY )
      {
        for ( i = 0; i < iter; i++ )
          state[r_temp[i]] = false;
        return ret;
      }
    }

    // Reset the state lookup table.
    for ( i = 0; i < iter; i++ )
      state[r_temp[i]] = false;

    l_iter = iter;

    swapper = reachable;
    reachable = r_temp;
    r_temp = swapper;

    // Increment/Decrement input.
    if ( !matched_bol && !matched_eolf )
    {
      if ( direction == DRE2_LEFT )
        *input--;
      else
        *input++;
    }

    // Check if there's a \z.
    for ( i = 0; i < l_iter; i++ )
    {
      if ( graph->v[reachable[i]].c == DRE2_EOF || graph->v[reachable[i]].c == DRE2_EOL )
      {
        if ( input - begin_ptr + 1 == length )
        {
          if ( ( direction == DRE2_LEFT && dre2_contains_int( graph->v[reachable[i]].p, graph->v[reachable[i]].p_count, 0 ) ) || ( direction == DRE2_RIGHT && dre2_contains_int( graph->v[reachable[i]].n, graph->v[reachable[i]].n_count, graph->count - 1 ) ) )
          {
            ret.match = input + 1;
            goto SINGLE_MATCH_FINISH;
          }
        }
      } else if ( direction == DRE2_LEFT && graph->v[reachable[i]].c == DRE2_BOL )
      {
        if ( input - begin_ptr == -1 && dre2_contains_int( graph->v[reachable[i]].p,  graph->v[reachable[i]].p_count, 0 ) )
        {
          ret.match = input + 1;
          goto SINGLE_MATCH_FINISH;
        }
      }
    }

    // If none of the active states matched, the regex failed to match.
    if ( matched == false )
      goto SINGLE_MATCH_FINISH;
  }

  SINGLE_MATCH_FINISH:
  if ( graph->options & DRE2_GREEDY )
  {
    if ( ret.match != NULL )
    {
      ret.active_states = 0;
      for ( i = 0; i < l_iter; i++ )
      {
        node = &graph->v[reachable[i]];
        if ( ret.match - begin_ptr >= 0 && ( ( node->c == DRE2_BOL && input - begin_ptr == 0 ) || ( node->c == DRE2_EOL || node->c == DRE2_EOF && ( ( *input == ' ' && input - begin_ptr + 1 == length - 1 ) || *input == '\0' ) ) || dre2_char_matches( graph, node, *input ) ) )
          ret.active_states++;
      }
    }
  }

  return ret;
}

// Extract the matching portion of the regex.
void dre2_matched_substring( unsigned char *input, struct dre2_match_value *value, unsigned char **match )
{
  int size;

  size = value->end_pos - value->start_pos;
  if ( size <= 0 )
  {
    ( *match )[0] = '\0';
  } else
  {
    sprintf( *match, "%.*s", size, input + value->start_pos );
  }
}


// Match when there is a single character and single node, horspool.
struct dre2_match_value dre2_sn_sc_horspool( struct dre2 *graph, unsigned char *input, int length, int *r_temp, int *reachable, int *state )
{
  int i;
  unsigned char c, *pch;
  struct dre2_single_match result;
  struct dre2_match_value ret_val;

  c = graph->c;
  if ( graph->min_length > 0 )
    pch = memchr( input + graph->min_length - 1, c, length - graph->min_length + 1 );
  else
    pch = input;

  while ( pch != NULL && pch - input <= length && *pch != '\0' )
  {
    result = dre2_matcher( graph, input, pch, graph->starting_point, DRE2_LEFT, length, r_temp, reachable, state );
    if ( result.match != NULL )
    {
      ret_val.can_continue = result.active_states > 0 ? true : false;
      ret_val.matched = true;
      ret_val.start_pos = result.match - input;
      ret_val.end_pos = pch - input + 1;
      return ret_val;
    }
    pch = memchr( pch + graph->skip_table[*pch], c, length - ( pch - input ) );
  }
  ret_val.matched = false;
  return ret_val;
}

// Match when there is a single character and single node.
struct dre2_match_value dre2_sn_sc( struct dre2 *graph, unsigned char *input, int length, int *r_temp, int *reachable, int *state )
{
  unsigned char c, *pch;
  struct dre2_single_match result;
  struct dre2_match_value ret_val;

  c = graph->c;
  pch = memchr( input, c, length );
  while ( pch != NULL && pch - input <= length && *pch != '\0' )
  {
    if ( graph->starting_point == 0 )
    {
      result = dre2_matcher( graph, input, pch, graph->starting_point, DRE2_RIGHT, length, r_temp, reachable, state );
      if ( result.match != NULL )
      {
        ret_val.can_continue = result.active_states > 0 ? true : false;
        ret_val.matched = true;
        ret_val.start_pos = pch - input;
        ret_val.end_pos = result.match - input + 1;
        return ret_val;
      }
    } else
    {
      result = dre2_matcher( graph, input, pch, graph->starting_point, DRE2_LEFT, length, r_temp, reachable, state );
      if ( result.match != NULL )
      {
        ret_val.start_pos = result.match - input;
        ret_val.can_continue = result.active_states > 0 ? true : false;
        result = dre2_matcher( graph, input, pch, graph->starting_point, DRE2_RIGHT, length, r_temp, reachable, state );
        if ( result.match != NULL )
        {
          if ( !ret_val.can_continue )
            ret_val.can_continue = result.active_states > 0 ? true : false;
          ret_val.can_continue = result.active_states > 0 ? true : false;
          ret_val.matched = true;
          ret_val.end_pos = result.match - input + 1;
          return ret_val;
        }
      }
    }
    pch = memchr( pch + 1, c, length - ( pch - input ) );
  }
  ret_val.matched = false;
  return ret_val;
}

// Match when there is a single node but multiple characters, horspool.
struct dre2_match_value dre2_sn_mc_horspool( struct dre2 *graph, unsigned char *input, int length, int *r_temp, int *reachable, int *state )
{
  unsigned char *pch;
  struct dre2_single_match result;
  struct dre2_match_value ret_val;

  if ( graph->min_length > 0 )
    pch = input + graph->min_length - 1;
  else
    pch = input;
  while ( pch - input <= length && *pch != '\0' && !graph->starting_chars[*pch] )
    *pch++;
  while ( pch - input <= length && *pch != '\0' )
  {
    result = dre2_matcher( graph, input, pch, graph->count - 1, DRE2_LEFT, length, r_temp, reachable, state );
    if ( result.match != NULL )
    {
      ret_val.can_continue = result.active_states > 0 ? true : false;
      ret_val.matched = true;
      ret_val.start_pos = result.match - input;
      ret_val.end_pos = pch - input + 1;
      return ret_val;
    }
    pch = pch + graph->skip_table[*pch];
    while ( pch - input <= length && *pch != '\0' && !graph->starting_chars[*pch] )
      *pch++;
  }
  ret_val.matched = false;
  return ret_val;
}

// Match when there is a single node but multiple characters.
struct dre2_match_value dre2_sn_mc( struct dre2 *graph, unsigned char *input, int length, int *r_temp, int *reachable, int *state )
{
  unsigned char *pch;
  struct dre2_single_match result;
  struct dre2_match_value ret_val;

  pch = input;
  while ( pch - input <= length && *pch != '\0' && !graph->starting_chars[*pch] )
    *pch++;
  while ( pch - input <= length && *pch != '\0' )
  {
    if ( graph->starting_point == 0 )
    {
      result = dre2_matcher( graph, input, pch, graph->starting_point, DRE2_RIGHT, length, r_temp, reachable, state );
      if ( result.match != NULL )
      {
        ret_val.can_continue = result.active_states > 0 ? true : false;
        ret_val.matched = true;
        ret_val.start_pos = pch - input;
        ret_val.end_pos = result.match - input + 1;
        return ret_val;
      }
    } else
    {
      result = dre2_matcher( graph, input, pch, graph->starting_point, DRE2_LEFT, length, r_temp, reachable, state );
      if ( result.match != NULL )
      {
        ret_val.can_continue = result.active_states > 0 ? true : false;
        ret_val.start_pos = result.match - input;
        result = dre2_matcher( graph, input, pch, graph->starting_point, DRE2_RIGHT, length, r_temp, reachable, state );
        if ( result.match != NULL )
        {
          if ( !ret_val.can_continue )
            ret_val.can_continue = result.active_states > 0 ? true : false;
          ret_val.matched = true;
          ret_val.end_pos = result.match - input + 1;
          return ret_val;
        }
      }
    }
    *pch++;
    while ( pch - input <= length && *pch != 0 && !graph->starting_chars[*pch] )
      *pch++;
  }
  ret_val.matched = false;
  return ret_val;
}

// Match when there are multiple nodes.
struct dre2_match_value dre2_mn( struct dre2 *graph, unsigned char *input, int length, int *r_temp, int *reachable, int *state )
{
  int i;
  unsigned char *pch;
  struct dre2_single_match result;
  struct dre2_match_value ret_val;

  pch = input;
  while ( pch - input <= length && *pch != '\0' && !graph->starting_chars[*pch] )
    *pch++;
  while ( pch - input <= length && *pch != '\0' )
  {
    for ( i = 0; i < graph->starting_count; i++ )
    {
      result = dre2_matcher( graph, input, pch, graph->starting_points[i], DRE2_LEFT, length, r_temp, reachable, state );
      if ( result.match != NULL )
      {
        ret_val.can_continue = result.active_states > 0 ? true : false;
        ret_val.start_pos = result.match - input;
        result = dre2_matcher( graph, input, pch, graph->starting_points[i], DRE2_RIGHT, length, r_temp, reachable, state );
        if ( result.match != NULL )
        {
          if ( !ret_val.can_continue )
            ret_val.can_continue = result.active_states > 0 ? true : false;
          ret_val.can_continue = result.active_states > 0 ? true : false;
          ret_val.matched = true;
          ret_val.end_pos = result.match - input + 1; 
          return ret_val;
        }
      }
    }
    *pch++;
    while ( pch - input <= length && *pch != 0 && !graph->starting_chars[*pch] )
      *pch++;
  }
  ret_val.matched = false;
  return ret_val;
}

// Check if the string is a complete match.
struct dre2_match_value dre2_full_match( struct dre2 *graph, unsigned char *input, int length, int *r_temp, int *reachable, int *state )
{
  unsigned char *pch;
  struct dre2_single_match result;
  struct dre2_match_value ret_val;

  ret_val.start_pos = 0;
  ret_val.end_pos = length;
  ret_val.matched = false;
  pch = input;
  result = dre2_matcher( graph, input, pch, 0, DRE2_RIGHT, length, r_temp, reachable, state );
  if ( result.match != NULL )
  {
    if ( result.match - input == length || result.match - input == length + 1 )
      ret_val.matched = true;
    if ( result.match - input == length - 1 && ( input[length - 1] == '\n' || input[length - 1] == '\r' ) )
      ret_val.matched = true;
    if ( result.match - input == length - 2 && ( input[length - 2] == '\r' && input[length - 1] == '\n' ) )
      ret_val.matched = true;
    ret_val.can_continue = result.active_states > 0 ? true : false;
  }
  return ret_val;
}

// Do a search when the regex has a '^'.
struct dre2_match_value dre2_bol_match( struct dre2 *graph, unsigned char *input, int length, int *r_temp, int *reachable, int *state )
{
  unsigned char *pch;
  int *n_ptr;
  struct dre2_single_match result;
  struct dre2_match_value ret_val;
  int temp;

  ret_val.start_pos = 0;
  ret_val.matched = false;

  // Make the first node's neighbor list the anchored neighbors.
  n_ptr = graph->v[0].n;
  temp = graph->v[0].n_count;
  graph->v[0].n = graph->v[0].anchored;
  graph->v[0].n_count = graph->v[0].a_count;

  if ( graph->v[0].n == NULL )
    printf( "Neighbors are null!\n" );

  // Try the anchored match.
  pch = input;
  result = dre2_matcher( graph, input, pch, 0, DRE2_RIGHT, length, r_temp, reachable, state );
  if ( result.match != NULL )
  {
    ret_val.matched = true;
    ret_val.end_pos = result.match - input + 1;
    ret_val.can_continue = false;
    graph->v[0].n = n_ptr;
    graph->v[0].n_count = temp;
    return ret_val;
  }

  // Check if we can do the unanchored search.
  if ( !graph->all_anchored )
  {
    graph->v[0].n = graph->v[0].unanchored;
    graph->v[0].n_count = graph->v[0].u_count;

    switch ( graph->secondary_method )
    {
      case DRE2_SN_SC_H:
        ret_val = dre2_sn_sc_horspool( graph, input, length, r_temp, reachable, state );
        break;
      case DRE2_SN_SC:
        ret_val = dre2_sn_sc( graph, input, length, r_temp, reachable, state );
        break;
      case DRE2_SN_MC_H:
        ret_val = dre2_sn_mc_horspool( graph, input, length, r_temp, reachable, state );
        break;
      case DRE2_SN_MC:
        ret_val = dre2_sn_mc( graph, input, length, r_temp, reachable, state );
        break;
      case DRE2_MN:
        ret_val = dre2_mn( graph, input, length, r_temp, reachable, state );
        break;
    }
  }
  graph->v[0].n = n_ptr;
  graph->v[0].n_count = temp;
  return ret_val;
}

// Do a search when the regex has a '\z' or '$'.
struct dre2_match_value dre2_eolf_match( struct dre2 *graph, unsigned char *input, int length, int *r_temp, int *reachable, int *state )
{
  unsigned char *pch;
  int *p_ptr;
  struct dre2_single_match result;
  struct dre2_match_value ret_val;
  int temp;

  ret_val.matched = false;

  // Make the first node's neighbor list the anchored neighbors.
  temp = graph->v[graph->count - 1].p_count;
  p_ptr = graph->v[graph->count - 1].p;
  graph->v[graph->count - 1].p = graph->v[graph->count - 1].anchored;
  graph->v[graph->count - 1].p_count = graph->v[graph->count - 1].a_count;

  if ( graph->v[graph->count - 1].p == NULL )
    printf( "Parents are null!\n" );

  // Try the anchored match.
  pch = input + length - 1;

  // Remove the new lines and carriage returns at the end.
  while ( pch - input >= 0 && ( *pch == '\n' || *pch == '\r' || *pch == '\0' ) )
    *pch--;

  // If it's a \z, we allow one extra space at the end.
  if ( pch - input >= 0 && graph->match_method == DRE2_EOF_ANCHORED && *pch == ' ' )
    *pch--;

  result = dre2_matcher( graph, input, pch, graph->count - 1, DRE2_LEFT, length, r_temp, reachable, state );
  if ( result.match != NULL )
  {
    ret_val.matched = true;
    ret_val.start_pos = result.match - input;
    ret_val.end_pos = pch - input + 1;
    ret_val.can_continue = false;
    graph->v[graph->count - 1].p = p_ptr;
    graph->v[graph->count - 1].p_count = temp;
    return ret_val;
  }

  // Check if we can do the unanchored search.
  if ( !graph->all_anchored )
  {
    graph->v[graph->count - 1].p = graph->v[graph->count - 1].unanchored;
    graph->v[graph->count - 1].p_count = graph->v[graph->count - 1].u_count;

    switch ( graph->secondary_method )
    {
      case DRE2_SN_SC_H:
        ret_val = dre2_sn_sc_horspool( graph, input, length, r_temp, reachable, state );
        break;
      case DRE2_SN_SC:
        ret_val = dre2_sn_sc( graph, input, length, r_temp, reachable, state );
        break;
      case DRE2_SN_MC_H:
        ret_val = dre2_sn_mc_horspool( graph, input, length, r_temp, reachable, state );
        break;
      case DRE2_SN_MC:
        ret_val = dre2_sn_mc( graph, input, length, r_temp, reachable, state );
        break;
      case DRE2_MN:
        ret_val = dre2_mn( graph, input, length, r_temp, reachable, state );
        break;
    }
  }
  graph->v[graph->count - 1].p = p_ptr;
  graph->v[graph->count - 1].p_count = temp;
  return ret_val;
}

// Expand the original match for greedy mode.
struct dre2_match_value dre2_expanded_match( struct dre2 *graph, unsigned char *input, int length, int *r_temp, int *reachable, int *state, struct dre2_match_value *original_match )
{
  unsigned char *pch;
  struct dre2_single_match result;
  struct dre2_match_value ret_val;

  ret_val.start_pos = original_match->start_pos;
  ret_val.matched = true;
  pch = input + original_match->start_pos;
  result = dre2_matcher( graph, input + original_match->start_pos, pch, 0, DRE2_RIGHT, length, r_temp, reachable, state );
  if ( result.match != NULL )
    ret_val.end_pos = result.match - input + 1;
  return ret_val;
}

// Regex match driver.
struct dre2_match_value dre2_match( struct dre2 *graph, unsigned char *input )
{
  int i, length;
  struct dre2_match_value result;
  int *r_temp, *reachable, *state;

  result.matched = false;

  // Make sure the string is even long enough before attempting matching.
  length = strlen( input );

  if ( length < graph->min_length )
    return result;

  if ( graph->options & DRE2_THREAD_SAFE )
  {
    r_temp = ( int * )malloc( sizeof( int ) * graph->count );
    reachable = ( int * )malloc( sizeof( int ) * graph->count );
    state = ( int * )calloc( graph->count, sizeof( int ) );
  } else
  {
    r_temp = graph->r_temp;
    reachable = graph->reachable;
    state = graph->state;
  }

  if ( graph->options & DRE2_FULL_MATCH )
  {
    result = dre2_full_match( graph, input, length, r_temp, reachable, state );
  } else
  {
    switch ( graph->match_method )
    {
      case DRE2_SN_SC_H:
        result = dre2_sn_sc_horspool( graph, input, length, r_temp, reachable, state );
        break;
      case DRE2_SN_SC:
        result = dre2_sn_sc( graph, input, length, r_temp, reachable, state );
        break;
      case DRE2_SN_MC_H:
        result = dre2_sn_mc_horspool( graph, input, length, r_temp, reachable, state );
        break;
      case DRE2_SN_MC:
        result = dre2_sn_mc( graph, input, length, r_temp, reachable, state );
        break;
      case DRE2_MN:
        result = dre2_mn( graph, input, length, r_temp, reachable, state );
        break;
      case DRE2_BOL_ANCHORED:
        result = dre2_bol_match( graph, input, length, r_temp, reachable, state );
        break;
      case DRE2_EOF_ANCHORED:
      case DRE2_EOL_ANCHORED:
        result = dre2_eolf_match( graph, input, length, r_temp, reachable, state );
        break;
    }
  }

  if ( result.matched && ( graph->options & DRE2_GREEDY ) && result.can_continue )
    result = dre2_expanded_match( graph, input, length, r_temp, reachable, state, &result );

  if ( ( graph->options & DRE2_THREAD_SAFE ) || graph->count >= RANGE )
  {
    free( reachable ); reachable = NULL;
    free( r_temp ); r_temp = NULL;
    free( state ); state = NULL;
  }
  return result;
}
