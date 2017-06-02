#include "typedefs.h"
#include "package.h"

int package_comparator(Package* lhs, Package* rhs) {
	if (lhs->priority == rhs->priority) {
		return (int)lhs->weight - (int)rhs->weight;
	}

	return lhs->priority - rhs->priority;
}
