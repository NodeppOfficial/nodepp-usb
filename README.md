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
        console::log( "->", x.get_endpoint( 
            ENDPOINT_IN, TRANSFER_CONTROL 
        )); 

        console::log( "->", x.get_manufacturer() );
        console::log( "->", x.get_product() );

        console::log("---");
    }

}
```

## Compilation
```bash
g++ -o main main.cpp -I./include -lusb-1.0 ; ./main
```
