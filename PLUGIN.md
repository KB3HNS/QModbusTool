## QModbusTool - A QT Based Modbus Client
The application has the ability to query the connected device for certain metadata items that pertain to a register including:

* Register minimum and maximum values
* Register default value
* Register encoding (signed, unsigned, bits, bytes, etc)
* A brief text string describing the register

Some manufactures provide the ability to read some, or all of this data through the use of non-standard or application specific Modbus function codes.  Modbus its self (unlike other industrial protocols) does not provide a standard mechanism for accessing this type of data.  Because in many cases this functionality is greatly helpful some manufacturers have implemented their own proprietary methods for accessing this type of information.
QModbusTool was written to allow manufacturer-defined meta data querying implementations.

To make the interface as versatile as possible the interface has been broken up into 5 sections:

1. Create request instance for a register.
2. Encode the outgoing request.
3. Decode the incoming response.
4. Extract various sections returning optional responses if specific element is unsupported.
5. Dispose of the previously created instance.

### Getting started
Review the available documentation in the header "[metadata.h][1]" for interface specification.  The application expects the file called "mod\_plugin.so" to be located in the same directory as the application its self.  This behavior can be modified in `metadata_wrapper.cpp`.
The plugin is a singleton wrapper that is initialized during start-up shortly before rendering the main window.  The poll scheduling logic has a special priority dedicated to just retrieving register metadata.  

[1]: metadata.h
