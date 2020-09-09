================================================================================
Introduction
================================================================================

NOTE: this file is highly out-of-date. Your best reference is the source-code
and the provided example configuration at this point.

The configuration file for Cray PVP simulator controls almost all aspects
of the simulation, including the HW setup of the simulated machine as well
as runtime behavior such as logging or break-points.

The file contains a set of name-value pairs, where the value could potentially
be a full sub-hierarchy enclosed in { and } characters.

The hierarchy of the file closely follows the class-herarchy in the simulator,
each major class having its own configuration section.

The names and values are separated by white-space characters (space, tab, 
new-line etc.). There's no special separator between names and values, the role
of each word is simply determined by it's position. Apart from the hierarchy,
each odd word is a name and each even one is a value. Because of this, be 
careful when editing the file: a misplaced word can skew the parsing of the rest
of the file

To include values with spaces in them, they can be quoted using " characters.

Comments can be included, using the ; character: this starts a single-line 
comment that terminates at the end of the line obviously. There's no 
block-comment facility.

Parsing of the configuration file is rather loose: if the simulator encounters
a name that it doesn't know, it simply ignores it. Because of this, a mis-typed
name will likely go unnoticed so be careful.

An example of a simple name-value pair would be:
ClusterCount          5

A hierarchical setting would look something like this:
Console {
	TIChannelIdx 046
	TOChannelIdx 047
	Port 20003
	NewTerminalCommand "xterm -e {cmd}&"
	ConsoleCommand "wy50_con {host} {port} -h 40 -e wy50 -l {port}.log"
}

Sometimes a hierarchy is used to describe an arbitrary number of similar items,
like a list of breakpoints:
BreakPoints {
    0x210F { Type Trace Message "FATAL handler called {stack}" }
    0x326A { Type LogOn }
    0x3ba6 { Type LogOn }
}

You can also include other files, using a C-style include directive:

#include "some_other_file.cfg"

Relative file names are relative to the current working directory, not relative
to the file where the include is.

================================================================================
Types
================================================================================

The configuration file parser converts values into C++ types internally. This
conversion follows most of the regular rules, though there are a few exceptions.

Boolean
-------
Boolean values can take the value of: yes, true or 1 for 'true' values and
no, false or 0 for 'false' states.

Integral types (numbers)
------------------------
Numbers in general can be represented in any of the customary C++ notations:
1236 for decimal, 05436 for octal and 0x2345 for hexadeciamal values.

Addresses
---------
Addresses in general follow the same rules as integrals with one exception:
for mainframe memory addresses that can be interpreted as program addresses
a custom 'parcel addressing' format is supported. The Cray X-MP architecture
inherently used 64-bit addressing without the ability to address individual
parts of a 64-bit value. This is in contrast to todays machines that use
byte-addressing independent of their word-length. There was one exception
though: the X-MP used 16-bit instructions in most cases, so there was a need
for the program counter (P register) to be able to address 16-bit entities, or
the four 16-bit parcels that make up a 64-bit word. The original CAL notation
was to simply add two extra bits to the addresses of this kind, but that makes
it very cumbersome to see when a data and instruction address points to the
same location (doesn't happen too often in real code, but a very common
problem during reverse-engineering).

To help with this issue, the simulator supports a parcel-address format, where
the 64-bit part of the address is written as a normal hexadecimal number, and
the four possible parcels are identified using a :p0..3 suffix. So the four
parcels of address 0x345 are: 0x345:p0; 0x345:p1; 0x345:p2 and 0x345:p3.

Strings
-------
Strings are just that: string. If spaces are to be included in a string, they
can be quoted using the " character.

Log level
---------
The simulator supports very flexible logging settings and consequently many
parameters are dealing with setting log levels either at initialization time
or during simulation. These parameters use the following values:

	All: everything is logged
	SideEffects: instruction side-effects are logged
	InstructionTrace: instruction trace is logged
	Event: Events are logged
	IoTrace: Detailed I/O activity is logged
	IoActivity: I/O activity is logged
	Communication: IOP to mainframe communication is logged
	EventFire: Firing events are logged (strings from trace points)
	Interrupt: Interrupts are logged
	Warning: Warnings are logged
	Error: Errors are logged
	None: Nothing is logged

================================================================================
Global parameters
================================================================================

WindowWidth: optional integer
	If specified, sets the width of the main window (only applied if 
	WindowHeight is specified as well)
WindowHeight: optional integer
	If specified, sets the height of the main window (only applied if 
	WindowWidth is specified as well)
ConsoleHeight: optinal integer
	If specified, sets the height of the console window. Defaults to 10.
LogFileName: optional string.
	If specified, logging is going into the specified file.	If not specified,
	logging is going to stdout.
MultiThreaded: optional boolean
	If set to 'true', simulator uses multiple threads. Defaults to 'false'
EnableTimeStamp: optional boolean
	If set to 'true', events in the log file will contain a timestamp.
	Defaults to 'true'.
DefaultLogLevel: optional log level
	Sets the default log level for the logfile. Defaults to 'None'.
RealTimeClockIncrement: optional integer
	How much faster the real-time clock runs compared to simulated clock cycles.
	Defaults to 1000.
ProcessorAsyncLimit: optional integer
	How many clock-cycles processors or IOPs can get out of sync with each 
	other in a multi-threaded simulation. Defaults to 10000.
CpuIopScale: optional integer
	Scaling of IOP clock cycles before they are compared to mainframe processors
	to check for async limit. This allows more asynchronity between IOPs and
	CPUs then within CPUs or IOPs. Defaults to 2.
CpuMemorySize: optional integer
	Mainframe memory size in 64-bit words. Defaults to 8388608, which is 8M.
BufferMemorySize: optional integer
	IOP buffer memory size in 64-bit words. Defaults to 8388608, which is 8M.
ImageFiles: optional sub-hierarchy
	A list of files to be loaded into mainframe memory. Each entry is in the 
	form of: <load address> <file name>. Load address is specified in
	64-bit quantities
BufferImageFiles: optional sub-hierarchy
	A list of files to be loaded into IOP buffer memory. Each entry is in the 
	form of: <load address> <file name>. Load address is specified in 
	64-bit quantities
ClusterCount: optional integer
	Number of clusters in the mainframe. Defaults to 5. The number of clusters 
	depends on the simulated machine: for an XMP-1x or XMP-2x machine, it should
	be 3. For XMP-4x machines, set it to 5.
StartupCpuIdx: optional integer
	The index of the mainframe CPU to start first. Defaults to 0.
StartupIopIdx: optional integer
	The index of the IOP to start first. Defaults to 0.
MemoryDumpFile: optional string
	If set, specifies the mainframe memory dump file. The contents of the 
	mainframe memory is dumped into this file when simulation terminates
BufferMemoryDumpFile: optional string
	If set, specifies the IOP buffer memory dump file. The contents of the IOP 
	buffer memory is dumped into this file when simulation terminates
OsType: optional string
	If set, specifies the way exchange packets get interpreted for logging.
	Setting this value has no impact on simulation, only on reporting in
	log files. Possible values:
	- None: no interpretation
	- COS: interpret exchange packets for COS system calls
	- UNICOS: interpret exchange packets for UNICOS system calls
	
================================================================================
Cpus
================================================================================

Each mainframe processor has its associated configuration.

DefaultLogLevel: optional loglevel
	If set, determines the default log level for the CPU
InstructionBurstSize: optional integer
	Sets the maximum number of instructions that a CPU can execute in a burst.
	If not specified, the default value is 1.
TimerIncrement: optional integer
	Each CPU has a real-time clock that generates an interrupt when it rolls
	over. This value sets the value this timer gets incremented by for every
	simulated clock-cycle. The default value is 10, which makes the simulated
	real-time clock 10x faster then the original HW one would be.
MemoryPokes: list of address-value pairs
	This list contains a set of values that are going to be written into 
	mainframe memory whenever the particular processor is released from reset.
	Address can be either a 64-bit word address (like 0x425432) or a 16-bit
	parcel address (like 0x42346:p2). For word addresses, the value is a 64-bit
	integer, for parcel-address the value is a 16-bit integer

================================================================================
BreakPoints
================================================================================

For the mainframe, a setup of breakpoints can be specified. Various actions can
be taken by the simulator when execution passes through these addresses by any
of the main CPUs. Multiple breakpoints can be specified for the same address.

Each breakpoint is specified with their address as the name of the section and
various parameters in the body of the hierarchy. So for example a simply
breakpoint can be specified such: 0x345f:p2 { Type: Dump }

For CPU break points, the address is specified in parcel address format
(ex. 0x536:p2). The type of the breakpoint is specified using the 'Type'
parameter. 

For mainframe breakpoints support the following qualifiers:
	AbsoluteAddr	  when specified, denotes the absolute address for the breakpoint.
			  This is important for user-mode breakpoints which can have
			  many aliases in different processes. The absolute address
			  is usually more unique. The value is specified as a program
			  address.
	TriggerCnt	  an optional integer that specifies an pass-count before
			  the breakpoint fires.

Type: string
	Trace		  when fires, puts the specified message into the log file
			  The trace message is specified using the 'Message' parameter
	CpuDump	  when fires, dumps the CPU state into the log file
	Break		  when fires, breaks the simulator into the debugger
			  (for debugging the simulator that is)
	Dump		  when fires dumps the mainframe state, including its memories.
	Terminate	  when fires, terminates the simulation by raising an exception
	LogOn		  when fires, sets log level to 'all' on the CPU that triggered
			  the breakpoint
	LogOff	  when fires, sets log level to 'none' on the CPU that triggered
			  the breakpoint
	LogLevel	  when fires, sets the log level to the defined level on the CPU
			  that triggered the breakpoint. The new log level is set using 
			  the 'Level' parameter
	LogLevelPush  When fires, sets the log level to the defined level as well as
			  push the current log level into the log level stack on the CPU
			  that triggered the breakpoint. The new log level is set using
			  the 'Level' parameter
	LogLevelPop	  When fires, pops the log level from the log level stack on the
			  CPU that triggered the breakpoint
	Event		  When fires, triggers an event. The event string is specified
			  using the 'Event' parameter. The special string {cpu} gets
			  repleced with the name of the CPU that triggered the breakpoint

================================================================================
WatchPoints
================================================================================

Watchpoints fire events when a CPU accesses a certain memory location.
The format is: <addr> <message>

The address is specified 64-bit word addresses. Instruction fetches are not
checked against watchpoints (use breakpoints to achieve similar behavior).

When fired, the event generated is of the following format:
	"Watchpoint at address <Addr> hit #:<HitCnt> <Message>"

This event then can be used to trigger other actions in the simulator, like
changing log levels in various parts.

Since every memory access is checked against all wathcpoints they are expensive
and slow simulation down significantly.

================================================================================
EventPoints
================================================================================

Events are generated by various components of the simulator when certain things
happen. Most events are hard-coded (for example when a sector is read or written
from one of the virtual hard drives) but events can be generated using
event-type breakpoints when execution reaches a certain address or by
watchpoints when a CPU loads or stores to a certain address. Every time an event
is generated, event points are checked for a match. If they do match, the event
point fires, executing its associated action.

Events are essentially string. Consequently event points match the string using
regular expressions. This allows for flexible matching but also slows simulation
down significantly if a lot of event are generated and there are a lot of event
points to match each time. Event points are defined using the following format:

<event regex> {
	Type <break point type>
}

The type could be anything that's a valid breakpoint.

================================================================================
CpuMemoryPokes
================================================================================

Very similar to per-CPU memory pokes, a global list of memory overrides can be
provided. The format is the same (<address> <value>) as for per-CPU pokes, but
this list is executed only once, when the mainframe first comes out of reset.
This usually happens during boot, when usually only a very small test program is
loaded (called MFINIT). Because of this, unless this initial test is patched up
to be skipped, this feature is not terribly useful.

================================================================================
BufferMemoryPokes
================================================================================

The buffer memory is a large, shared piece of memory used by the IOPs to store
all sorts of information, that doesn't fit in their operating memory. Most
importantly, all the IOP overlay modules are loaded here.

Just as for other memory types a list of writes (using <address> <value> pairs)
can be specified. Because this memory connects to the IO subsystem, which uses
16-bit addresses, the addresses here are 16-bit word addresses and the values
are 16-bit quantities as well.

Buffer memory pokes are executed right at the beginning of the simulation after
the memory it is initialized from the files listed in BufferImageFiles.
	
================================================================================
Iops
================================================================================

The X-MP line of machines contained up to four IO processors, or IOPs. The
configuration file contains a section for all four IO processors. These are
numbered consecutively from 0:

Iops {
	0 {}
	1 {}
	2 {}
	3 {}
}

All four sections must exist even if the configuration only contains fewer IO
processors. IO processors that are not installed in the system can be marked
with the 'Exists' key with the 'no' value.

The available set of configuration options are identical for all four IOPs:

Exists: optional boolean
	Defines if a given IOP exists in the system. Defaults to 'true'. This value
	is mostly used to speed up simulation.
DefaultLogLevel: optional loglevel
	Sets the default log level for messages coming from the current IOP
	processor subsystem (the processor or one of its peripherals). If not
	specified, the value is inherited from the global log level.
Type: string
	Specifies the type of IOP. Possible values are 'MIOP', 'BIOP', 'DIOP' and
	'XIOP'. The value helps the instruction trace logging to print meaningful
	I/O function comments.
ChannelCount: optional integer
	Specifies the number of I/O channels in the IOP. Defaults to 42.
InstructionBurstSize: optional integer
	Maximum number of instructions to execute in a single burst. Defaults to 1.
MemoryDumpFile: optional string
	If set, specifies the IOP local memory dump file. The contents of the IOP
	local memory is dumped into this file when simulation terminates

Channels
========
Each IOP has a set of peripherals, called channels or I/O channels. These are
set up in the channels list. Each peripheral has a set of parameters, specific
to the peripheral. The common format of a peripheral section is:

<peripheral name> {
	<parameter> <value>
	...
}

The channel associated with a peripheral is commonly set by the ChannelIdx
parameter.

The following peripherals are implemented:

ERA
---
This is an error-reporting peripheral that in the original hardware is used to
monitor various - memory-related - correctable and uncorrectable errors. In
simulation it's a very simple stub that never reports any errors. The only
parameter available for ERA channels is the ChannelIdx setting.

ChannelIdx: integer
	Sets the IOP channel this peripheral connects to

Console
-------
Consoles in the original HW are simple serial terminals. They are implemented by
a pair of channel interfaces (TI and TO). In the simulator they are set up as a
single entity and provide a TCP based interface to the actual terminal. The
following parameters can be set for console peripherals:

TIChannelIdx: integer
	Sets the IOP input channel this peripheral connects to
TOChannelIdx: integer
	Sets the IOP output channel this peripheral connects to
Port: integer
	The TCP port to be opened by this console
ConsoleCommand: optional string
	Optional command to execute when a character is written to the console
	while no terminal is attached. This feature can be used to automatically
	launch a terminal emulator to display the console. Some special values are
	replaced in the command:
	{host} gets replaced with the IP address of the machine the simulator is
	       running on
	{port} gets replaced with the listening port for the console
NewTerminalCommand - optional string
	Optional value, used on Linux targets. If specified, contains a command line
	to open a new terminal. The special value {cmd} gets replaced with the
	actual command to be run in that terminal.

HIA/HOA
-------
These channels implement high-speed data transfers between the IOP and the
mainframe memory. They are DMA controllers in modern terms. The only
configurable parameter for them is their channel index:
	
ChannelIdx - integer
	Sets the IOP channel this peripheral connects to

Expander
--------
The peripheral expander allowed the X-MP mainframes to connect to Data-general
compatible equipment. On most machines these peripherals were used for the
maintenance console and consisted of a hard drive, a tape drive a printer and
potentially a real-time clock. The peripheral expander itself however implements
several 'sub-channels' which can be connected to different peripherals. The
configuration file follows this hierarchy: after the main set of parameters to
configure the expander itself a sub-section is used to set up each of the
attached peripherals. The Expander parameters are:

ChannelIdx - integer
	Sets the IOP channel this peripheral connects to
DeviceCount - optional integer
	Sets the number of expander device channels. Defaults to 64.
UseDummyDevices - optional boolean
	If specified and set true, all unspecified devices are replaced by dummy
	devices.

The following devices can be specified:

Tape
~~~~
Specifies a tape drive.

ChannelIdx - integer
	Sets the expander channel this peripheral connects to
Interrupt - integer
	Specifies the expander interrupt index for this peripheral
Tape - optional string
	Specifies the name of the initial TAP file to mount
ReadOnly - optional boolean
	Optionally setting the tape as read only. This properly will be reflected in
	the 'write-ring removed' bit in the emulated HW
Pokes - optional
	A collection of pokes, applied to each TAP file as they are read in. For
	each file, the general structure is the following:

	<tap file name> {
		<file idx> {
			<offset> {
			}
		}
	}

	The offsets are relative to the file within the TAP file. For each specified
	offset, the following parameters can be specified

	Size - optional integer
		Sets the size of the value in bytes. Possible values are: 1,2,4 or 8.
		The default value is 1.
	Value - integer
		Sets the value to be read at the given offset
	BigEndien - optional boolean
		Species the endienness of the value. Default is 'true'.

AmpexDM980Disk
~~~~~~~~~~~~~~
These were 80MB disk drives, but the X-MP IOS kernel formatted them to 64MB
useful capacity. They contained a file system (which can be created by the
exp_disk_create utility). The files on the disk could be transferred to the
mainframe for execution. Depending on the IOS configuration either on of these,
or a CDC9448 cartridge disk drive was installed in the system. The parameters
to set this peripheral up are:

ChannelIdx - integer
	Sets the expander channel this peripheral connects to
Interrupt - integer
	Specifies the expander interrupt index for this peripheral
ImageFileName - string
	Specifies the image file representing the hard drive
Heads - optional integer
	Sets the number of simulated heads in the hard drive. Defaults to 5
Sectors - optional integer
	Sets the number of simulated sectors per track in the hard drive.
	Defaults to 35.
Tracks - optional intger
	Sets the number of simulated tracks in the hard drive. Defaults to 823
SectorSize - optional integer
	Sets the size of the sector in bytes in the simulated hard drive.
	Defaults to 512.

CDC9448Disk
~~~~~~~~~~~
These were 80MB cartridge disk drives, but the X-MP IOS kernel formatted them to
64MB useful capacity. They contained a file system (which can be created by the
exp_disk_create utility). The files on the disk could be transferred to the
mainframe for execution. Depending on the IOS configuration either on of these,
or an Ampex DM980 disk drive was installed in the system. The parameters to set
this peripheral up are:

ChannelIdx - integer
	Sets the expander channel this peripheral connects to
Interrupt - integer
	Specifies the expander interrupt index for this peripheral
ImageFileName - string
	Specifies the image file representing the hard drive
Heads - optional integer
	Sets the number of simulated heads in the hard drive. Defaults to 5
Sectors - optional integer
	Sets the number of simulated sectors per track in the hard drive.
	Defaults to 35.
Tracks - optional intger
	Sets the number of simulated tracks in the hard drive. Defaults to 823
SectorSize - optional integer
	Sets the size of the sector in bytes in the simulated hard drive.
	Defaults to 512.

Printer
~~~~~~~
A simulated line printer. The following configuration parameters are valid:

ChannelIdx - integer
	Sets the expander channel this peripheral connects to
Interrupt - integer
	Specifies the expander interrupt index for this peripheral
Name - string
	Name of the printer device, used for logging purposes
PrintFileName - string
	Sets the output file of the simulated printer. Any printed character will
	get appended to this file.

Clock
~~~~~
Some systems had a real-time clock installed. There were two choices, a
Chronolog or a Hayes clock. This peripheral simulation implement the Hayes one.
The following parameters can be used with it:

PrimaryChannelIdx - integer
	Sets the primary expander channel this peripheral connects to
RequestChannelIdx - optional integer
	Sets the request expander channel this peripheral connects to. If not
	specified, set to PrimaryChannelIdx+1
ResponseChannelIdx - optional integer
	Sets the response expander channel this peripheral connects to. If not
	specified, set to  PrimaryChannelIdx
Interrupt - integer
	Specifies the expander interrupt index for this peripheral
ResponseTimeout - optional integer
	Sets the frequency of the interrupts generated by the clock. If not
	specified, defaults to 100, which is once every 100 simulated clock cycles
Name - string
	Name of the clock device, used for logging purposes
YearOffset - optional integer
	Year correction in date. COS releases were not Y2K compliant so this allows
	the simulator to lie to COS about the year, yet keep the month and day
	accurate. Default value is 20

DD29
----
DD29s were the mainframe hard disks. They had about 600MB of formatted capacity.
Originally up to four of these drives were connected to a controller, but this
aspect of the machine is not simulated: each DD29 is treated independently. The
simulation uses disk image files for each disk drive in the system. The
following are the valid parameters:

ChannelIdx - integer
	Sets the IOP channel this peripheral connects to
ImageFileName - string
	Specifies the image file representing the hard drive
ReadOnly - optional boolean
	Optional setting to disallow writes to the disk image. If set to true, any
	write attempt will terminate the simulation.
Heads - optional integer
	Sets the number of simulated heads in the hard drive. Defaults to 10.
Sectors - optional integer
	Sets the number of simulated sectors per track in the hard drive.
	Defaults to 18
Tracks - optional integer
	Sets the number of simulated tracks in the hard drive. Defaults to 823
SectorSize - optional integer
	Sets the size of the sector in bytes in the simulated hard drive.
	Defaults to 4096.

Bmx
---
Block Multiplexer channels allowed Cray computers to connect to IBM compatible
peripherals (mostly tape drives). Similarly to the expander peripheral, here you
can specify the peripherals attached to each BMX channel with one section each,
using the 'Devices' section:

	Devices {
		Tape {}
		Tape {}
	}

Note, that at the moment only 'Tape' peripherals are implemented and even their
simulation is rather rudimentary due to the lack of test cases I could find.

The parameters of every BMX channel are the following:

ChannelIdx - integer
	Sets the IOP channel this peripheral connects to
DelayLimit - optional integer
	Sets the number simulated clock cycles the delay timer takes to expire. The
	delay counter is a test operation in the BMX channel hardware.
	Default value is 10.
DeviceCount - optional integer
	Maximum number of devices that can be connected to this BMX channel. 
	Default value is 255

For each tape device the following parameters are valid:

DeviceAddress - integer
	The BMX address this device is connected to
DeviceName - optional string
	Optionally specifies the name of the device. If not specified, name is
	derived from DeviceAddress
ReadOnly - optional boolean
	If set to 'true', sets the tape as read only.
Tape - string
	Specifies the name of the initial TAP file to mount
Pokes - optional
	A collection of pokes, applied to each TAP file as they are read in. For
	each file, the general structure is the following:

	<tap file name> {
		<file idx> {
			<offset> {
			}
		}
	}

	The offsets are relative to the file within the TAP file. For each specified
	offset, the following parameters can be specified

	Size - optional integer
		Sets the size of the value in bytes. Possible values are: 1,2,4 or 8.
		The default value is 1.
	Value - integer
		Sets the value to be read at the given offset
	BigEndien - optional boolean
		Species the endienness of the value. Default is 'true'.

CI/CO
-----
6MBps, low-speed communication channels between an IOP and the mainframe. The
following parameters can be specified for these channels:

IopChannelIdx - integer
	Sets the IOP channel this peripheral connects to
CrayChannelIdx - integer
	Sets the mainframe channel this peripheral connects to
CONC
----
A rather incomplete concentrator (or front-end interface) channel. These
peripherals were the equivalent of network interfaces in these old machines. The
parameters that can be specified are the following:

InputChannelIdx - integer
	Sets the input IOP channel this peripheral connects to
OutputChannelIdx - integer
	Sets the output IOP channel this peripheral connects to
SID - optional string
	Specifies the station ID. If not specified, defaults to "FE" as in Front
	End.

BreakPoints
===========
There are several types of break points supported by the IOPs. Each breakpoint
is associated with an address and potentially fires when execution passes that
address. The difference between the breakpoint types is the type of action that
takes place when the break-point fires. Multiple breakpoints can be specified
for the same address.

Each breakpoint is specified with their address as the name of the section and
various parameters in the body of the hierarchy. So for example a simply
breakpoint can be specified such: 0x345f { Type: Dump }

For IOP break points, the address is specified in 16-bit address format
(ex. 0x5362) as these processors used a simple 16-bit instruction set and
address bus width. The type of the breakpoint is specified using the 'Type'
parameter. 

Type - string
	Trace		  when fires, puts the specified message into the log file
			  The trace message is specified using the 'Message' parameter
	CpuDump	  when fires, dumps the CPU state into the log file
	Break		  when fires, breaks the simulator into the debugger
			  (for debugging the simulator that is)
	Dump		  when fires dumps the mainframe state, including its memories.
	Terminate	  when fires, terminates the simulation by raising an exception
	LogOn		  when fires, sets log level to 'all' on the CPU that triggered
			  the breakpoint
	LogOff	  when fires, sets log level to 'none' on the CPU that triggered
			  the breakpoint
	LogLevel	  when fires, sets the log level to the defined level on the CPU
			  that triggered the breakpoint. The new log level is set using 
			  the 'Level' parameter
	LogLevelPush  When fires, sets the log level to the defined level as well as
			  push the current log level into the log level stack on the CPU
			  that triggered the breakpoint. The new log level is set using
			  the 'Level' parameter
	LogLevelPop	  When fires, pops the log level from the log level stack on the
			  CPU that triggered the breakpoint
	Event		  When fires, triggers an event. The event string is specified
			  using the 'Event' parameter. The special string {cpu} gets
			  repleced with the name of the CPU that triggered the breakpoint


DataConditions - optional section
	A collection of data conditions to be met for this breakpoint to fire. Data
	conditions are specified in the format of: <address> <value> where both are
	16-bit unsigned integer values

TriggerCnt - optional integer
	Specifies the number of passes through this breakpoint before it starts
	firing. If data conditions are specified, those will have to match as well
	for a pass to be counted. Once the pass count reaches the trigger count,
	every subsequent pass will fire the breakpoint. The default value is 0,
	which makes the break point fire on every pass.

EventPoints
===========
Events are generated by various components of the simulator when certain things
happen. Most events are hard-coded (for example when a sector is read or written
from one of the virtual hard drives) but events can be generated using
event-type breakpoints when execution reaches a certain address or by
watchpoints when a CPU loads or stores to a certain address. Every time an event
is generated, event points are checked for a match. If they do match, the event
point fires, executing its associated action.

Events are essentially string. Consequently event points match the string using
regular expressions. This allows for flexible matching but also slows simulation
down significantly if a lot of event are generated and there are a lot of event
points to match each time. Event points are defined using the following format:

<event regex> {
	Type <break point type>
}

The type could be anything that's a valid breakpoint. The following optional
parameters can also be used with event-points to further qualify their
operation:

DataConditions - optional section
	A collection of data conditions to be met for this event point to fire. Data
	conditions are specified in the format of: <address> <value> where both are
	16-bit unsigned integer values

TriggerCnt - optional integer
	Specifies the number of passes through this event point before it starts
	firing. If data conditions are specified, those will have to match as well
	for a pass to be counted. Once the pass count reaches the trigger count,
	every subsequent pass will fire the event point. The default value is 0,
	which makes the break point fire on every pass.
