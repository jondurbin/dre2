all:
	gcc -o dre2 examples/dre2.c src/dre2_parse.c src/dre2_match.c src/dre2_backtracking.c -O3
	gcc -o dre2_submatch examples/submatch.c src/dre2_parse.c src/dre2_match.c src/dre2_backtracking.c -O3
