# Compile the WSL kernel for development in Windows

## Context
These are the instructions to compile a WSL kernel with `usbserial` module and all available `USB-to-TTL` cable drivers installed by default. Check [Development in Windows](https://inputlabs.io/devices/alpakka/manual/dev_windows) for more context.

Alternatively you can get the kernel compiled by us in [Releases](https://github.com/inputlabs/alpakka_firmware/releases) (wsl_kernel_dev).

**This is NOT needed if you are developing in Linux or MacOS.**

## Get source code
```
sudo apt install build-essential flex bison dwarves libssl-dev libelf-dev
git clone --depth 1 https://github.com/microsoft/WSL2-Linux-Kernel.git
cd WSL2-Linux-Kernel
cp Microsoft/config-wsl .config
```

## Configure
```
make menuconfig
```
In the interactive menu:
- Go into `Device drivers`
- Go into `USB support`
- Go into `USB serial converter support`
- Select all the drivers as `<*>` (with the space key)
- Save config to `.config`

## Compile
Compile in parallel, with the number or physical cores you have, eg: 6 cores.
```
make -j6
```

## Result
The kernel file is located `./arch/x86/boot/bzImage`

## Use
- Rename and move the kernel `bzImage` to Windows `C:\Users\youruser\wsl_kernel_dev`
- Create a WSL config file at `C:\Users\youruser\.wslconfig` with the content:

```
[wsl2]
kernel=C:\\Users\\youruser\\wsl_kernel_dev
```

- Restart WSL with `wsl --shutdown`

After this point you should be able to use the instructions for [Development in GNU/Linux](https://inputlabs.io/devices/alpakka/manual/dev_unix) within Windows.


