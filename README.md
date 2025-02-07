# NODEPP-USB
Run USB in Nodepp

## Dependencies
```bash
#libub-dev
🪟: pacman -S mingw-w64-x86_64-libusb
🐧: sudo apt install libusb-1.0-0-dev
```

## Example
```cpp
#include <nodepp/nodepp.h>
#include <usb/usb.h>

using namespace nodepp;

void onMain() {

    usb_t ctx; auto devices = ctx.get_devices();
    for( auto x: devices ){

        ptr_t<uchar> bff ( 64, '\0' ); int len=0;
    
        while( (len=x.control_read( 0x21, 0x09, 0x0300, 0x0000, bff ))==-2 )
             { process::next(); }

        console::log( "->", x.get_manufacturer() );
        console::log( "->", x.get_product() );
        console::log( "->", len );

        console::log("---");
    }

}
```

## Compilation
```bash
g++ -o main main.cpp -I./include -lusb-1.0 ; ./main
```
