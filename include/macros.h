#define MAX(a, b) (a) > (b) ? (a) : (b)

#define ALLOC(t) (t *)malloc(sizeof(t))
#define ALLOC_ARR(t, n) (t *)calloc(n, sizeof(t))
#define ZERO_MEM(ptr, n) memset(ptr, 0, n);
#define ZERO_TYPE(ptr, t) memset(ptr, 0, sizeof(t));
#define ZERO_ARR(arr, t, n) memset(arr, 0, sizeof(t) * n);
