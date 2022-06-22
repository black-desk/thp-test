#include <stdlib.h>
int main()
{
        int *arr = malloc(sizeof(int) * 100000000);
        arr[0] = 1;
        arr[1000] = 1;
        arr[2000] = 1;
        arr[4000] = 1;
        while (1) {
        }
        return 0;
}
