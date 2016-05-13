vmxinfo
=======

Get information about VMX (Virtual Machine eXtensions) on a CPU.

This is a kernel module that, when inserted, gets information from the running
CPU about the ability (or lack thereof) for hardware-assisted virtualization
and writes this information out to the system log.

# Usage

You'll need a Linux machine with the kernel headers installed. This differs
from distro to distro, but usually it's just an extra package to install.

Download and compile the module:

```bash
git clone https://github.com/briansteffens/vmxinfo
cd vmxinfo
make
```

If it worked (does vmxinfo.ko exist?), insert the module:

```bash
sudo insmod vmxinfo.ko
```

It should error out, but don't worry, that's expected! This module has no
reason to run long-term so when loaded, it does its work, writes to the logs,
and then returns a failure so it gets unloaded right away.

To view the output, check your system logs. This will differ between
distributions.

**If you have systemd:**

```bash
sudo journalctl -e
```

**Otherwise:**

```bash
sudo tail -f /var/log/messages
```

You should see something like the following (but with values specific to your
system [and possibly less information if your system doesn't support as many
VMX features]):

```bash
May 12 20:38:27 main kernel: ---vmxinfo starting-----------------------------
May 12 20:38:27 main kernel: 64-bit mode
May 12 20:38:27 main kernel: CPU vendor ID: GenuineIntel
May 12 20:38:27 main kernel: Highest CPUID index available: 13
May 12 20:38:27 main kernel: Virtual Machine eXtensions (VMX): 1
May 12 20:38:27 main kernel: VMCS revision identifier: 16
May 12 20:38:27 main kernel: VMCS region size: 1024
May 12 20:38:27 main kernel: VMXON max width (Intel 64 support): 1
May 12 20:38:27 main kernel: Dual-monitor treatment support: 1
May 12 20:38:27 main kernel: VMCS access memory type: writeback (6)
May 12 20:38:27 main kernel: VM exit reporting: 1
May 12 20:38:27 main kernel: VMX controls can be cleared to 0: 1
May 12 20:38:27 main kernel: ---vmxinfo done---------------------------------
```
