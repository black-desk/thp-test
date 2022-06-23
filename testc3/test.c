#include <stdlib.h>
#include "stdint.h"
int main()
{
        uint32_t size = 1UL << (22 - 2);
        int32_t *arr = malloc(size * 4); // 4Mb
        arr += (1UL << (21 - 2)) - 1; // 2Mb - 1 Byte
        *arr = 0;
        while (1) {
        }
        return 0;
}
