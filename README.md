## QModbusTool - A QT Based Modbus Client
QModbusTool is a multi-modal interface for interacting with Modbus registers on devices ranging from mid-range PLCs to smaller industrial end-devices that provide access through Modbus/TCP.  It aims to be simple and light weight while providing several key capabilities that are extremely helpful when commissioning and calibrating Modbus enabled devices including:

* **Live data trend**
* Save/Restore register sets
* Save/Restore session
* Support for loading register metadata using a simple plugin interface (see [PLUGIN.md][1]).

### License
Copyright (C) 2021 Andrew Buettner (ABi)

QModbusTool is released under the GPLv2 license.  See [LICENSE.md][2] for complete terms and conditions.

The metadata plugin header "[metadata.h][3]" is released under the 3-clause BSD license to allow equipment developers to create proprietary plugins specific to their equipment.

### Basic usage
Open the application and add 1 or more register sets to poll.  Each window represents a sequential block of registers that are polled with a single poll.  The number of registers presented can be polled is between 1 and the protocol maximum (125 for 16-bit analog values, 2000 for digital signals).  Polls can be directed to a specific "Slave ID", also known as an "Instance ID", "Device ID", or "Node".

The communication parameters may also be configured (remote device IP address and port).  The timeout is a local timeout to wait for a response.  Generally, Modbus/TCP does not implement a timeout in the way that it does on other transports such as UDP, RTU, or ASCII.  This is provided for recovery from Modbus/TCP devices and protocol gateways that don't handle Modbus timeouts correctly.  Alternatively, a previously saved session can be restored.

Optionally, a trend window can be created.  Using the available controls on the trend add one or more registers to be graphed.  These registers must be polled VIA another register window.  The trend will be updated once for each set of registers polled.

Once the communication parameters have been correctly configured and the desired windows have been created, select "Connect" from the "File" menu and the program will connect.  If the device connected to supports "Read Device ID" at address 0, the device name will briefly appear in the status bar section.  Once connected data may be polled either on request or automatically by selecting the appropriate option from the "Poll" menu.  If the meta data plug-in is available, the system may also poll register meta data from the the connected device.  The session may also be saved as can any window data and the trend.

### Building
QModbusTool was specifically designed for Linux, it should be reasonably easy to build under both Windows and macOS. However, the plugin interface has not been ported to these platforms. 
QModbusTool is built with qmake and requires a compiler that supports C++17 or newer.  It has been successfully built using both GCC and CLang.

### Prerequisites
* Qt 5+
	- += core gui xml widgets printsupport
* [LibModbus][4] (Modbus back-end)
* [QtCSV][5] (saving / restoring register and trend data)
* [QCustomPlot][6] (trending widget)
	- The build process expects this to be available as a library.  It does not ship with the amalgamation.
	- It is recommended to build with OpenGL support as it will utilize that functionality if available to improve redraw performance.

### Expanding
QModbusTool can easily have functionality expanded.  The base class for nearly all data-driven displays is defined in [base\_dialog.h][7]/.cpp.  This provides a bare-minimum interface needed to send and receive data from the scheduler.  The most important interfaces are:

* *`window_closed`* - Guaranteed to be emitted exactly once when the window is closed regardless of method.
* *`setupUi`* - Guaranteed to be called exactly once the very first time that the *show* event is called.
* *`window_first_display`* - Guaranteed to be emitted exactly once after the *startUi* function has been called.
* *`write_requested`* - may be emitted if the window requests to write registers.
* *`metadata_requested`* - may be emitted if the window requests to read metadata _(`set_metadata` must be implemented)_.
* *`on_new_value`* - received whenever new data is received or a scheduler/system event occurs.
* *`on_exception_status`* - received if a read, write, or metadata request results in a Modbus exception.

The appropriate signals and slots must be connected to the Scheduler in order for the window to receive Modbus events.

### Missing Features
While the application is mostly complete and should be usable for many applications, there are some notable items currently missing:

1. Modbus/RTU - I had to do some "hackey" stuff in order to handle custom modbus requests see [this issue][8].  This may not translate well to other flavors of the protocol.
2. Translations - All strings should be annotated.
3. Help documentation / about box

[1]: PLUGIN.md
[2]: LICENSE.md
[3]: metadata.h
[4]: https://libmodbus.org/
[5]: https://github.com/iamantony/qtcsv
[6]: https://www.qcustomplot.com/
[7]: base_data.h
[8]: https://github.com/stephane/libmodbus/issues/231
