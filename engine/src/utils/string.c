#include "string.h"

#include <core/assert.h>

b8 strings_equal(const char *lhs, const char *rhs)
{
	LASSERT(lhs != NULL);
	LASSERT(rhs != NULL);

	while (*lhs && *rhs) {
		if (*lhs != *rhs) {
			return false;
		}
		lhs++;
		rhs++;
	}
	return *lhs == *rhs;
}
