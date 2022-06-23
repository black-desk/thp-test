# THP test

本仓库尝试研究内核的透明大页分配的触发机制.以探究为什么实际上并没有申请什么内存
的go程序也会消耗很多内存.

为了复现,本仓库中提交了二进制程序.

## 写一个go程序

写了一个简单的go程序,放在`./testgo`中:

``` go
package main

func main() {
	for {
	}
}
```

其[strace记录](./testgo/strace)表示,golang的runtime在启动后,使用mmap申请了一
段4096KB的内存用作其子进程的栈,这些线程大概是go语言runtime设计中的M.

并且通过对比不同THP设置下go程序的smaps可以确定,这段4M的内存触发了内核的大页分配
机制,装载了go程序确实没有使用到的内存.

## 用c程序来模仿过程

c程序`./testc`并不能触发内核的大页分配:

``` c
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
```

但是`./testc2`可以触发:

``` c
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
```

## 内核中的相关代码

``` c
/* copy from 78ca55889a549a9a194c6ec666836329b774ab6d */

/* #define PMD_SHIFT	21 */

/* ... */

#define HPAGE_PMD_SHIFT PMD_SHIFT
#define HPAGE_PMD_SIZE ((1UL) << HPAGE_PMD_SHIFT)
#define HPAGE_PMD_MASK	(~(HPAGE_PMD_SIZE - 1))

/* ... */

static inline bool transhuge_vma_suitable(struct vm_area_struct *vma,
                                          unsigned long haddr)
{
        /* Don't have to check pgoff for anonymous vma */
        if (!vma_is_anonymous(vma)) {
                if (!IS_ALIGNED((vma->vm_start >> PAGE_SHIFT) - vma->vm_pgoff,
                                HPAGE_PMD_NR))
                        return false;
        }

        if (haddr < vma->vm_start || haddr + HPAGE_PMD_SIZE > vma->vm_end)
                return false;
        return true;
}

/* ... */

vm_fault_t do_huge_pmd_anonymous_page(struct vm_fault *vmf)
{
        struct vm_area_struct *vma = vmf->vma;
        gfp_t gfp;
        struct folio *folio;
        unsigned long haddr = vmf->address & HPAGE_PMD_MASK;

        if (!transhuge_vma_suitable(vma, haddr))
                return VM_FAULT_FALLBACK;

        /* ... */
}

/* ... */
```

所以我们知道了,经过构造,只需要一次内存访问,也能触发到THP,写出`./testc3`:

``` c
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
```

结论:

任何向内核申请了单个大于2M的连续虚拟内存的程序,都可以触发到THP机制.所以这对于申
请了内存但是并不实际使用的用户态程序而言,会造成内存的浪费.
