#include <stdlib.h>
int main()
{
        int size = 67108864 / sizeof(int);
        int *arr = malloc(sizeof(int) * size);
        int cnt = 0;
        for (int i = 0; i < size && cnt < 5; i += size / 2048) {
                cnt++;
                arr[i] = i;
        }
        while (1) {
        }
        return 0;
}
