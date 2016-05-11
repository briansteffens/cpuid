#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#if defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64)
    #define _x64_
#endif

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

typedef enum REGISTER
{
    EAX,
    EBX,
    ECX,
    EDX,
} REGISTER;

typedef enum MSR_OFFSET
{
    VMX_BASIC = 0x480,
} MSR_OFFSET;

void show_architecture()
{
#if defined(_x64_)
    printf("64-bit CPU detected.\n");
#else
    printf("32-bit CPU detected.\n");
#endif
}

void cpuid(CPUID_INDEX index, unsigned int registers[4])
{
    __asm__ __volatile__(
#if defined(_x64_)
            "pushq %%rbx          \n\t" /* save %rbx */
#else
            "pushl %%ebx          \n\t" /* save %ebx */
#endif
            "cpuid                \n\t"
            "movl %%ebx, %[ebx]   \n\t" /* write result into output var */
#if defined(_x64_)
            "popq %%rbx           \n\t"
#else
            "popl %%ebx           \n\t"
#endif
            : "=a"(registers[EAX]), [ebx] "=r"(registers[EBX]),
              "=c"(registers[ECX]), "=d"(registers[EDX])
            : "a"(index));
}

int cpuid_highest_function(char vendor_id[13])
{
    unsigned int registers[4];

    cpuid(HIGHEST_FUNCTION, registers);

    for (int i = 4; i < 8; i++)
    {
        vendor_id[i - 4] = ((char*)registers)[i];
    }

    for (int i = 12; i < 16; i++)
    {
        vendor_id[i - 8] = ((char*)registers)[i];
    }

    for (int i = 8; i < 12; i++)
    {
        vendor_id[i] = ((char*)registers)[i];
    }

    vendor_id[12] = 0;

    return registers[EAX];
}

void rdmsr(int index, unsigned int registers[4])
{
    __asm__ __volatile__(
#if defined(_x64_)
            "pushq %%rax          \n\t"
#else
            "pushl %%eax          \n\t"
#endif
            "rdmsr                \n\t"
//            "movl %%ebx, %[ebx]   \n\t"
#if defined(_x64_)
            "popq %%rax           \n\t"
#else
            "popl %%eax           \n\t"
#endif
//            : "=a"(registers[EAX]), [ebx] "=r"(registers[EBX]),
//              "=c"(registers[ECX]), "=d"(registers[EDX])
            :: "c"(index));
}

int main()
{
    show_architecture();

    char vendor_id[13];
    int highest_function = cpuid_highest_function(vendor_id);

    printf("\nCPU vendor ID: %s\n", vendor_id);
    printf("Highest CPUID function available: %d\n", highest_function);

    if (highest_function < FEATURES)
    {
        printf("CPU does not support the next call.\n");
        exit(EXIT_FAILURE);
    }

    unsigned int registers[4];
    cpuid(FEATURES, registers);

    bool feature_vmx = ((bits*)registers)[2].b5;

    printf("\nFeatures:\n");
    printf("  Virtual Machine eXtensions (VMX): %d\n", feature_vmx);

    if (!feature_vmx)
    {
        printf("CPU does not support VMX.\n");
        exit(EXIT_FAILURE);
    }

    //rdmsr(VMX_BASIC, registers);
    //printf("VMCS revision identifier: %d\n", registers[EDX]);

    return 0;
}
