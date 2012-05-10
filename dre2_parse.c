#include "dre2.h"

// Frequency analysis ranking.
const int dre2_frequency[RANGE] = {
   0,   1,  69, 136, 139, 140, 141, 142,
 143, 144, 241,   2,   3, 245,   8,  15,
  26,  37,  47,  58,  70,  81,  92, 103,
 114, 125, 132, 133, 134, 135, 137, 138,
 255, 206, 226, 152, 176, 154, 183, 224,
 192, 191, 175, 158, 233, 228, 231, 172,
 197, 196, 189, 186, 182, 187, 184, 181,
 190, 185, 201, 222, 167, 161, 168, 204,
 159, 223, 214, 218, 207, 216, 202, 203,
 221, 227, 198, 193, 210, 219, 213, 212,
 208, 177, 211, 220, 225, 195, 194, 217,
 188, 200, 174, 179, 149, 180, 164, 178,
 162, 252, 232, 238, 244, 254, 237, 236,
 247, 249, 209, 229, 243, 240, 250, 251,
 234, 205, 246, 248, 253, 242, 230, 239,
 215, 235, 199, 165, 171, 166, 145,   4,
 146,   5, 169, 157,   6, 150,   7, 163,
 151, 173, 170, 160, 153,   9,  10,  11,
 155, 147,  12, 156,  13,  14,  16,  17,
  18,  19,  20,  21,  22,  23,  24,  25,
  27,  28,  29,  30,  31,  32,  33,  34,
  35,  36,  38, 148,  39,  40,  41,  42,
  43,  44,  45,  46,  48,  49,  50,  51,
  52,  53,  54,  55,  56,  57,  59,  60,
  61,  62,  63,  64,  65,  66,  67,  68,
  71,  72,  73,  74,  75,  76,  77,  78,
  79,  80,  82,  83,  84,  85,  86,  87,
  88,  89,  90,  91,  93,  94,  95,  96,
  97,  98,  99, 100, 101, 102, 104, 105,
 106, 107, 108, 109, 110, 111, 112, 113,
 115, 116, 117, 118, 119, 120, 121, 122,
 123, 124, 126, 127, 128, 129, 130, 131
};

// Create an escaped string, optionally with skip-matching.
unsigned char *
dre2_escaped( unsigned char *re )
{
  unsigned char *buffer, *escaped, *ptr;
  int length;
  int skip_match;

  if ( *re == '!' )
    skip_match = false;
  else
    skip_match = true;

  length = strlen( re );
  if ( skip_match )
    length = length * 3 + 1;
  else
    length = length * 2 + 1;

  escaped = ( unsigned char * )calloc( length, sizeof( unsigned char ) );
  buffer = escaped;

  ptr = re;
  while ( *ptr )
  {
    switch ( *ptr )
    {
      case '(':
      case ')':
      case '\\':
      case '.':
      case '*':
      case '+':
      case '{':
      case '}':
      case '[':
      case ']':
      case '|':
        *buffer++ = '\\';
        *buffer++ = *ptr;
        break;
      case '?':
        *buffer++ = '.';
        break;
      default:
        *buffer++ = *ptr;
    }
    if ( skip_match )
    {
       *buffer++ = '.';
       *buffer++ = '?';
    }
    *ptr++;
  }
  *buffer = '\0';
  return escaped;
}

// Determine if a sorted array contains an item.
int
dre2_binsearch( int *values, int min, int max, int key )
{
  int mid;

  if ( max < min )
    return false;
  mid = ( min + max ) / 2;
  if ( values[mid] > key )
    return dre2_binsearch( values, min, mid - 1, key );
  else if ( values[mid] < key )
    return dre2_binsearch( values, mid + 1, max, key );
  else
    return true;
}

// Determine the largest element of an array.
int
dre2_largest( int *values, int length )
{
  int i, largest, index;
  largest = -1;
  for ( i = 0; i < length; i++ )
  {
    if ( largest < values[i] )
    {
      largest = values[i];
      index = i;
    }
  }
  return index;
}

// Determine if an array contains an item.
int
dre2_contains_int( int *values, int length, int key )
{
  int i;
  for ( i = 0; i < length; i++ )
  {
    if ( values[i] == key )
      return i + 1;
  }
  return false;
}

// Determine if a string contains a character.
int
dre2_contains_char( unsigned char *string, unsigned char c )
{
  unsigned char *pch;
  int length;

  length = strlen( string );
  pch = ( unsigned char * )memchr( string, c, length );
  if ( pch != NULL && pch - string <= length )
    return true;
  return false;
}

// Predefined character classes, e.g. \a, \d, \s, etc.
void
dre2_predefined_class( struct dre2_node *node, unsigned char *c, int action, int part_of_class )
{
  int k;
  if ( !part_of_class )
  {
    free( node->possible );
    node->possible = NULL;
  }

  if ( *c == 'a' )
  {
    // Match any alphabetic character.
    if ( part_of_class )
    {
      for ( k = 'a'; k <= 'z'; k++ )
        node->possible[k] = action;
    } else
    {
      node->c = DRE2_ALPHA;
    }
  } else if ( *c == 'b' )
  {
    // Non-alphanumeric or '-'
    if ( part_of_class )
    {
      for ( k = 'a'; k < 'z'; k++ )
        node->possible[k] = action == true ? false : true;
      for ( k = 'A'; k < 'Z'; k++ )
        node->possible[k] = action == true ? false : true;
      for ( k = '0'; k < '9'; k++ )
        node->possible[k] = action == true ? false : true;
      node->possible['-'] = action == true ? false : true;
    } else
    {
      node->c = DRE2_BORDER;
    }
  } else if ( *c == 'd' )
  {
    // Match any digit.
    if ( part_of_class )
    {
      for ( k = '0'; k <= '9'; k++ )
        node->possible[k] = action;
    } else
    {
      node->c = DRE2_DIGIT;
    }
  } else if ( *c == 'e' )
  {
    // Match any digit, including 8-bit obfuscations.
    if ( part_of_class )
    {
      for ( k = '0'; k <= '9'; k++ )
        node->possible[k] = action;
      node->possible[215] = action; //× "4" in CP1251 ("3" in ISO-8859-5)
      node->possible[225] = action; //á - (225/E1) - "6" in CP1251
      node->possible[226] = action; //â - (226/E2) - "6" in KOI8-R
      node->possible[231] = action; //ç - (231/E7) - "3" in CP1251 ("4" in ISO-8859-5)
      node->possible[238] = action; //î - (238/EE) - "0" in CP1251
      node->possible[239] = action; //ï - (239/EF) - "0" in KOI8-R
      node->possible[247] = action; //÷ - (247/F7) - "4" in CP1251
      node->possible[250] = action; //ú - (250/FA) - "3" in KOI8-R
      node->possible[254] = action; //þ - (254/FE) - "4" in KOI8-R
    } else
    {
      node->c = DRE2_SIM_DIGIT;
    }
  } else if ( *c == 'f' )
  {
    // Match any 8-bit obfuscations.
    if ( part_of_class )
    {
      node->possible[215] = action; //× "4" in CP1251 ("3" in ISO-8859-5)
      node->possible[225] = action; //á - (225/E1) - "6" in CP1251
      node->possible[226] = action; //â - (226/E2) - "6" in KOI8-R
      node->possible[231] = action; //ç - (231/E7) - "3" in CP1251 ("4" in ISO-8859-5)
      node->possible[238] = action; //î - (238/EE) - "0" in CP1251
      node->possible[239] = action; //ï - (239/EF) - "0" in KOI8-R
      node->possible[247] = action; //÷ - (247/F7) - "4" in CP1251
      node->possible[250] = action; //ú - (250/FA) - "3" in KOI8-R
      node->possible[254] = action; //þ - (254/FE) - "4" in KOI8-R
    } else
    {
      node->c = DRE2_OBF_DIGIT;
    }
  } else if ( *c == 'h' )
  {
    // Match any hex digit.
    if ( part_of_class )
    {
      for ( k = '0'; k <= '9'; k++ )
        node->possible[k] = action;
      for ( k = 'a'; k <= 'f'; k++ )
        node->possible[k] = action;
      for ( k = 'A'; k <= 'F'; k++ )
        node->possible[k] = action;
    } else
    {
      node->c = DRE2_HEX;
    }
  } else if ( *c == 'u' )
  {
    // Match any char allowed in URLs: <alphanum> . - / : _ @
    if ( part_of_class )
    {
      for ( k = '0'; k <= '9'; k++ )
        node->possible[k] = action;
      for ( k = 'a'; k <= 'z'; k++ )
        node->possible[k] = action;
      for ( k = 'A'; k <= 'Z'; k++ )
        node->possible[k] = action;
      node->possible['-'] = action;
      node->possible['.'] = action;
      node->possible['/'] = action;
      node->possible[':'] = action;
      node->possible['_'] = action;
      node->possible['@'] = action;
    } else
    {
      node->c = DRE2_URL;
    }
  } else if ( *c == 'v' )
  {
    // Match any character allowed in domains: <alphanum> . -
    if ( part_of_class )
    {
      for ( k = '0'; k <= '9'; k++ )
        node->possible[k] = action;
      for ( k = 'a'; k <= 'z'; k++ )
        node->possible[k] = action;
      for ( k = 'A'; k <= 'Z'; k++ )
        node->possible[k] = action;
      node->possible['.'] = action;
      node->possible['-'] = action;
    } else
    {
      node->c = DRE2_DOMAIN;
    }
  } else if ( *c == 'w' )
  {
    // Match any alphanumeric or '-'
    if ( part_of_class )
    {
      for ( k = '0'; k <= '9'; k++ )
        node->possible[k] = action;
      for ( k = 'a'; k <= 'z'; k++ )
        node->possible[k] = action;
      for ( k = 'A'; k <= 'Z'; k++ )
        node->possible[k] = action;
      node->possible['-'] = action;
    } else
    {
      node->c = DRE2_WORD;
    }
  } else if ( *c == 's' )
  {
    // Match a space or tab (or newline, carriage return, or form feed.
    if ( part_of_class )
    {
      node->possible['\t'] = true;
      node->possible['\n'] = true;
      node->possible['\r'] = true;
      node->possible['\f'] = true;
      node->possible[' '] = true;
    } else
    {
      node->c = DRE2_WHITE_SPACE;
    }
  } else if ( *c == 'z' )
  {
    // Match end of message.
    if ( !part_of_class )
      node->c = DRE2_EOF;
  } else if ( *c == '8' )
  {
    // Match any 8-bit char.
    if ( part_of_class )
    {
      for ( k = 128; k < RANGE; k++ )
        node->possible[k] = action;
    } else
    {
      node->c = DRE2_8BIT;
    }
  } else
  {
    // Match literation '\', '.', '?', '*', '+', etc.
    if ( part_of_class )
      node->possible[*c] = action;
    else
      node->c = *c;
  }
}

// Evaluate a character class.
int
dre2_character_class( struct dre2_node *node, unsigned char *re, int s )
{
  int length = strlen(re);
  int i = s;
  int action;
  int k;

  if ( re[i] == ']' )
  {
    printf( "No content in charcter class.\n" );
    return false;
  } else if ( re[i] == '^' )
  {
    i++;
    action = false;
  } else
  {
    action = true;
  }

  for ( k = 0; k < RANGE; k++ )
    node->possible[k] = action == true ? false : true;

  unsigned char c;
  unsigned char last = '\0';
  while ( i < length )
  {
    c = re[i];
    if ( c == '\\' )
    {
      i++;
      if ( i == length )
      {
        printf( "Invalid character class.\n" );
        return false;
      }
      c = re[i];
      dre2_predefined_class( node, &c, action, true );
      last = '\0';
      i++;
      continue;
    } else if ( c == '-' )
    {
      i++;
      if ( i == length )
      {
        printf( "Invalid character class.\n" );
        return false;
      }
      c = re[i];
      if ( c == ']' )
      {
        node->possible['-'] = action;
        return i;
      } else
      {
        if ( !last || last > c )
        {
          printf( "Invalid range in character class.\n" );
          return false;
        }
        for ( k = last; k <= c; k++ )
          node->possible[k] = action;
      }
      i++;
      continue;
    } else if ( c == ']' )
    {
      return i;
    } else
    {
      node->possible[c] = action;
    }
    i++;
    last = c;
  }
  return false;
}

// Convert a string into an integer.
int
string_to_int( unsigned char *s )
{
  // Number of digits in the string.
  int d = strlen( s );

  // Multiplier.
  int m = 1;

  // Iterator.
  int i;

  // Check for null
  if ( !s || !*s ) {
    return 0;
  }

  // Check for infinite
  if ( s[0] == '+' )
  {
    return DRE2_INFINITE;
  }

  // Find out the max multiplier.
  for ( i = 0; i < d; i++ )
    m *= 10;
  m /= 10;

  // Convert it to an int.
  int ret = 0;
  for ( i = 0; i < d; i++ )
  {
    ret += ( s[i] - '0' ) * m;
    m /= 10;
  }
  return ret;
}


// Exact size or size range, e.g. {0,4}, {2,}, {3}
struct dre2_range_return
dre2_range( unsigned char *re, int length, int pos )
{
  unsigned char *min_buffer, *max_buffer;
  min_buffer = ( unsigned char * )calloc( 5, sizeof( unsigned char ) );
  max_buffer = ( unsigned char * )calloc( 5, sizeof( unsigned char ) );

  pos++;

  int t = pos;

  // Is this modifier valid?
  int valid = true;
  // Are we still working on the first part of the range?
  int w_min = true;
  // Number of characters in the min string.
  int cmin = 0;
  // Number of characters in the max string.
  int cmax = 0;

  unsigned char c;
  while ( pos < length )
  {
    c = re[pos];
    if ( c == ',' )
    {
      if ( t == pos )
      {
        valid = false;
        break;
      }
      if ( !w_min )
      {
        valid = false;
        break;
      }
      w_min = false;
    } else if ( c == '}' )
    {
      break;
    } else if ( c < '0' || c > '9' )
    {
      valid = false;
      break;
    } else
    {
      int j;
      if ( w_min )
      {
        cmin++;
        min_buffer[cmin - 1] = c;
      } else
      {
        cmax++;
        max_buffer[cmax - 1] = c;
      }
    }
    pos++;
  }
  if ( !w_min )
  {
    // See if it's unbounded on the right, e.g. {1,}
    if ( cmax == false )
    {
      max_buffer[0] = '+';
    }
  } else
  {
    // See if it's an exact size, e.g. {3}
    strcat( max_buffer, min_buffer );
  }

  // Setup the return structure.
  struct dre2_range_return ret;
  ret.min = DRE2_INFINITE;
  ret.max = DRE2_INFINITE;
  ret.pos = DRE2_INFINITE;
  ret.valid = false;

  // Make sure it's valid.
  if ( !valid )
  {
    printf( "Invalid range {}\n" );
    return ret;
  }

  // Convert strings to integers.
  int min = string_to_int( min_buffer );
  int max = string_to_int( max_buffer );

  // Make sure it's a valid size range.
  if ( min > max && max != DRE2_INFINITE )
  {
    printf("Invalid range, min is greater than max!\n");
    return ret;
  }
  if ( min > 50 || max > 50 )
  {
    printf( "Way too high of a range, use + or *.\n" );
    return ret;
  }

  // Free buffers.
  free( min_buffer );
  free( max_buffer );
  min_buffer = NULL;
  max_buffer = NULL;

  // Setup the return structure and return it.
  ret.min = min;
  ret.max = max;
  ret.pos = pos;
  ret.valid = true;
  return ret;
}

// Clean up memory, etc.
void
cleanup_nodes( struct dre2_node **v, int node_count )
{
  int i;
  if ( *v != NULL )
  {
    for ( i = 0; i < node_count; i++ )
    {
      if ( v[0][i].n != NULL )
      {
        free( v[0][i].n );
        v[0][i].n = NULL;
      }
      if ( v[0][i].p != NULL )
      {
        free( v[0][i].p );
        v[0][i].p = NULL;
      }
      if ( v[0][i].c == DRE2_CHAR_CLASS )
      {
        free( v[0][i].possible );
        v[0][i].possible = NULL;
      }
      if ( v[0][i].min_n != NULL )
      {
        free( v[0][i].min_n );
        v[0][i].min_n = NULL;
      }
    }
    free( *v );
    *v = NULL;
  }
}

// Clean up memory, etc.
void
cleanup_dre2( struct dre2 *graph )
{
  cleanup_nodes( &graph->v, graph->count );
  free( graph->v );
  if ( graph->reachable != NULL ) { free( graph->reachable ); graph->reachable = NULL; }
  if ( graph->r_temp != NULL ) { free( graph->r_temp ); graph->r_temp = NULL; }
  if ( graph->state != NULL ) { free( graph->state ); graph->state = NULL; }
  if ( graph->starting_points != NULL ) { free( graph->starting_points ); graph->starting_points = NULL; }
  graph->v = NULL;
}

void
dre2_find_paths_recursive( struct dre2 *graph, int id, int *path_count, struct dre2_path **paths )
{
  int i, j;
  int size, last;
  struct dre2_path *path, *next_path;

  size = paths[0][id].count;
  last = paths[0][id].nodes[paths[0][id].count - 1];

  for ( i = 0; i < graph->v[last].min_n_count; i++ )
  {
    path = &paths[0][id];
    if ( i == 0 )
    {
      // For the first possible node, just add it to the current path.
      paths[0][id].count++;
      paths[0][id].nodes[paths[0][id].count - 1] = graph->v[last].min_n[i];
      dre2_find_paths_recursive( graph, id, path_count, paths );
    } else
    {
      // For the other nodes, split the path.
      *path_count = *path_count + 1;
      *paths = ( struct dre2_path * )realloc( *paths, sizeof( struct dre2_path ) * *path_count );
      next_path = &paths[0][*path_count - 1];
      next_path->count = size + 1;
      next_path->nodes = ( int * )malloc( sizeof( int ) * graph->count );

      // Copy the previous nodes and add the current one.
      for ( j = 0; j < size; j++ )
        next_path->nodes[j] = paths[0][id].nodes[j];
      next_path->nodes[size] = graph->v[last].min_n[i];
      dre2_find_paths_recursive( graph, *path_count - 1, path_count, paths );
    }
  }
}

// Frequency/cost of a single node.
int
dre2_node_cost( struct dre2 *graph, int id )
{
  int i, j;
  int cost;
  struct dre2_node *node;

  node = &graph->v[id];

  cost = 0;
  switch( node->c )
  {
    case DRE2_ALPHA:
      for ( i = 'a'; i <= 'z'; i++ )
        cost += dre2_frequency[i];
      for ( i = 'A'; i <= 'Z'; i++ )
        cost += dre2_frequency[i];
      return cost;
    case DRE2_WORD:
      for ( i = 'a'; i <= 'z'; i++ )
        cost += dre2_frequency[i];
      for ( i = 'A'; i <= 'Z'; i++ )
        cost += dre2_frequency[i];
      for ( i = '0'; i <= '9'; i++ )
        cost += dre2_frequency[i];
      cost += dre2_frequency['-'];
      return cost;
    case DRE2_DOMAIN:
      for ( i = 'a'; i <= 'z'; i++ ) 
        cost += dre2_frequency[i];
      for ( i = 'A'; i <= 'Z'; i++ )
        cost += dre2_frequency[i];
      for ( i = '0'; i <= '9'; i++ )
        cost += dre2_frequency[i];
      cost += dre2_frequency['-'] + dre2_frequency['.'];
      return cost;
    case DRE2_URL:
      for ( i = 'a'; i <= 'z'; i++ )
        cost += dre2_frequency[i];
      for ( i = 'A'; i <= 'Z'; i++ )
        cost += dre2_frequency[i];
      for ( i = '0'; i <= '9'; i++ )
        cost += dre2_frequency[i];
      cost += dre2_frequency['-'] + dre2_frequency['.'] + dre2_frequency['/'] + dre2_frequency[':'] + dre2_frequency['_'] + dre2_frequency['@'];
      return cost;
    case DRE2_DIGIT:
      for ( i = '0'; i <= '9'; i++ )
        cost += dre2_frequency[i];
      return cost;
    case DRE2_BORDER:
      for ( i = 0; i < RANGE; i++ )
      {
        if ( ( i >= 'a' && i <= 'z' ) || ( i >= 'A' && i <= 'Z' ) || ( i >= '0' && i <= '9' ) || ( i == '-' ) )
          continue;
        cost += dre2_frequency[i];
      }
      return cost;
    case DRE2_WHITE_SPACE:
      cost = dre2_frequency[' '] + dre2_frequency['\t'] + dre2_frequency['\r'] + dre2_frequency['\f'] + dre2_frequency['\n'];
      return cost;
    case DRE2_CHAR_CLASS:
      for ( i = 0; i < RANGE; i++ )
      {
        if ( node->possible[i] )
          cost += dre2_frequency[i];
      }
      return cost;
    case DRE2_SIM_DIGIT:
      for ( i = '0'; i <= '9'; i++ )
        cost += dre2_frequency[i];
      cost += dre2_frequency[215] + dre2_frequency[225] + dre2_frequency[226] + dre2_frequency[231] + dre2_frequency[238] + dre2_frequency[239] + dre2_frequency[247] + dre2_frequency[250] + dre2_frequency[254];
      return cost;
    case DRE2_OBF_DIGIT:
      cost = dre2_frequency[215] + dre2_frequency[225] + dre2_frequency[226] + dre2_frequency[231] + dre2_frequency[238] + dre2_frequency[239] + dre2_frequency[247] + dre2_frequency[250] + dre2_frequency[254];
      return cost;
    case DRE2_HEX:
      for ( i = 'a'; i <= 'f'; i++ )
        cost += dre2_frequency[i];
      for ( i = 'A'; i <= 'F'; i++ )
        cost += dre2_frequency[i];
      for ( i = '0'; i <= '9'; i++ )
        cost += dre2_frequency[i];
      return cost;
    default:
      return dre2_frequency[node->c];
  }
}

// Pick the best node that is required.
int
dre2_best_choice( struct dre2 *graph, int *required, int count )
{
  int i, j;
  int temp;
  int *ids, *cost;
  int best;

  ids = ( int * )malloc( sizeof( int ) * count );
  cost = ( int * )malloc( sizeof( int ) * count );
  for ( i = 0; i < count; i++ )
  {
    ids[i] = required[i];
    cost[i] = required[i] == 0 || required[i] == graph->count - 1 ? 100000 : dre2_node_cost( graph, required[i] );
  }

  for ( i = 0; i < count - 1; i++ )
  {
    for ( j = i + 1; j < count; j++ )
    {
      if ( cost[i] > cost[j] || ( cost[i] == cost[j] && ids[i] < ids[j] ) )
      {
        temp = cost[i];
        cost[i] = cost[j];
        cost[j] = temp;
        temp = ids[i];
        ids[i] = ids[j];
        ids[j] = temp;
      }
    }
  }
  best = ids[0];

  free( cost );
  free( ids );
  cost = NULL;
  ids = NULL;

  return best;
}

// Set starting chars for multiple nodes.
void
dre2_set_chars( struct dre2 *graph, int id )
{
  int i, iter;
  struct dre2_node *node;

  node = &graph->v[id];
  if ( node->c == DRE2_ALPHA || node->c == DRE2_WORD || node->c == DRE2_DOMAIN || node->c == DRE2_URL )
  {
    for ( i = 'a'; i <= 'z'; i++ )
      graph->starting_chars[i] = true;
    for ( i = 'A'; i <= 'Z'; i++ )
      graph->starting_chars[i] = i;
  }
  if ( node->c == DRE2_WORD || node->c == DRE2_DOMAIN || node->c == DRE2_URL || node->c == DRE2_DIGIT || node->c == DRE2_SIM_DIGIT || node->c == DRE2_HEX )
  {
    for ( i = '0'; i <= '9'; i++ )
      graph->starting_chars[i] = true;
  }
  if ( node->c == DRE2_WORD )
    graph->starting_chars['-'] = true;
  if ( node->c == DRE2_DOMAIN )
  {
    graph->starting_chars['-'] = true;
    graph->starting_chars['.'] = true;
  }
  if ( node->c == DRE2_URL )
  {
    graph->starting_chars['-'] = true;
    graph->starting_chars['.'] = true;
    graph->starting_chars['/'] = true;
    graph->starting_chars[':'] = true;
    graph->starting_chars['_'] = true;
    graph->starting_chars['@'] = true;
  }
  if ( node->c == DRE2_BORDER )
  {
    for ( i = 0; i < RANGE; i++ )
    {
      if ( ( i >= 'a' && i <= 'z' ) || ( i >= 'A' && i <= 'Z' ) || ( i >= '0' && i <= '9' ) || ( i == '-' ) )
        continue;
      graph->starting_chars[i] = true;
    }
  }
  if ( node->c == DRE2_WHITE_SPACE )
  {
    graph->starting_chars[' '] = true;
    graph->starting_chars['\t'] = true;
    graph->starting_chars['\r'] = true;
    graph->starting_chars['\f'] = true;
    graph->starting_chars['\n'] = true;
  }
  if ( node->c == DRE2_CHAR_CLASS )
  {
    for ( i = 0; i < RANGE; i++ )
    {
      if ( node->possible[i] )
        graph->starting_chars[i] = true;
    }
  }
  if ( node->c == DRE2_SIM_DIGIT || node->c == DRE2_OBF_DIGIT )
  {
    graph->starting_chars[215] = true;
    graph->starting_chars[225] = true;
    graph->starting_chars[226] = true;
    graph->starting_chars[231] = true;
    graph->starting_chars[238] = true;
    graph->starting_chars[239] = true;
    graph->starting_chars[247] = true;
    graph->starting_chars[250] = true;
    graph->starting_chars[254] = true;
  }
  if ( node->c == DRE2_HEX )
  {
    for ( i = 'a'; i <= 'f'; i++ )
      graph->starting_chars[i] = true;
    for ( i = 'A'; i <= 'F'; i++ )
      graph->starting_chars[i] = true;
  }
  if ( node->c >= 0 )
    graph->starting_chars[node->c] = true;
}

// Set the starting chars.
void
dre2_starting_chars( struct dre2 *graph, int *minimal )
{
  int i, count;
  struct dre2_node *node;

  for ( i = 0; i < RANGE; i++ )
  {
    graph->starting_chars[i] = false;
  }

  if ( graph->starting_point == -1 )
  {
    // Using multiple nodes.
    for ( i = 0; i < graph->starting_count; i++ )
      dre2_set_chars( graph, graph->starting_points[i] );
  } else if ( graph->starting_point != 0 && graph->starting_point != graph->count - 1 )
  {
    // Only a single node, set the chars for that node.
    dre2_set_chars( graph, graph->starting_point );
  } else if ( graph->starting_point == 0 )
  {
    // Set all of the chars possible from the reachable nodes of the first node.
    for ( i = 0; i < graph->v[0].n_count; i++ )
    {
      if ( minimal[graph->v[0].n[i]] )
        dre2_set_chars( graph, graph->v[0].n[i] );
    }
  } else
  {
    // Set all of the chars possible from the parents of the last node.
    for ( i = 0; i < graph->v[graph->count - 1].p_count; i++ )
    {
      if ( minimal[graph->v[graph->count - 1].p[i]] )
        dre2_set_chars( graph, graph->v[graph->count - 1].p[i] );
    }
  }

  // Check if the there is a single starting char or multiple (determines matching method)
  count = 0;
  for ( i = 0; i < RANGE; i++ )
  {
    if ( graph->starting_chars[i] )
      count++;
    if ( count > 1 )
      break;
  }
  if ( count > 1 )
    graph->single = false;
  else
    graph->single = true;
}

// Find the best starting node in the graph.
int
dre2_starting_point( struct dre2 *graph, int *minimal, int *minimal_id, int minimal_count )
{
  int i, j, k, l, m;
  int min_iter, iter;
  int id, all, found, temp;
  int *required;
  int required_count;
  int best;
  int **found_in, *found_in_count, *found_in_id;
  int *swapper;
  int *remaining;
  int largest, last_largest;
  struct dre2_path *paths;
  int path_count;
  struct dre2_fl_cost cost;

  // First, check if the first or last node is acceptable.
  cost = dre2_first_or_last_cost( graph, minimal );

  // Initialize paths.
  path_count = 1;
  paths = ( struct dre2_path * )malloc( sizeof( struct dre2_path ) );
  paths[0].nodes = ( int * )malloc( sizeof( int ) * graph->count );
  paths[0].count = 1;
  paths[0].nodes[0] = 0;

  graph->starting_points = ( int * )malloc( sizeof( int ) * graph->count );
  graph->starting_count = 0;

  // Recursively find paths through the minimal graph.
  dre2_find_paths_recursive( graph, 0, &path_count, &paths );

  for ( i = 0; i < path_count; i++ )
  {
    //printf( "Path %d: ", i );
    //for ( j = 0; j < paths[i].count; j++ )
    //  printf( "%d -> ", paths[i].nodes[j] );
    //printf( "\n" );
    best = dre2_best_choice( graph, paths[i].nodes, paths[i].count );
    //printf( "Best node: %d\n", best );
    if ( !dre2_contains_int( graph->starting_points, graph->starting_count, best ) )
      graph->starting_points[graph->starting_count++] = best;
  }

  // Allocate some memory.
  found_in = ( int ** )malloc( sizeof( int * ) * ( minimal_count - 2 ) );
  found_in_count = ( int * )malloc( sizeof( int ) * ( minimal_count - 2 ) );
  found_in_id = ( int * )malloc( sizeof( int ) * ( minimal_count - 2 ) );
  remaining = ( int * )malloc( sizeof( int ) * ( minimal_count - 2 ) );

  for ( i = 0; i < minimal_count - 2; i++ )
  {
    found_in_count[i] = 0;
    found_in[i] = ( int * )malloc( sizeof( int ) * path_count );
    found_in_id[i] = minimal_id[i + 1];
  }

  best = -1;
  if ( path_count == 1 && paths[0].count > 2 )
  {
    required_count = paths[0].count - 2;
    required = ( int * )malloc( sizeof( int ) * required_count );
    for ( i = 1; i < paths[0].count -1; i++ )
      required[i - 1] = paths[0].nodes[i];
    best = dre2_best_choice( graph, required, required_count );
    goto INITIAL_CLEANUP;
  } else if ( path_count > 1 )
  {
    required_count = 0;
    required = ( int * )malloc( sizeof( int ) * graph->count );
    for ( i = 1; i < minimal_count - 1; i++ )
    {
      all = true;
      id = minimal_id[i];
      for ( j = 0; j < path_count; j++ )
      {
        if ( !dre2_binsearch( paths[j].nodes, 0, paths[j].count - 1, id ) )
        {
          all = false;
        } else
        {
          found_in_count[i - 1]++;
          found_in[i - 1][found_in_count[i - 1] - 1] = j;
        }
      }
      if ( all )
        required[required_count++] = id;
    }
    if ( required_count > 0 )
    {
      best = dre2_best_choice( graph, required, required_count );
      goto INITIAL_CLEANUP;
    }
  }

  best = dre2_first_or_last( graph, &cost );
  INITIAL_CLEANUP:
  if ( path_count > 1 && dre2_use_paths( graph, best, &cost ) )
    best = -1;

  free( required ); required = NULL;
  for ( i = 0; i < path_count; i++ )
  {
    free( paths[i].nodes );
    paths[i].nodes = NULL;
  }
  for ( i = 0; i < minimal_count - 2; i++ )
  {
    free( found_in[i] ); found_in[i] = NULL;
  }
  free( found_in ); found_in = NULL;
  free( found_in_count ); found_in_count = NULL;
  free( found_in_id ); found_in_id = NULL;
  free( remaining ); remaining = NULL;
  free( paths ); paths = NULL;

  return best;
}

// Find all of the reachable nodes from a node excluded backlinks.
void
dre2_min_reachable( struct dre2 *graph, int **reachable, int **visited, int id )
{
  if ( visited[0][id] == true )
    return;
  visited[0][id] = true;

  int i;
  for ( i = 0; i < graph->v[id].n_count; i++ )
  {
    if ( id < graph->v[id].n[i] )
    {
      struct dre2_node *neighbor = &graph->v[graph->v[id].n[i]];
      if ( neighbor->c == DRE2_GROUP_OPEN || neighbor->c == DRE2_GROUP_CLOSE )
      {
        if ( graph->v[id].n[i] == graph->count - 1 )
          reachable[0][graph->v[id].n[i]] = true;
        dre2_min_reachable( graph, reachable, visited, graph->v[id].n[i] );
      } else
      {
        reachable[0][graph->v[id].n[i]] = true;
      }
    }
  }
}

// Find all of the reachable nodes from a node.
void
dre2_reachable( struct dre2 *graph, int **reachable, int **visited, int id )
{
  if ( visited[0][id] == true )
    return;
  visited[0][id] = true;

  int i;
  for ( i = 0; i < graph->v[id].n_count; i++ )
  {
    struct dre2_node *neighbor = &graph->v[graph->v[id].n[i]];

    // If it's a group node, check neighbors.
    if ( neighbor->c == DRE2_GROUP_OPEN || neighbor->c == DRE2_GROUP_CLOSE )
    {
      if ( graph->v[id].n[i] == graph->count - 1 )
        reachable[0][graph->v[id].n[i]] = true;
      dre2_reachable( graph, reachable, visited, graph->v[id].n[i] );
    } else
    {
      reachable[0][graph->v[id].n[i]] = true;
    }
  }
}

// Strip out the group open and close nodes.
void
dre2_strip_groups( struct dre2 *graph, struct dre2 *new_graph, struct dre2_node **new_nodes, int *minimal, int **new_minimal, int *new_minimal_count, int **minimal_id )
{
  int i, j, k;
  struct dre2_node *nodes, *node;
  int node_count;
  int removed, offset;
  int *reachable, *visited, *temp_minimal;
  int *min_reachable, *min_visited;

  // Allocate some memory.
  reachable = ( int * )malloc( sizeof( int ) * graph->count );
  visited = ( int * )malloc( sizeof( int ) * graph->count );
  temp_minimal = ( int * )malloc( sizeof( int ) );
  min_reachable = ( int * )malloc( sizeof( int ) * graph->count );
  min_visited = ( int * )malloc( sizeof( int ) * graph->count );

  for ( i = 0; i < graph->count; i++ )
    new_minimal[0][i] = false;

  node_count = 0;
  *new_minimal_count = 0;
  for ( i = 0; i < graph->count; i++ )
  {
    // Initialize reachable nodes to all false.
    for ( j = 0; j < graph->count; j++ )
    {
      reachable[j] = false;
      visited[j] = false;
      min_reachable[j] = false;
      min_visited[j] = false;
    }
    dre2_reachable( graph, &reachable, &visited, i );
    dre2_min_reachable( graph, &min_reachable, &min_visited, i );

    if ( ( graph->v[i].c != DRE2_GROUP_OPEN && graph->v[i].c != DRE2_GROUP_CLOSE ) || ( i == 0 || i == graph->count - 1 ) )
    {
      // Add node to our min tree.
      dre2_add_node( new_nodes, &node_count, graph->v[i].c, &temp_minimal );

      new_nodes[0][node_count - 1].min_n_count = 0;
      new_nodes[0][node_count - 1].min_n = NULL;
      if ( minimal[i] )
      {
        new_minimal[0][node_count - 1] = true;
        *new_minimal_count = *new_minimal_count + 1;
        minimal_id[0][*new_minimal_count - 1] = node_count - 1;
      }
      graph->v[i].min_id = node_count - 1;
      if ( graph->v[i].c == DRE2_CHAR_CLASS )
      {
        for ( j = 0; j < RANGE; j++ )
          new_nodes[0][node_count - 1].possible[j] = graph->v[i].possible[j];
      }

      // Find the reachable nodes.
      for ( j = 0; j < graph->count; j++ )
      {
        if ( reachable[j] )
          dre2_add_neighbor( new_nodes, node_count - 1, j );
        if ( min_reachable[j] && minimal[j] )
        {
          new_nodes[0][node_count - 1].min_n_count++;
          if ( new_nodes[0][node_count - 1].min_n == NULL )
            new_nodes[0][node_count - 1].min_n = ( int * )malloc( sizeof( int ) * graph->count );
          new_nodes[0][node_count - 1].min_n[new_nodes[0][node_count - 1].min_n_count - 1] = j;
        }
      }
    }
  }

  // Update the node IDs.
  for ( i = 0; i < node_count; i++ )
  {
    for ( j = 0; j < new_nodes[0][i].n_count; j++ )
      new_nodes[0][i].n[j] = graph->v[new_nodes[0][i].n[j]].min_id;
  }

  for ( i = 0; i < *new_minimal_count; i++ )
  {
    for ( j = 0; j < new_nodes[0][minimal_id[0][i]].min_n_count; j++ )
      new_nodes[0][minimal_id[0][i]].min_n[j] = graph->v[new_nodes[0][minimal_id[0][i]].min_n[j]].min_id;
  }

  // Setup the minimized graph.
  new_graph->count = node_count;

  // Free the temp reachable nodes array.
  free( reachable );
  free( visited );
  free( temp_minimal );
  free( min_reachable );
  free( min_visited );

  // Set them all to null.
  reachable = NULL;
  visited = NULL;
  temp_minimal = NULL;
  min_reachable = NULL;
  min_visited = NULL;
}

// Find out the cost of a single node.
struct dre2_cost
dre2_single_cost( struct dre2 *graph, int id )
{
  int i, j;
  int c_count, frequency;
  unsigned char *temp;
  int *tp, *original;
  struct dre2_cost cost;

  temp = ( unsigned char * )calloc( RANGE, sizeof( unsigned char ) );
  tp = ( int * )calloc( RANGE, sizeof( int ) );

  original = graph->starting_chars;
  graph->starting_chars = tp;

  for ( i = 0; i < RANGE; i++ )
    tp[i] = false;
  graph->starting_chars = tp;

  dre2_set_chars( graph, id );

  c_count = 0;
  frequency = 0;
  for ( j = 1; j < RANGE; j++ )
  {
    if ( graph->starting_chars[j] && !dre2_contains_char( temp, j ) )
    {
      c_count++;
      temp[c_count - 1] = j;
      frequency += dre2_frequency[j];
    }
  }
  free( temp ); temp = NULL;
  free( tp ); tp = NULL;
  graph->starting_chars = original;
  cost.c_count = c_count;
  cost.frequency = frequency;
  return cost;
}

// Find out the cost of the first node or last node.
struct dre2_fl_cost
dre2_first_or_last_cost( struct dre2 *graph, int *minimal )
{
  int i, j, k;
  int f_n_count, f_c_count, f_frequency;
  int l_n_count, l_c_count, l_frequency;
  int next_count, *next_ptr;
  struct dre2_node *node;
  struct dre2_fl_cost cost;
  unsigned char *temp;
  int *tp, *original;

  f_n_count = graph->v[0].n_count;
  f_c_count = 0;
  f_frequency = 0;
  l_n_count = graph->v[graph->count - 1].p_count;
  l_c_count = 0;
  l_frequency = 0;

  for ( i = 0; i < graph->v[0].n_count; i++ )
  {
    if ( !minimal[graph->v[0].n[i]] )
      f_n_count--;
  }
  for ( i = 0; i < graph->v[graph->count - 1].p_count; i++ )
  {
    if ( !minimal[graph->v[graph->count - 1].p[i]] )
      l_n_count--;
  }

  // Get number of possible chars in the first reachable nodes.
  tp = ( int * )malloc( sizeof( int ) * RANGE );
  temp = ( unsigned char * )malloc( sizeof( unsigned char ) * RANGE );

  original = graph->starting_chars;
  graph->starting_chars = tp;

  for ( k = 0; k < 2; k++ )
  {
    for ( i = 0; i < RANGE; i++ )
      tp[i] = '\0';
    graph->starting_chars = tp;
    if ( k == 0 )
    {
      next_count = graph->v[0].n_count;
      next_ptr = graph->v[0].n;
    } else
    {
      next_count = graph->v[graph->count - 1].p_count;
      next_ptr = graph->v[graph->count - 1].p;
    }
    for ( i = 0; i < next_count; i++ )
    {
      if ( !minimal[next_ptr[i]] )
        continue;
      dre2_set_chars( graph, next_ptr[i] );
      for ( j = 1; j < RANGE; j++ )
      {
        if ( graph->starting_chars[j] && !dre2_contains_char( temp, j ) )
        {
          if ( k == 0 )
          {
            f_c_count++;
            temp[f_c_count - 1] = j;
            f_frequency += dre2_frequency[j];
          } else
          {
            l_c_count++;
            temp[l_c_count - 1] = j;
            l_frequency += dre2_frequency[j];
          }
        }
      }
    }
  }
  free( temp ); temp = NULL;
  free( tp ); tp = NULL;
  graph->starting_chars = original;

  cost.f_n_count = f_n_count;
  cost.f_c_count = f_c_count;
  cost.f_frequency = f_frequency;
  cost.l_n_count = l_n_count;
  cost.l_c_count = l_c_count;
  cost.l_frequency = l_frequency;
  return cost;
}

// See if it might be better to start somewhere in the middle of the graph.
int
dre2_use_paths( struct dre2 *graph, int best, struct dre2_fl_cost *cost )
{
  int i, j;
  struct dre2_node *node;
  int b_n_count, b_c_count, b_frequency;
  int p_n_count, p_c_count, p_frequency;
  int *original, *tp;
  double bf, pf, diff;
  unsigned char *temp;
  struct dre2_cost node_cost;

  if ( best == 0 )
  {
    b_n_count = cost->f_n_count;
    b_c_count = cost->f_c_count;
    b_frequency = cost->f_frequency;
  } else if ( best == graph->count - 1 )
  {
    b_n_count = cost->l_n_count;
    b_c_count = cost->l_c_count;
    b_frequency = cost->l_frequency;
  } else
  {
    node_cost = dre2_single_cost( graph, best );
    b_n_count = 1;
    b_c_count = node_cost.c_count;
    b_frequency = node_cost.frequency;
  }

  temp = ( unsigned char * )calloc( RANGE, sizeof( unsigned char ) );
  tp = ( int * )calloc( RANGE, sizeof( int ) );
  original = graph->starting_chars;
  graph->starting_chars = tp;

  p_n_count = graph->starting_count;
  for ( i = 0; i < RANGE; i++ )
    tp[i] = false;

  for ( i = 0; i < RANGE; i++ )
    tp[i] = false;
  graph->starting_chars = tp;

  p_c_count = 0;
  p_frequency = 0;
  for ( i = 0; i < graph->starting_count; i++ )
  {
    dre2_set_chars( graph, graph->starting_points[i] );
    for ( j = 1; j < RANGE; j++ )
    {
      if ( graph->starting_chars[j] && !dre2_contains_char( temp, j ) )
      {
        p_c_count++;
        temp[p_c_count - 1] = j;
        p_frequency += dre2_frequency[j];
      }
    }
  }

  graph->starting_chars = original;
  free( temp ); temp = NULL;
  free( tp ); tp = NULL;

  if ( b_n_count * b_c_count < p_n_count * p_c_count ||
     ( b_n_count * b_c_count == p_n_count * p_c_count && b_frequency < p_frequency ) )
    return false;
  return true;
}

// Use first or last node as starting point.
int
dre2_first_or_last( struct dre2 *graph, struct dre2_fl_cost *cost )
{
  int i, j;
  double first, last, diff;

  first = ( double )cost->f_frequency;
  last = ( double )cost->l_frequency;
  diff = first / last;

  if ( cost->l_n_count == 1 && graph->v[graph->v[graph->count - 1].p[0]].c == DRE2_EOF )
  {
    return 0;
  } else {
    if ( cost->l_n_count * cost->l_c_count < cost->f_n_count * cost->f_c_count ||
       ( cost->l_n_count * cost->l_c_count == cost->f_n_count * cost->f_c_count && cost->l_frequency < cost->f_frequency ) ||
       ( cost->l_n_count * cost->l_c_count == cost->f_n_count * cost->f_c_count && diff >= 0.9 ) )
      return graph->count - 1;
  }
  return 0;
}

// Set the minimal bit to false.
void
dre2_remove_minimal( int **minimal, int id )
{
  minimal[0][id] = false;
}

// Add a minimal bit.
void
dre2_add_minimal( int **minimal, int *node_count )
{
  *minimal = ( int * )realloc( *minimal, sizeof( int ) * *node_count );
  minimal[0][*node_count - 1] = true;
}

// Add a node.
void
dre2_add_node( struct dre2_node **v, int *node_count, int c, int **minimal )
{
  // Initialize the node count and vertex array if necessary.
  if ( ! *node_count )
  {
    *node_count = 1;
    *v = ( struct dre2_node * )malloc( sizeof( struct dre2_node ) );
  } else
  {
    // Increment node count and allocate additional memory.
    *node_count = *node_count + 1;
    *v = ( struct dre2_node * )realloc( *v, sizeof( struct dre2_node ) * *node_count );
  }

  // Set this node's data.
  v[0][*node_count - 1].c = c;

  // Initialize neighbor array and count.
  v[0][*node_count - 1].n = ( int * )malloc( sizeof( int ) );
  v[0][*node_count - 1].n_count = 0;

  // Initalize the parent array and count.
  v[0][*node_count - 1].p = ( int * )malloc( sizeof( int ) );
  v[0][*node_count - 1].p_count = 0;

  // Allocate some memory for character class possible lookup list.
  if ( c == DRE2_CHAR_CLASS )
    v[0][*node_count - 1].possible = ( int * )malloc( sizeof( int ) * RANGE );

  // Initialize min_n to NULL.
  v[0][*node_count - 1].min_n = NULL;

  dre2_add_minimal( minimal, node_count );
}

// Add a node to another node's neighbors.
void
dre2_add_neighbor( struct dre2_node **v, int origin, int dest )
{
  // Add the neighbor.
  v[0][origin].n_count++;
  v[0][origin].n = ( int * )realloc( v[0][origin].n, sizeof( int ) * v[0][origin].n_count );
  v[0][origin].n[v[0][origin].n_count - 1] = dest;
}

// Add the parent nodes
void
dre2_add_parents( struct dre2 *graph )
{
  int i, j;
  for ( i = 0; i < graph->count; i++ )
  {
    struct dre2_node *node = &graph->v[i];
    for ( j = 0; j < graph->v[i].n_count; j++ )
    {
      struct dre2_node *neighbor = &graph->v[graph->v[i].n[j]];
      neighbor->p_count++;
      neighbor->p = ( int * )realloc( neighbor->p, sizeof( int ) * neighbor->p_count );
      neighbor->p[neighbor->p_count - 1] = i;
    }
  }
}

// Caculate the min length.
void
dre2_skip_table( struct dre2 *graph )
{
  int i, j, r;
  int iter, l_iter;
  int matched, complete, total;
  int *swapper, *visited;
  int *reachable, *state, *r_temp;
  struct dre2_node *node;

  // Allocate some memory.
  state = ( int * )malloc( sizeof( int ) * graph->count );
  reachable = ( int * )malloc( sizeof( int ) * graph->count );
  r_temp = ( int * )malloc( sizeof( int ) * graph->count );

  // Initialize some variables.
  i = 0;
  iter = 0;
  total = 0;
  complete = false;

  // Initialize the reachable states to be neighbors of the first node.
  for ( i = 0; i < graph->v[graph->count - 1].p_count; i++ )
  {
    reachable[i] = graph->v[graph->count - 1].p[i];
    if ( reachable[i] == 0 )
    {
      for ( j = 0; j < RANGE; j++ )
        graph->skip_table[j] = 1;
      graph->min_length = 0;
      return;
    }
  }
  l_iter = i;

  visited = ( int * )malloc( sizeof( int ) * graph->count );
  for ( j = 0; j < RANGE; j++ )
    graph->skip_table[j] = 0;

  // Outer loop, runs until the initial node is reachable.
  while ( true )
  {
    if ( l_iter == 0 )
      break;

    total++;
    iter = 0;

    // Iterate through all of the reachable nodes.
    for ( j = 0; j < graph->count; j++ )
    {
      visited[j] = false;
      state[j] = false;
    }
    for ( i = 0; i < l_iter; i++ )
    {
      r = reachable[i];
      node = &graph->v[r];
      if ( total > 1 )
      {

        // Setup the skip table for the current node's chars.
        if ( node->c == DRE2_CHAR_CLASS )
        {
          for ( j = 0; j < RANGE; j++ )
          {
            if ( node->possible[j] && graph->skip_table[j] > total - 1 || graph->skip_table[j] == 0 )
              graph->skip_table[j] = total - 1;
          }
        } else if ( node->c >= 0 )
        {
          if ( graph->skip_table[node->c] > total - 1 || graph->skip_table[node->c] == 0 )
            graph->skip_table[node->c] = total - 1;
        }
      }

      // Add the current node's parents to the next reachable nodes.
      for ( j = 0; j < node->p_count; j++ )
      {
        if ( !state[node->p[j]] && !visited[node->p[j]] )
          r_temp[iter++] = node->p[j];

        visited[node->p[j]] = true;
        state[node->p[j]] = true;
      }

      // Check if the initial state was reached.
      if ( state[0] == true )
      {
        complete = true;
      }
    }
    
    l_iter = iter;

    // Change the current reachable nodes to the next reachable nodes from above.
    swapper = reachable;
    reachable = r_temp;
    r_temp = swapper;
    if ( complete )
      break;
  }

  // Set remaining character's skip table value.
  for ( i = 0; i < RANGE; i++ )
  {
    if ( graph->skip_table[i] == 0 )
      graph->skip_table[i] = total;
  }

  // Set the min length of the regular expression.
  graph->min_length = total;

  free( reachable );
  free( r_temp );
  free( state );
  free( visited );
  reachable = NULL;
  r_temp = NULL;
  state = NULL;
  visited = NULL;
}

// Duplicate a group.
void
dre2_duplicate_group( struct dre2_node **v, int *node_count, int *last_node, struct dre2_parse_return *res, int **minimal )
{
  int i, j, diff;
  int o, c;

  for ( i = res->open; i <= res->close; i++ )
  {
    dre2_duplicate_node( v, node_count, i, minimal );
    if ( i != res->open )
    {
      v[0][*node_count - 1].p_count = v[0][i].p_count;
      v[0][*node_count - 1].p = ( int * )malloc( sizeof( int ) * v[0][i].p_count );
      for ( j = 0; j < v[0][i].p_count; j++ )
      {
        diff = v[0][i].p[j] - res->open;
        v[0][*node_count - 1].p[j] = *last_node + diff + 1;
      }
    } else
    {
      v[0][*node_count - 1].p_count = 1;
      v[0][*node_count - 1].p = ( int * )malloc( sizeof( int ) );
      v[0][*node_count - 1].p[0] = res->close;
    }

    if ( i != res->close )
    {
      v[0][*node_count - 1].n_count = v[0][i].n_count;
      v[0][*node_count - 1].n = ( int * )malloc( sizeof( int ) * v[0][i].n_count );
      for ( j = 0; j < v[0][i].n_count; j++ )
      {
        diff = v[0][i].n[j] - res->open;
        v[0][*node_count - 1].n[j] = *last_node + diff + 1;
      }
    }
  }
  res->open = res->close + 1;
  res->close = *node_count - 1;
  *last_node = res->close;
}

// Duplicate a node.
void
dre2_duplicate_node( struct dre2_node **v, int *node_count, int last_node, int **minimal )
{
  dre2_add_node( v, node_count, v[0][last_node].c, minimal );
  dre2_add_minimal( minimal, node_count );
  if ( v[0][last_node].c == DRE2_CHAR_CLASS )
    memcpy( v[0][*node_count - 1].possible, v[0][last_node].possible, RANGE * sizeof( int ) );
}

// Setup a character range, e.g. a{2,3}
void
dre2_make_range( struct dre2_node **v, int *node_count, int *last_node, struct dre2_parse_return *res, int min, int max, int **minimal )
{
  int i, j, last;
  int group = false;
  int max_position;
  int *parents, parent_count;

  if ( v[0][*last_node].c == DRE2_GROUP_CLOSE )
  {
    int length = res->close - res->open + 1;
    if ( max == DRE2_INFINITE )
      max_position = *last_node + length + 1;
    else
      max_position = *last_node + ( length * ( max - 1 ) ) + 1;
    group = true;
  } else
  {
    max_position = *last_node + max;
  }

  parents = ( int * )malloc( sizeof( int ) * *last_node );
  parent_count = 0;
  if ( min == 0 )
  {
    for ( i = 0; i < *last_node; i++ )
    {
      struct dre2_node *node = &v[0][i];
      for ( j = 0; j < node->n_count; j++ )
      {
        if ( ( group && node->n[j] == res->open ) || ( node->n[j] == *last_node ) )
        {
          parent_count++;
          parents[parent_count - 1] = i;
        }
      }
    }
    if ( v[0][*last_node].c == DRE2_GROUP_CLOSE )
    {
      for ( i = res->open; i <= res->close; i++ )
        dre2_remove_minimal( minimal, i );
    } else
    {
      dre2_remove_minimal( minimal, *last_node );
    }
  }

  for ( i = 0; i < min - 1; i++ )
  {
    dre2_add_neighbor( v, *last_node, *last_node + 1 );
    if ( group )
    {
      dre2_duplicate_group( v, node_count, last_node, res, minimal );
      
    } else
    {
      dre2_duplicate_node( v, node_count, *last_node, minimal );
      *last_node = *node_count - 1;
    }
  }

  if ( min > 0 )
  {
    parent_count = 1;
    parents[0] = *last_node;
  }

  if ( max == min )
  {
    free( parents );
    parents = NULL;
    return;
  }

  if ( min == 0 && group )
    dre2_add_neighbor( v, res->open, res->close );

  if ( max == DRE2_INFINITE )
  {
    if ( group )
      dre2_add_neighbor( v, res->close, res->open );
    else
      dre2_add_neighbor( v, *last_node, *last_node );
  } else
  {
    int last = *last_node;
    for ( i = min; i < max; i++ )
    {
      int last2 = *last_node;
      dre2_add_neighbor( v, *last_node, *node_count );
      if ( group )
      {
        if ( i > 0 )
        {
          dre2_duplicate_group( v, node_count, last_node, res, minimal );
          for ( j = res->open; j <= res->close; j++ )
            dre2_remove_minimal( minimal, j );
        }
      } else
      {
        if ( i > 0 )
        {
          dre2_duplicate_node( v, node_count, *last_node, minimal );
          dre2_remove_minimal( minimal, *node_count - 1 );
          *last_node = *node_count - 1;
        }
      }
      for ( j = 0; j < parent_count; j++ )
        dre2_add_neighbor( v, parents[j], *last_node );
    }
    for ( j = 0; j < parent_count; j++ )
      dre2_add_neighbor( v, parents[j], max_position );
  }

  free( parents );
  parents = NULL;
}

// Parsing algorithm.
struct dre2_parse_return
dre2_parse_recursive( struct dre2_node **v, int *node_count, unsigned char *re, int length, int pos, int **minimal )
{
  int i, j, k;
  int last_node;
  int *option_end, option_count;
  int mod;
  struct dre2_parse_return ret_val, res;
  struct dre2_range_return range;
  unsigned char c;

  // Add the group open node.
  dre2_add_node( v, node_count, DRE2_GROUP_OPEN, minimal );
  ret_val.open = *node_count - 1;

  // Which node was the preceeding node.
  last_node = ret_val.open;

  // End of option nodes.
  option_count = 0;
  option_end = ( int * )malloc( sizeof( int ) );

  // Loop through regex string.
  mod = false;
  while ( pos < length )
  {
    c = re[pos];
    if ( c == '(' )
    {
      res = dre2_parse_recursive( v, node_count, re, length, pos + 1, minimal );
      if ( res.pos == -1 )
      {
        free( option_end );
        option_end = NULL;
        ret_val.pos = -1;
        return ret_val;
      }

      // Add a link from the previous node to the group open node.
      dre2_add_neighbor( v, last_node, res.open );

      // Set the regex input pos.
      pos = res.pos;

      // Update the last node.
      last_node = res.close;

      mod = true;
    } else if ( c == ')' )
    {
      if ( ret_val.open == 0 )
      {
        free( option_end );
        option_end = NULL;
        ret_val.pos = -1;
        return ret_val;
      }

      // Add the group close node.
      dre2_add_node( v, node_count, DRE2_GROUP_CLOSE, minimal );
      ret_val.close = *node_count - 1;
      ret_val.pos = pos;
      for ( i = 0; i < option_count; i++ )
        dre2_add_neighbor( v, option_end[i], ret_val.close );
      dre2_add_neighbor( v, last_node, ret_val.close );
      free( option_end );
      option_end = NULL;
      return ret_val;
    } else if ( c == '|' )
    {
      // Store the last node so we can setup end of group links.
      option_count++;
      option_end = ( int * )realloc( option_end, sizeof( int ) * option_count );
      option_end[option_count - 1] = last_node;

      // Reset the last node to the group open.
      last_node = ret_val.open;
      mod = false;
    } else if ( c == '[' )
    {
      // Character class.
      dre2_add_node( v, node_count, DRE2_CHAR_CLASS, minimal );

      // Add it to the previous node's neighbor list.
      dre2_add_neighbor( v, last_node, *node_count - 1 );

      last_node = *node_count - 1;
      pos = dre2_character_class( &v[0][last_node], re, pos + 1 );
      if ( pos == false )
      {
        free( option_end );
        option_end = NULL;
        ret_val.pos = -1;
        return ret_val;
      }
      mod = true;
    } else if ( c == ']' || c == '}' )
    {
      free( option_end );
      option_end = NULL;
      ret_val.pos = -1;
      return ret_val;
    } else if ( c == '\\' )
    {
      // Character class.
      dre2_add_node( v, node_count, DRE2_CHAR_CLASS, minimal );

      // Add it to the previous node's neighbor list.
      dre2_add_neighbor( v, last_node, *node_count - 1 );

      last_node = *node_count - 1;
      dre2_predefined_class( &v[0][last_node], &re[pos + 1], true, false );
      pos++;
      mod = true;
    } else if ( c == '.' )
    {
      // Character class, all chars welcome.
      dre2_add_node( v, node_count, DRE2_CHAR_CLASS, minimal );

      // Add it to the previous node's neighbor list.
      dre2_add_neighbor( v, last_node, *node_count - 1 );

      last_node = *node_count - 1;
      for ( k = 0; k < RANGE; k++ )
        v[0][last_node].possible[k] = true;
      mod = true;
    } else if ( c == '*' )
    {
      if ( mod == false )
      {
        printf( "Invalid modifier.\n" );
        free( option_end );
        option_end = NULL;
        ret_val.pos = -1;
        return ret_val;
      }
      mod = false;
      if ( v[0][last_node].c == DRE2_GROUP_CLOSE )
      {
        // Add link from group open to close and vice versa.
        dre2_add_neighbor( v, res.open, res.close );
        dre2_add_neighbor( v, res.close, res.open );
        for ( i = res.open; i <= res.close; i++ )
          dre2_remove_minimal( minimal, i );
      } else
      {
        dre2_add_node( v, node_count, v[0][last_node].c, minimal );
        if ( v[0][last_node].c == DRE2_CHAR_CLASS )
        {
          for ( i = 0; i < RANGE; i++ )
            v[0][*node_count - 1].possible[i] = v[0][last_node].possible[i];
        }

        v[0][last_node].c = DRE2_GROUP_OPEN;

        dre2_add_node( v, node_count, DRE2_GROUP_CLOSE, minimal );

        // Add a link from group open to node.
        dre2_add_neighbor( v, last_node, last_node + 1 );

        // Add a link from group open to group close.
        dre2_add_neighbor( v, last_node, *node_count - 1 );

        // Add a link from the node to itself.
        dre2_add_neighbor( v, last_node + 1, last_node + 1 );

        // Add a link from the node to the group close.
        dre2_add_neighbor( v, last_node + 1, *node_count - 1 );

        // Update the minimal bits.
        dre2_remove_minimal( minimal, last_node );
        dre2_remove_minimal( minimal, last_node + 1 );
        dre2_remove_minimal( minimal, *node_count - 1 );

        last_node = *node_count - 1;
      }
    } else if ( c == '?' )
    {
      if ( mod == false )
      {
        printf( "Invalid modifier.\n" );
        free( option_end );
        option_end = NULL;
        ret_val.pos = -1;
        return ret_val;
      }
      mod = false;
      // If it's a group, add a link from group open to group close.
      if ( v[0][last_node].c == DRE2_GROUP_CLOSE )
      {
        dre2_add_neighbor( v, res.open, res.close );
        for ( i = res.open; i <= res.close; i++ )
          dre2_remove_minimal( minimal, i );
      } else
      {
        dre2_add_node( v, node_count, v[0][last_node].c, minimal );
        if ( v[0][last_node].c == DRE2_CHAR_CLASS )
        {
          for ( i = 0; i < RANGE; i++ )
            v[0][*node_count - 1].possible[i] = v[0][last_node].possible[i];
        }

        v[0][last_node].c = DRE2_GROUP_OPEN;
        dre2_add_node( v, node_count, DRE2_GROUP_CLOSE, minimal );

        // Add a link from group open to node.
        dre2_add_neighbor( v, last_node, last_node + 1 );

        // Add a link from group open to group close.
        dre2_add_neighbor( v, last_node, *node_count - 1 );

        // Add a link from the node to the group close.
        dre2_add_neighbor( v, last_node + 1, *node_count - 1 );

        // Update the minimal bits.
        dre2_remove_minimal( minimal, last_node );
        dre2_remove_minimal( minimal, last_node + 1 );
        dre2_remove_minimal( minimal, *node_count - 1 );

        last_node = *node_count - 1;
      }
    } else if ( c == '+' )
    {
      if ( mod == false )
      {
        printf( "Invalid modifier.\n" );
        free( option_end );
        option_end = NULL;
        ret_val.pos = -1;
        return ret_val;
      }
      mod = false;

      if ( v[0][last_node].c == DRE2_GROUP_CLOSE )
        dre2_add_neighbor( v, res.close, res.open );
      else
        dre2_add_neighbor( v, last_node, last_node );
    } else if ( c == '{' )
    {
      if ( mod == false )
      {
        printf( "Invalid modifier.\n" );
        free( option_end );
        option_end = NULL;
        ret_val.pos = -1;
        return ret_val;
      }
      mod = false;

      // Get the size range.
      range = dre2_range( re, length, pos );

      // Make sure it was a valid range.
      if ( !range.valid )
      {
        free( option_end );
        option_end = NULL;
        ret_val.pos = -1;
        return ret_val;
      }

      // Setup the range.
      dre2_make_range( v, node_count, &last_node, &res, range.min, range.max, minimal );
      pos = range.pos;
    } else
    {
      // Add the node.
      dre2_add_node( v, node_count, c, minimal );

      // Add it to the previous node's neighbor list.
      dre2_add_neighbor( v, last_node, *node_count - 1 );

      // Update last node.
      last_node = *node_count - 1;
      mod = true;
    }
    pos++;
  }

  if ( ret_val.open != 0 )
  {
    free( option_end );
    option_end = NULL;
    ret_val.pos = -1;
    return ret_val;
  }

  // Add links from the last node in each option group to the group close node.
  dre2_add_node( v, node_count, DRE2_GROUP_CLOSE, minimal );
  for ( i = 0; i < option_count; i++ )
    dre2_add_neighbor( v, option_end[i], *node_count - 1 );

  dre2_add_neighbor( v, last_node, *node_count - 1 );

  free( option_end );
  option_end = NULL;

  return ret_val;
}

// Parse function wrapper.
struct dre2
dre2_parse( unsigned char *re )
{
  int i;
  struct dre2_node *v, *min_v;
  int node_count;
  int length;
  int *minimal, *new_minimal, minimal_count, *minimal_id;
  struct dre2_parse_return ret;
  struct dre2 min_graph;

  // Call the recursive parse function.
  struct dre2 graph;
  graph.state = NULL;
  graph.reachable = NULL;
  graph.r_temp = NULL;
  graph.starting_points = NULL;
  graph.starting_chars = NULL;
  v = NULL;

  minimal = ( int * )malloc( sizeof( int ) );
  node_count = 0;
  length = strlen( re );
  ret = dre2_parse_recursive( &v, &node_count, re, length, 0, &minimal );
  if ( ret.pos == -1 )
  {
    cleanup_nodes( &v, node_count );
    free( minimal );
    free( v );
    minimal = NULL;
    v = NULL;
    graph.v = v;

    return graph;
  }

  // Setup the parents for getting the min graph.
  graph.v = v;
  graph.count = node_count;
  dre2_add_parents( &graph );

  min_graph.starting_points = NULL;
  min_graph.starting_chars = ( int * )calloc( RANGE, sizeof( int ) );

  new_minimal = ( int * )calloc( graph.count, sizeof( int ) );
  minimal_id = ( int * )malloc( sizeof( int ) * graph.count );
  minimal_count = 0;

  // Strip out the group nodes and make it an epsilon-free graph.
  min_v = NULL;
  dre2_strip_groups( &graph, &min_graph, &min_v, minimal, &new_minimal, &minimal_count, &minimal_id );
  min_graph.v = min_v;

  // Setup the reverse graph.
  dre2_add_parents( &min_graph );

  // Setup the skip table.
  dre2_skip_table( &min_graph );

  // Find the best starting point and chars.
  min_graph.starting_point = dre2_starting_point( &min_graph, new_minimal, minimal_id, minimal_count );
  if ( min_graph.starting_point == 1 )
    min_graph.starting_point = 0;
  if ( min_graph.starting_point == min_graph.count - 2 )
    min_graph.starting_point = min_graph.count - 1;
  dre2_starting_chars( &min_graph, new_minimal );

  // Clean up the original graph's memory.
  cleanup_dre2( &graph );

  free( new_minimal );
  free( minimal_id );
  free( minimal );
  new_minimal = NULL;
  minimal_id = NULL;
  minimal = NULL;

  min_graph.state = NULL;
  min_graph.r_temp = ( int * )malloc( sizeof( int ) * min_graph.count );
  min_graph.reachable = ( int * )malloc( sizeof( int ) * min_graph.count );

  if ( min_graph.single )
  {
    for ( i = 0; i < RANGE; i++ )
    {
      if ( min_graph.starting_chars[i] )
        min_graph.c = i;
    }
  }

  if ( min_graph.starting_point == -1 )
  {
    min_graph.match_method = DRE2_MN;
  } else if ( min_graph.starting_point == 0 || min_graph.starting_point != min_graph.count - 1 )
  {
    if ( min_graph.single )
      min_graph.match_method = DRE2_SN_SC;
    else
      min_graph.match_method = DRE2_SN_MC;
  } else
  {
    if ( min_graph.single )
      min_graph.match_method = DRE2_SN_SC_H;
    else
      min_graph.match_method = DRE2_SN_MC_H;
  }

  return min_graph;
}

// Display the regex dre2.
void
print_dre2( struct dre2 *graph )
{
  printf( "=======================\n" );
  printf( "|| Node || Neighbors ||\n" );
  printf( "=======================\n" );
  int i, j;
  for ( i = 0; i < graph->count; i++ )
  {
    if ( graph->v[i].c == DRE2_CHAR_CLASS )
    {
      printf( "Node %d C: ", i );
    } else
    {
      printf( "Node %d (%c): ", i, graph->v[i].c == DRE2_GROUP_OPEN || graph->v[i].c == DRE2_GROUP_CLOSE ? 'G' : graph->v[i].c );
    }
    for ( j = 0; j < graph->v[i].n_count; j++ )
      printf( "%d, ", graph->v[i].n[j] );
    printf( "\n" );
  }
  printf( "\n" );
}

// Display the reverse dre2.
void
print_reverse_dre2( struct dre2 *graph )
{
  printf( "=======================\n" );
  printf( "|| Node || Parents   ||\n" );
  printf( "=======================\n" );
  int i, j;
  for ( i = graph->count - 1; i >= 0; i-- )
  {
     printf( "Node %d (%c): ", i, graph->v[i].c == DRE2_GROUP_OPEN || graph->v[i].c == DRE2_GROUP_CLOSE ? 'G' : graph->v[i].c );
     for ( j = 0; j < graph->v[i].p_count; j++ )
       printf( "%d, ", graph->v[i].p[j] );
     printf( "\n" );
  }
  printf( "\n" );
}
