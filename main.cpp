#include <nodepp/nodepp.h>
#include <usb/usb.h>

using namespace nodepp;

void onMain() {

    usb_t ctx; auto devices = ctx.get_devices();
    for( auto x: devices ){
        console::log( "->", x.get_product() );
    }

}