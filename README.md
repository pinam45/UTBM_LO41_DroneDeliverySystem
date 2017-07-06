# UTBM_LO41_DroneDeliverySystem

## Information

### School project

- University: [UTBM](http://www.utbm.fr/)
- Unit value: LO41 (Operating system: Principles and Communication)
- Semester: Spring 2017

### Authors / students

- [Jérôme Boulmier](https://github.com/Lomadriel)
- [Maxime Pinard](https://github.com/pinam45)

### Subject

The aim is to made a drone delivery system simulation with C libraries from Linux (System V and POSIX).

A mothership send drones to deliver packages to clients within the delivery range.

### Realisation

In this simulation we chose to use POSIX threads, messages queues and mutexes. The mothership and each drone and client has its own thread, the communication is made with the messages queues and the mutexes avoid concurrent accesses.

The console UI was made with [ConsoleControl](https://github.com/pinam45/ConsoleControl).

For more information about the realisation, see the french report (made with LaTeX) in the [report-fr](report-fr) folder.

## Content

- **cmake/**: CMake related files
- **ConsoleControl/**: ConsoleControl library submodule
- **DroneDeliverySystem/**: Project sources
- **report-fr/**: French report sources
- **clientsX.csv** / **dronesX.csv** / **packagesX.csv**: Test configuration files
- **LisezMoi.md**: French README

## Compilation

### CMake

For more information about CMake see [the CMake website](https://cmake.org/). Common CMake use:

	$ mkdir build
	$ cd build
	$ cmake ..
	$ make

### Make

For more information about Make see [the GNU website](https://www.gnu.org/software/make/). To compile, use the default target with:

	$ make

For information about the other targets, use:

	$ make help

## Execution

To launch the program with the default configuration files:

	$ ./build/bin/UTBM\_LO41\_DroneDeliverySystem.elf

To choose the configuration files:

	$ ./build/bin/UTBM_LO41_DroneDeliverySystem.elf PACKAGES_FILE CLIENT_FILE DRONE_FILE

## Copyright

This work is under the MIT License

[Read the license file](LICENSE)
