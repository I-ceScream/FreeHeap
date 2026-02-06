#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "heap.h"

/* 定义一个简单的打印函数，方便观察 */
void print_heap_info(const char* tag) {
    printf("[%s] Current Free: %zu bytes, Min Ever Free: %zu bytes\n", 
           tag, xPortGetFreeHeapSize(), xPortGetMinimumEverFreeHeapSize());
}

int main(void) {
    printf("--- Final Heap_4 Independent Module Test ---\n\n");

    // 1. 初始化后的状态 (第一次 malloc 会触发初始化)
    printf("Initial State:\n");
    print_heap_info("START");

    // 2. 连续分配测试与对齐检测
    printf("\n1. Allocation & Alignment Test:\n");
    void *p1 = pvPortMalloc(10); // 故意给一个不对齐的大小
    void *p2 = pvPortMalloc(128);
    void *p3 = pvPortMalloc(50);
    void *p4 = pvPortMalloc(100);

    if (p1 && p2 && p3) {
        printf("  p1 (10b) addr: %p (aligned: %s)\n", p1, ((uintptr_t)p1 % 8 == 0) ? "YES" : "NO");
        printf("  p2(128b) addr: %p (aligned: %s)\n", p2, ((uintptr_t)p2 % 8 == 0) ? "YES" : "NO");
        printf("  p3 (50b) addr: %p (aligned: %s)\n", p3, ((uintptr_t)p3 % 8 == 0) ? "YES" : "NO");
        printf("  p4(100b) addr: %p (aligned: %s)\n", p4, ((uintptr_t)p4 % 8 == 0) ? "YES" : "NO");
    }
    print_heap_info("AFTER_ALLOC");

    // 3. 内存写入安全测试 (确保分配的地址有效)
    printf("\n2. Memory Write Test:\n");
    memset(p1, 0xAA, 10);
    memset(p2, 0xBB, 128);
    memset(p3, 0xCC, 50);
    printf("  Memory write successful.\n");

    // 4. 碎片合并 (Coalescence) 深度测试
    // 我们释放 p2 (中间块)，然后再释放 p1 (前块)
    // 逻辑上 p1 和 p2 应该合并成一个连续的空闲块
    printf("\n3. Fragmentation Coalescence Test:\n");
    printf("  Freeing middle block (p2)...\n");
    vPortFree(p2);
    print_heap_info("FREE_P2");

    printf("  Freeing adjacent block (p1)...\n");
    vPortFree(p1);
    print_heap_info("FREE_P1_P2_MERGED");

    // 5. 极端情况测试
    printf("\n4. Boundary Test:\n");
    void *p_too_large = pvPortMalloc(1024 * 1024); // 尝试申请 1MB
    if (p_too_large == NULL) {
        printf("  Correctly refused oversized allocation (1MB).\n");
    }

    // 6. 最终清理与回归测试
    printf("\n5. Cleanup Test:\n");
    vPortFree(p3);
    vPortFree(p4);
    print_heap_info("FINAL");

    // 理论上最终剩余大小应该等于初始化后的最大可用空间
    // 如果最终剩余大小等于之前某个状态的最大值，说明合并逻辑完美
    printf("\nTest Complete.\n");

    return 0;
}