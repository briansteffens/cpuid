#include <stdio.h>
#include <stdlib.h>

typedef struct bits
{
    unsigned b0:1;
    unsigned b1:1;
    unsigned b2:1;
    unsigned b3:1;
    unsigned b4:1;
    unsigned b5:1;
    unsigned b6:1;
    unsigned b7:1;
} bits;

typedef enum CPUID_INDEX
{
    HIGHEST_FUNCTION = 0,
    FEATURES = 1,
} CPUID_INDEX;

void cpuid(CPUID_INDEX index, unsigned int regs[4])
{
    __asm__ __volatile__(
#if defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64)
            "pushq %%rbx          \n\t" /* save %rbx */
#else
            "pushl %%ebx          \n\t" /* save %ebx */
#endif
            "cpuid                \n\t"
            "movl %%ebx, %[ebx]   \n\t" /* write result into output var */
#if defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64)
            "popq %%rbx           \n\t"
#else
            "popl %%ebx           \n\t"
#endif
            : "=a"(regs[0]), [ebx] "=r"(regs[1]), "=c"(regs[2]), "=d"(regs[3])
            : "a"(index));
}

int cpuid_highest_function(char vendor_id[13])
{
    unsigned int regs[4];

    cpuid(HIGHEST_FUNCTION, regs);

    for (int i = 4; i < 8; i++)
    {
        vendor_id[i - 4] = ((char*)regs)[i];
    }

    for (int i = 12; i < 16; i++)
    {
        vendor_id[i - 8] = ((char*)regs)[i];
    }

    for (int i = 8; i < 12; i++)
    {
        vendor_id[i] = ((char*)regs)[i];
    }

    vendor_id[12] = 0;

    return regs[0];
}

int main()
{
    char vendor_id[13];
    int highest_function = cpuid_highest_function(vendor_id);

    printf("CPU vendor id: %s\n", vendor_id);
    printf("Highest CPUID function available: %d\n", highest_function);

    if (highest_function < FEATURES)
    {
        printf("CPU does not support the next call.\n");
        exit(EXIT_FAILURE);
    }

    unsigned int regs[4];
    cpuid(FEATURES, regs);

    printf("Features\n");
    printf("  Virtual Machine eXtensions (VMX): %d\n", ((bits*)regs)[2].b5);

    return 0;
}
