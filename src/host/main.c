#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

#include "../communication.h"

#define VENDOR_ID       0x16c0
#define PRODUCT_ID      0x05dc
#define MANUFACTURER    "fsv3-se,rb"
#define PRODUCTNAME     "cash-box-ctrl"

#define STR_LENGTH  50

int open_usb_device(const uint16_t idVendor,
                    const uint16_t idProduct,
                    const char const* maufacturer,
                    const char const* product,
                    libusb_device_handle **device);

int main(int argc,
         char **argv)
{
    libusb_device_handle *device;
    int ret;

    ret = libusb_init(NULL); // NULL for the default context
    if(ret != LIBUSB_SUCCESS)
    {
        fprintf(stderr,
                "libusb initialize error: %s\n",
                libusb_error_name(ret));
        exit(EXIT_FAILURE);
    }
    libusb_set_debug(NULL,
                     3); //informational messages are printed to stdout, warning and error messages are printed to stderr

    ret = open_usb_device(VENDOR_ID,
                          PRODUCT_ID,
                          MANUFACTURER,
                          PRODUCTNAME,
                          &device);
    if(ret != LIBUSB_SUCCESS)
    {
        fprintf(stderr,
                "error opening usb device: %04x:%04x %s|%s\n",
                VENDOR_ID,
                PRODUCT_ID,
                MANUFACTURER,
                PRODUCTNAME);
        exit(EXIT_FAILURE);
    }

    libusb_control_transfer(device,
                            LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
                            RQ_CASH_BOX_OPEN,
                            0,
                            0,
                            NULL,
                            0,
                            0);

    libusb_close(device);
    libusb_exit(NULL);
    return EXIT_SUCCESS;
}

int open_usb_device(const uint16_t idVendor,
                    const uint16_t idProduct,
                    const char const* maufacturer,
                    const char const* product,
                    libusb_device_handle **device)
{

    libusb_device **device_list;
    struct libusb_device_descriptor desc;
    ssize_t cnt;
    int i;
    int ret;
    int len = 0;
    unsigned char str[STR_LENGTH];

    cnt = libusb_get_device_list(NULL,
                                 &device_list );
    if(cnt <= 0)
    {
        fprintf(stderr,
                "get device list error: %s\n",
                libusb_error_name(cnt));
        return cnt;
    }

    #ifdef DEBUG
    printf("Found %d USB Devices\n",
           (int)cnt);
    #endif

    for(i = 0; i < cnt; i++)
    {
        ret = libusb_get_device_descriptor(device_list[i],
                                           &desc);
        if(ret != LIBUSB_SUCCESS)
        {
            fprintf(stderr,
                    "cant get device descriptor: %s\n",
                    libusb_error_name(ret));
            continue;
        }

        /* check vendor id and product id */
        if(desc.idVendor != idVendor || desc.idProduct != idProduct)
        {
            continue;
        }

        /* open device to perform closer checks */
        ret = libusb_open(device_list[i],
                          device);
        if(ret != LIBUSB_SUCCESS)
        {
            fprintf(stderr,
                    "cant open device: %s\n",
                    libusb_error_name(ret));
            continue;
        }
        libusb_set_configuration(*device,
                                 1);

        /* check manufacturer */
        ret = libusb_get_string_descriptor_ascii(*device,
                                                 desc.iManufacturer,
                                                 str,
                                                 STR_LENGTH);
        if(ret < LIBUSB_SUCCESS)
        {
            fprintf(stderr,
                    "cant get string manufacturer: %s\n",
                    libusb_error_name(ret));
            continue;
        }
        else if(strncmp(maufacturer,
                         (char *)str,
                         strlen(maufacturer)))
        {
            printf("%d", len);
            continue;
        }

        /* check productname */
        ret = libusb_get_string_descriptor_ascii(*device,
                                                 desc.iProduct,
                                                 str,
                                                 STR_LENGTH);
        if(ret < LIBUSB_SUCCESS)
        {
            fprintf(stderr,
                    "cant get string product: %s\n",
                    libusb_error_name(ret));
            continue;
        }
        else if(strncmp(product,
                   (char *)str,
                   strlen(product)))
        {
            continue;
        }

        #ifdef DEBUG
        printf("found device and selected");
        #endif
        libusb_free_device_list(device_list, i);
        return LIBUSB_SUCCESS;
    }
    libusb_free_device_list(device_list, 0);
    device = NULL;
    return -100;
}
