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

namespace nodepp { class usb_device_t {
protected:

    struct NODE {
        libusb_device_handle* fd = nullptr;
        libusb_device  *ctx      = nullptr;
        libusb_context *idx      = nullptr;
        bool state = 0;
    };  ptr_t<NODE> obj;

    bool is_blocked( int error ) const noexcept {
        return error == LIBUSB_ERROR_TIMEOUT | LIBUSB_ERROR_BUSY;
    }

public:

    usb_device_t ( uint16_t vendorID, uint16_t productID ) : obj( new NODE() ) { libusb_init( &obj->idx );
        obj->fd = libusb_open_device_with_vid_pid( obj->idx, vendorID, productID );
        if( obj->fd == nullptr ){ 
            process::error( "can't initialize device" ); return; 
        }   obj->state = 1;
    }

    usb_device_t ( libusb_device* ctx ) : obj( new NODE() ) {
        if( ctx == nullptr ){ return; } if( libusb_open( ctx, &obj->fd )<0 ){ 
            process::error( "can't initialize device" ); return; 
        }   obj->state = 1; obj->ctx = ctx;
    }

    /*.........................................................................*/

   ~usb_device_t () noexcept { if( obj.count()>1 ){ return; } free(); }
    usb_device_t () noexcept : obj( new NODE() ){}

    /*.........................................................................*/

    libusb_device_descriptor get_descriptor() const noexcept {
        libusb_device_descriptor idx;
        libusb_get_device_descriptor( obj->ctx, &idx ); return idx;
    }

    string_t get_product() const noexcept { 
        uchar buff[256]; memset( buff, 0, sizeof(buff) ); auto idx = get_descriptor();
        auto len = libusb_get_string_descriptor_ascii( obj->fd, idx.iProduct, buff, sizeof(buff) );
        if ( len != LIBUSB_SUCCESS ) { return nullptr; } return string_t( (char*) buff, len );
    }

    string_t get_manufacturer() const noexcept { 
        uchar buff[256]; memset( buff, 0, sizeof(buff) ); auto idx = get_descriptor();
        auto len = libusb_get_string_descriptor_ascii( obj->fd, idx.iManufacturer, buff, sizeof(buff) );
        if ( len != LIBUSB_SUCCESS ) { return nullptr; } return string_t( (char*) buff, len );
    }

    string_t get_serial_id() const noexcept { 
        uchar buff[256]; memset( buff, 0, sizeof(buff) ); auto idx = get_descriptor();
        auto len = libusb_get_string_descriptor_ascii( obj->fd, idx.iSerialNumber, buff, sizeof(buff) );
        if ( len != LIBUSB_SUCCESS ) { return nullptr; } return string_t( (char*) buff, len );
    }

    /*.........................................................................*/

    int _interrupt_read() const noexcept { return 0; }

    int _control_read() const noexcept { return 0; }

    int _bulk_read() const noexcept { return 0; }

    /*.........................................................................*/

    int _interrupt_write() const noexcept { return 0; }

    int _control_write() const noexcept { return 0; }

    int _bulk_write() const noexcept { return 0; }

    /*.........................................................................*/

    void free() const noexcept {
        if( obj->state == 0 ){ return; } obj->state = 0;
        if( obj->idx != nullptr ) { libusb_exit( obj->idx ); }
        if( obj->fd  != nullptr ) { libusb_close( obj->fd ); } 
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class usb_t {
protected:

    struct NODE {
        libusb_context* ctx  = nullptr;
        libusb_device **list = nullptr;
        bool state = 0;
    };  ptr_t<NODE> obj;

    int count() const noexcept { 
        if( obj->state == 0 ){ return -1; }
        return libusb_get_device_list( obj->ctx, &obj->list ); 
    }

public:

   ~usb_t () noexcept { if( obj.count()>1 ){ return; } free(); }

    usb_t () : obj( new NODE() ) { if( libusb_init( &obj->ctx ) < 0 ) {
        process::error("Can't initialize USB"); return;
    } obj->state = 1; }

    /*.........................................................................*/

    array_t<usb_device_t> get_devices() const noexcept {
        array_t<usb_device_t> res = nullptr;
        for( auto x=0; x<count(); x++ ){ try {
             res.push( usb_device_t( obj->list[x] ) );
        } catch(...){ } } return res;
    }

    /*.........................................................................*/

    void free() const noexcept { 
        if( obj->state == 0 ){ return; } obj->state = 0;
        if( obj->ctx  != nullptr ) { libusb_exit( obj->ctx ); }
        if( obj->list != nullptr ) { libusb_free_device_list( obj->list, 1 ); }
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

#endif