#define INT_SHIFT 2
#define SIZE (1UL << (22 - INT_SHIFT))

int cnt = 0;

void f()
{
        if (cnt == 10) {
                while (1) {
                }
        }
        int a[SIZE]; // 4M
        a[SIZE / 2 - 1] = 0;
        cnt++;
        f();
}

int main()
{
        f();
        return 0;
}
