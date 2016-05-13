#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Brian Steffens <briansteffens@gmail.com>");
MODULE_DESCRIPTION("VMX information module");

#if defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64)
    #define _x64_
#endif

#define CPUID_HIGHEST_FUNCTION 0
#define CPUID_FEATURES 1

#define EAX 0
#define EBX 1
#define ECX 2
#define EDX 3

#define MSR_VMX_BASIC 0x480

#define VMCS_MEM_UNCACHEABLE 0
#define VMCS_MEM_WRITEBACK 6

void show_architecture(void)
{
#if defined(_x64_)
    printk(KERN_INFO "64-bit mode\n");
#else
    printf(KERN_INFO "32-bit mode\n");
#endif
}

void cpuid_call(unsigned index, unsigned registers[4])
{
    __asm__ __volatile__(
#if defined(_x64_)
            "pushq %%rbx          \n\t"
#else
            "pushl %%ebx          \n\t"
#endif
            "cpuid                \n\t"
            "movl %%ebx, %[ebx]   \n\t"
#if defined(_x64_)
            "popq %%rbx           \n\t"
#else
            "popl %%ebx           \n\t"
#endif
            : "=a"(registers[EAX]), [ebx] "=r"(registers[EBX]),
              "=c"(registers[ECX]), "=d"(registers[EDX])
            : "a"(index));
}

int check_cpuid_highest_function(void)
{
    unsigned int registers[4];
    unsigned i;
    unsigned highest_function;
    char vendor_id[13];

    cpuid_call(CPUID_HIGHEST_FUNCTION, registers);

    // Assemble CPU vendor ID
    for (i = 4; i < 8; i++)
    {
        vendor_id[i - 4] = ((char*)registers)[i];
    }

    for (i = 12; i < 16; i++)
    {
        vendor_id[i - 8] = ((char*)registers)[i];
    }

    for (i = 8; i < 12; i++)
    {
        vendor_id[i] = ((char*)registers)[i];
    }

    vendor_id[12] = 0;

    printk(KERN_INFO "CPU vendor ID: %s\n", vendor_id);

    // Check highest function result
    highest_function = registers[EAX];

    printk(KERN_INFO "Highest CPUID index available: %d\n", highest_function);

    if (highest_function < CPUID_FEATURES)
    {
        printk(KERN_INFO "CPU does not support the next call, exiting\n");
        return -1;
    }

    return 0;
}

void rdmsr_call(int index, unsigned int registers[4])
{
    __asm__ __volatile__(
            "rdmsr"
            : "=a"(registers[EAX]), "=d"(registers[EDX])
            : "c"(index));
}

unsigned char get_byte(void* value, int byte_index)
{
    return ((unsigned char*)(value))[byte_index];
}

unsigned get_bit(void* value, int bit_index)
{
    int byte_index = bit_index / 8;
    int bit_offset = bit_index % 8;

    return (get_byte(value, byte_index) >> bit_offset) & 1;
}

unsigned extract_bits(void* source, int source_start, int output_offset,
        int count)
{
    unsigned ret = 0;
    int i;

    for (i = source_start; i < source_start + count; i++)
    {
        ret |= (get_bit(source, i) << (i - source_start + output_offset));
    }

    return ret;
}

int check_for_vmx_support(void)
{
    unsigned registers[4];
    bool feature_vmx;

    cpuid_call(CPUID_FEATURES, registers);

    feature_vmx = get_bit(&registers[ECX], 5);

    printk(KERN_INFO "Virtual Machine eXtensions (VMX): %d\n", feature_vmx);

    if (!feature_vmx)
    {
        printk(KERN_INFO "CPU does not support VMX, exiting\n");
        return 0;
    }

    return 0;
}

int check_vmx_settings(void)
{
    unsigned registers[4];
    unsigned vmcs_access_mem_type;

    rdmsr_call(MSR_VMX_BASIC, registers);
    printk(KERN_INFO "VMCS revision identifier: %d\n", registers[EAX]);

    if (get_bit(&registers[EDX], 12) == 0)
    {
        printk(KERN_INFO "Unhandled VMCS size specification");
        return -1;
    }

    printk(KERN_INFO "VMXON max width (Intel 64 support): %d\n",
            !get_bit(&registers[EDX], 16));

    printk(KERN_INFO "Dual-monitor treatment support: %d\n",
            get_bit(&registers[EDX], 17));

    // VMCS access memory type
    vmcs_access_mem_type = extract_bits(&registers[EDX], 18, 0, 4);

    switch (vmcs_access_mem_type)
    {
        case VMCS_MEM_UNCACHEABLE:
            printk(KERN_INFO "VMCS access memory type: uncacheable (0)\n");
            break;

        case VMCS_MEM_WRITEBACK:
            printk(KERN_INFO "VMCS access memory type: writeback (6)\n");
            break;

        default:
            printk(KERN_INFO "Unrecognized VMCS access memory type\n");
            return -1;
    }

    printk(KERN_INFO "VM exit reporting: %d\n", get_bit(&registers[EDX], 22));

    printk(KERN_INFO "VMX controls can be cleared to 0: %d\n",
            get_bit(&registers[EDX], 23));

    return 0;
}

static int __init supermod_init(void)
{
    printk(KERN_INFO "---vmxinfo starting-----------------------------\n");

    show_architecture();

    if (check_cpuid_highest_function() != 0)
    {
        return -1;
    }

    if (check_for_vmx_support() != 0)
    {
        return -1;
    }

    if (check_vmx_settings() != 0)
    {
        return -1;
    }
}

static void __exit supermod_exit(void)
{
    printk(KERN_INFO "---vmxinfo done---------------------------------\n");
}

module_init(supermod_init);
module_exit(supermod_exit);
