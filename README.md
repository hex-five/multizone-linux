# MultiZone for Linux SDK

Initial target is SiFive's Unleashed U540.

U540 BSP components: 

* MultiZone: loaded to flash via OpenOCD => 0x2000_0000;
* ZSBL:      loaded to flash via OpenOCD => 0x2080_0000;
* FSBL:      loaded to mSD card => 0x0800_0000 L2 LIM;
* BBL Linux: loaded to mSD card => 0x8000_0000 DDR;

This repository is maintained by Hex Five Security.
For Questions or feedback - send email to info 'at' hex-five.com


## Get it Ready

### Prebuilt RISC-V Toolchain

Download Hex Five's reference toolchain or build your own:

1. GNU Embedded Toolchain - v2018.12.26:
http://hex-five.com/riscv-gnu-toolchain-20181226

2. OpenOCD - v2018.12.26:
http://hex-five.com/riscv-openocd-20181226

### Install the Toolchain

Install the prebuilt RISC-V toolchain for Linux:
 ```
 sudo apt update
 sudo apt upgrade -y
 sudo apt install git make default-jre libftdi1-dev
 sudo ln -s /usr/lib/x86_64-linux-gnu/libmpfr.so.6 /usr/lib/x86_64-linux-gnu/libmpfr.so.4
 wget https://hex-five.com/wp-content/uploads/riscv-gnu-toolchain-20181226.tar.xz
 tar -xvf riscv-gnu-toolchain-20181226.tar.xz
 wget https://hex-five.com/wp-content/uploads/riscv-openocd-20181226.tar.xz
 tar -xvf riscv-openocd-20181226.tar.xz
 git clone https://github.com/hex-five/multizone-sdk
 sudo apt-get install libusb-0.1-4
 sudo apt-get install picocom
 sudo apt-get install screen
```

> Note: for the Ubuntu 19.04 LTS you may need to install the libncurses5 package.
> ```
> sudo apt-get install libncurses5:i386 
> sudo apt-get install libncurses5:amd64
> ```

Add environment variables and a path to allow the Makefiles to find the toolchain by editing the ~/.bashrc and placing the following text at the bottom of the file:

```
export RISCV=/home/<username>/riscv-gnu-toolchain-20181226
export OPENOCD=/home/<username>/riscv-openocd-20181226
export PATH="$PATH:/home/<username>/riscv-gnu-toolchain-20181226/bin"
```

> Note: <_username_> shall be replaced by your username.

Close and restart the terminal session for these changes to take effect.
 


## Flash Programming Instructions

1. Download the prebuilt MultiZone for Linux package from Hex Five's website (http://hex-five.com/multizone-linux-20190808):

```
$ wget https://hex-five.com/wp-content/uploads/multizone-linux-20190808.tar.xz
$ tar -xvf multizone-linux-20190808.tar.xz
$ cd multizone-linux-release/
```

2. Flash the Board (Multizone & ZSBL):

```
$ ./flash.sh board
```

2. Flash the SD Card (FSBL & BBL)

```
$ ./flash.sh </dev/sdX>
```

> Note: </dev/sdX> shall be replaced with the device name of the SD card (e.g, /dev/mmcblock0 or /dev/sda). 


## MultiZone Instructions

(If you just want to test the Multizone for Linux SDK reference image, please skip directly to "Demo".)

1. Clone the MultiZone for Linux SDK repo:

```
$ git clone https://github.com/hex-five/multizone-linux
```

2. Build the MultiZone SDK for the U540:
```
$ make BOARD=U540
```

3. Load the MultiZone SDK image to flash (0x2000_0000):
```
$ make load BOARD=U540
```



## Demo

The MultiZone system contains four zones:

* Zone 1: Multizone demo console accessible via Linux device /dev/multizone1;
* Zone 2: Multizone demo console accessible via Linux device /dev/multizone2;
* Zone 3: Multizone demo console accessible via UART 115200/8/N/1 (expansion board);
* Zone 4: MultiZone Heartbeat LED D4 and message echo;


### Get the board ready

Set the mode select switches to '0001': 

```
      USB   LED    Mode Select                  Ethernet
 +===|___|==****==+-+-+-+-+-+-+=================|******|===+
 |                | | | |X|X|X|                 |      |   |
 |                | | | | | | |                 |      |   |
 |        HFXSEL->|X|X|X| | | |                 |______|   |
 |                +-+-+-+-+-+-+                            |
 |        RTCSEL-----/ 0 1 2 3 <--MSEL                     |
 |                                                         |
```

### Operate the Demo

1. Power the board.

2. Connect to the UART by setting the baudrate to 115200/8/N/1:

```
$ picocom /dev/<ttyUSB_DEVICE> -b 115200
```

> Note: <ttyUSB_DEVICE> shall be replaced with the correct UART device name (e.g, ttyUSB0, ttyUSB1). 

You should be able to see on the terminal the FSBL loading the BBL image:

```
SiFive FSBL:       2019-08-7-d8f6e2f
Using FSBL DTB
HiFive-U serial #: 00000068
Loading boot payload.........................
```

Once the BBL boots Linux, Linux starts executing:

```
bbl loader
...
```

3. Login to Linux using the root accout:

```
Welcome to Buildroot
buildroot login: root
Password: hexfive 
# 
```

4. Connect to Zone1 via Linux:

```
# screen /dev/multizone1
```

You shall be able to see and operate the terminal running on Zone1:

```
=====================================================================
      	           Hex Five MultiZone(TM) Security                   
    Copyright (C) 2018 Hex Five Security Inc. All Rights Reserved    
=====================================================================
This version of MultiZone(TM) is meant for evaluation purposes only. 
As such, use of this software is governed by your Evaluation License.
There may be other functional limitations as described in the        
evaluation kit documentation. The full version of the software does  
not have these restrictions.                                         
=====================================================================
Machine ISA   : 0x00101105 RV64 ACIMU 
Vendor        : 0x00000000  
Architecture  : 0x00000000  
Implementation: 0x00000000 
Hart ID       : 0x00000000 
CPU clock     : 1000 MHz 

Z1 > 
```

> Note: you may need to type 'restart' to see the full initial screen.


5. Connect to Zone2 via Linux using SSH:

```
$ ssh root@192.168.0.2
The authenticity of host '192.168.0.2 (192.168.0.2)' can't be established.
ECDSA key fingerprint is SHA256:5CMY+vBWX7VIyJ+ejpwtw9kojdYogIdBFPSJ9YmG33g.
Are you sure you want to continue connecting (yes/no)? yes
Warning: Permanently added '192.168.0.2' (ECDSA) to the list of known hosts.
root@192.168.0.2's password: hexfive
# screen /dev/multizone2
```

> Note: you may need to delete the old key before executing the SSH command.
> ```
> ssh-keygen -f "/home/<username>/.ssh/known_hosts" -R "192.168.0.2"
> ```


6. Operate the Zone2 console:

```
=====================================================================
      	           Hex Five MultiZone(TM) Security                   
    Copyright (C) 2018 Hex Five Security Inc. All Rights Reserved    
=====================================================================
This version of MultiZone(TM) is meant for evaluation purposes only. 
As such, use of this software is governed by your Evaluation License.
There may be other functional limitations as described in the        
evaluation kit documentation. The full version of the software does  
not have these restrictions.                                         
=====================================================================
Machine ISA   : 0x00101105 RV64 ACIMU 
Vendor        : 0x00000000  
Architecture  : 0x00000000  
Implementation: 0x00000000 
Hart ID       : 0x00000000 
CPU clock     : 1000 MHz 

Z2 >  pmp
0x20020000 0x2002FFFF r-x NAPOT 
0x08002000 0x08002FFF rw- NAPOT 

Z2 > 
```

7. In case the HiFive Unleashed U540 is connected to the Microchip / Microsemi Aloe Vera kit, it's possible to operate Zone3 via the UART (J36) available on the expansion board:

```
$ picocom /dev/<ttyUSB_DEVICE> -b 115200
```

> Note: <ttyUSB_DEVICE> shall be replaced with the correct UART device name (e.g, ttyUSB0, ttyUSB1). 

You shall be able to see and operate the terminal running on Zone3:

```
=====================================================================
      	           Hex Five MultiZone(TM) Security                   
    Copyright (C) 2018 Hex Five Security Inc. All Rights Reserved    
=====================================================================
This version of MultiZone(TM) is meant for evaluation purposes only. 
As such, use of this software is governed by your Evaluation License.
There may be other functional limitations as described in the        
evaluation kit documentation. The full version of the software does  
not have these restrictions.                                         
=====================================================================
Machine ISA   : 0x00101105 RV64 ACIMU 
Vendor        : 0x00000000  
Architecture  : 0x00000000  
Implementation: 0x00000000 
Hart ID       : 0x00000000 
CPU clock     : 1000 MHz 

Z3 > 
```

> Note:  
> Alternative way to run the demo via Ethernet (no UART connection required): 
>  
> Open a first SSH session as described above then connect to Zone1 via: 
> ```
># screen /dev/multizone1
> ```
> Open a second SSH sessions as described above then connect to Zone2 via:
> ```
> # screen /dev/multizone2
> ```
>
> To terminate screen sessions type: crtl-a k.

 
_MultiZone and HEX-Five are registered trademarks of Hex Five Security, Inc._

_MultiZone technology is protected by patents US 11,151,262 and PCT/US2019/038774_
