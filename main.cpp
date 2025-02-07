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