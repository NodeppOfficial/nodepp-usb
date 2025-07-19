/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_USB
#define NODEPP_USB

/*────────────────────────────────────────────────────────────────────────────*/

#include <libusb-1.0/libusb.h>
#include <nodepp/nodepp.h>

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp {
enum usb_request_type {
	REQUEST_STANDARD = (0x00 << 5),
	REQUEST_CLASS    = (0x01 << 5),
	REQUEST_VENDOR   = (0x02 << 5),
	REQUEST_RESERVED = (0x03 << 5)
};

enum usb_request_recipient {
	RECIPIENT_DEVICE    = 0x00,
	RECIPIENT_INTERFACE = 0x01,
	RECIPIENT_ENDPOINT  = 0x02,
	RECIPIENT_OTHER     = 0x03
};

enum usb_transfer_type {
	TRANSFER_CONTROL     = 0,
	TRANSFER_ISOCHRONOUS = 1,
	TRANSFER_BULK        = 2,
	TRANSFER_INTERRUPT   = 3
};

enum usb_endpoint_direction {
	ENDPOINT_IN  = 0x80,
	ENDPOINT_OUT = 0x00
};
}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class usb_device_t {
protected:

    struct NODE {
        libusb_device_handle* fd = nullptr;
        libusb_device       *ctx = nullptr;
        libusb_context      *idx = nullptr;
        bool state = 0;
    };  ptr_t<NODE> obj;

    /*.........................................................................*/

    bool is_blocked( int c ) const noexcept { if( c < 0 ){ return (
        c == LIBUSB_ERROR_TIMEOUT || 
        c == LIBUSB_ERROR_BUSY
    );} return 0; }

    /*.........................................................................*/

    int control_transfer( uint8 type, uint8 id, uint16 value, uint16 index, uchar* buffer, ulong size ) const noexcept {
        if( obj->state==0 ){ return -1; } int len = 0;
        if( is_blocked( len=libusb_control_transfer ( 
             obj->fd, type, id, value, index, buffer, size, 0 
       ))){ return -2; } return len < 0 ? -1 : len;
    }

    int bulk_transfer( uchar* buffer, ulong size, ptr_t<uint8> ctx ) const noexcept {
        if( obj->state==0 || ctx == nullptr ){ return -1; } int len = 0;

        if( libusb_claim_interface( obj->fd, ctx[3] )<0 ) { goto DONE; }
        if( is_blocked( libusb_interrupt_transfer ( 
             obj->fd, ctx[0], buffer, size, &len, 0 
       ))){ libusb_release_interface( obj->fd, ctx[3] ); return -2; }

        DONE:; libusb_release_interface( obj->fd, ctx[3] ); return len;
    }

    int interrupt_transfer( uchar* buffer, ulong size, ptr_t<uint8> ctx ) const noexcept {
        if( obj->state==0 || ctx == nullptr ){ return -1; } int len = 0;

        if( libusb_claim_interface( obj->fd, ctx[3] )<0 ) { goto DONE; }
        if( is_blocked( libusb_interrupt_transfer ( 
             obj->fd, ctx[0], buffer, size, &len, 0 
       ))){ libusb_release_interface( obj->fd, ctx[3] ); return -2; }

        DONE:; libusb_release_interface( obj->fd, ctx[3] ); return len;
    }

public:

    usb_device_t ( uint16 vendorID, uint16 productID ) : obj( new NODE() ) { libusb_init( &obj->idx );
        obj->fd = libusb_open_device_with_vid_pid( obj->idx, vendorID, productID );
        if( obj->fd == nullptr ){ 
            throw except_t( "can't initialize device" ); 
        return; } obj->state = 1;
    }

    usb_device_t ( libusb_device* ctx ) : obj( new NODE() ) {
        if( ctx == nullptr ){ return; } if( libusb_open( ctx, &obj->fd )<0 ){ 
            throw except_t( "can't initialize device" ); 
        return; } obj->state = 1; obj->ctx = ctx;
    }

    /*.........................................................................*/

   ~usb_device_t () noexcept { if( obj.count()>1 ){ return; } free(); }
    usb_device_t () noexcept : obj( new NODE() ){}

    /*.........................................................................*/

    ptr_t<uint8> get_endpoint( uint8 _type_, uint8 _mode_ ) const noexcept {
        libusb_config_descriptor *config_descriptor = NULL;

        if( libusb_get_active_config_descriptor( obj->ctx, &config_descriptor )<0 ) 
          { goto DONE; }

        for( int j=0; j<config_descriptor->bNumInterfaces; j++ ) {
             auto interface_descriptor = &config_descriptor->interface[j];

        for( int k=0; k<interface_descriptor->num_altsetting; k++ ) {
             auto alt_descriptor = &interface_descriptor->altsetting[k];

        for( int l=0; l<alt_descriptor->bNumEndpoints; l++ ) {
             auto endpoint_descriptor = &alt_descriptor->endpoint[l];

            if ( endpoint_descriptor->bEndpointAddress & _type_ &&
                 endpoint_descriptor->bmAttributes     & _mode_
            )  { continue; }

            uint8 endpoint_address   = endpoint_descriptor->bEndpointAddress;
            uint8 endpoint_direction = endpoint_descriptor->bEndpointAddress & _type_;
            uint8 endpoint_type      = endpoint_descriptor->bmAttributes     & _mode_;

            return ptr_t<uint8>({ 
                endpoint_address  ,
                endpoint_type     ,
                endpoint_direction, (uint8)j
            });

        }}}

        DONE:; libusb_free_config_descriptor( config_descriptor ); return nullptr;
    }

    /*.........................................................................*/

    libusb_device_descriptor get_descriptor() const noexcept {
        libusb_device_descriptor idx;
        libusb_get_device_descriptor( obj->ctx, &idx ); return idx;
    }

    /*.........................................................................*/

    uint8 get_vendor_id()  const noexcept { return get_descriptor().idVendor; }
    uint8 get_product_id() const noexcept { return get_descriptor().idProduct; }
    uint8 get_serial_id()  const noexcept { return get_descriptor().iSerialNumber; } 

    /*.........................................................................*/

    string_t get_product() const noexcept { 
        uchar buff[256]; memset( buff, 0, 256 ); auto idx = get_descriptor();
        auto len = libusb_get_string_descriptor_ascii( obj->fd, idx.iProduct, buff, 256 );
        if ( len<= 0 ) { return nullptr; } return string_t( (char*) buff, len );
    }

    string_t get_manufacturer() const noexcept { 
        uchar buff[256]; memset( buff, 0, 256 ); auto idx = get_descriptor();
        auto len = libusb_get_string_descriptor_ascii( obj->fd, idx.iManufacturer, buff, 256 );
        if ( len<= 0 ) { return nullptr; } return string_t( (char*) buff, len );
    }

    string_t get_serial() const noexcept { 
        uchar buff[256]; memset( buff, 0, 256 ); auto idx = get_descriptor();
        auto len = libusb_get_string_descriptor_ascii( obj->fd, idx.iSerialNumber, buff, 256 );
        if ( len<= 0 ) { return nullptr; } return string_t( (char*) buff, len );
    }

    /*.........................................................................*/

    int control_read( uint8 id, uint8 _type_, uint16 value, uint16 index, ptr_t<uchar> data ) const noexcept { 
        auto ctx = get_endpoint( TRANSFER_CONTROL, ENDPOINT_IN );
        return control_transfer( _type_|ENDPOINT_IN, id, value, index, data.get(), data.size() );
    }

    int control_write( uint8 id, uint8 _type_, uint16 value, uint16 index, ptr_t<uchar> data ) const noexcept { 
        auto ctx = get_endpoint( TRANSFER_CONTROL, ENDPOINT_OUT );
        return control_transfer( _type_|ENDPOINT_OUT, id, value, index, data.get(), data.size() );
    }

    /*.........................................................................*/

    int interrupt_read( ptr_t<uchar> data ) const noexcept { 
        auto ctx = get_endpoint( TRANSFER_INTERRUPT, ENDPOINT_IN );
        return interrupt_transfer( data.get(), data.size(), ctx );
    }

    int interrupt_write( ptr_t<uchar> data ) const noexcept {
        auto ctx = get_endpoint( TRANSFER_INTERRUPT, ENDPOINT_OUT );
        return interrupt_transfer( data.get(), data.size(), ctx );
    }

    /*.........................................................................*/

    int bulk_read( ptr_t<uchar> data ) const noexcept {
        auto ctx = get_endpoint( TRANSFER_BULK, ENDPOINT_IN );
        return bulk_transfer( data.get(), data.size(), ctx );
    }

    int bulk_write( ptr_t<uchar> data ) const noexcept {
        auto ctx = get_endpoint( TRANSFER_BULK, ENDPOINT_OUT );
        return bulk_transfer( data.get(), data.size(), ctx );
    }

    /*.........................................................................*/

    void free() const noexcept {
        if( obj->state == 0 )    { return; } obj->state =0;
        if( obj->idx != nullptr ){ libusb_exit( obj->idx ); }
        if( obj->fd  != nullptr ){ libusb_close( obj->fd ); } 
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class usb_t {
protected:

    struct NODE {
        libusb_context* ctx  = nullptr;
        libusb_device** list = nullptr;
        bool state = 0;
    };  ptr_t<NODE> obj;

    int count() const noexcept { 
        if( obj->state == 0 ){ return -1; }
        return libusb_get_device_list( obj->ctx, &obj->list ); 
    }

public:

   ~usb_t () noexcept { if( obj.count()>1 ){ return; } free(); }

    usb_t () : obj( new NODE() ) { if( libusb_init( &obj->ctx ) < 0 ) {
        throw except_t("Can't initialize USB"); return;
    }   obj->state = 1; }

    /*.........................................................................*/

    array_t<usb_device_t> get_devices() const noexcept {
        array_t<usb_device_t> res = nullptr;
        for( auto x=0; x<count(); x++ ){ try {
             res.push( usb_device_t( obj->list[x] ) );
        } catch(...){ } } if( obj->list !=nullptr ) { 
            libusb_free_device_list( obj->list, 1 ); 
        }   return res;
    }

    /*.........................................................................*/

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } obj->state = 0;
        if( obj->ctx != nullptr ) { libusb_exit( obj->ctx ); }
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif