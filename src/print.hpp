#ifndef PRINT_HPP
#define PRINT_HPP

#include <stdio.h>
#define print_error(...)																	\
	fprintf(stderr, "\e[36m%s: \"%s\" %i:\e[0m ", __FILE__, __PRETTY_FUNCTION__, __LINE__);		\
	fprintf(stderr, __VA_ARGS__);															\
	fprintf(stderr, "\n");

#endif