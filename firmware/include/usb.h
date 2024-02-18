#include "tusb.h"

enum STRING_DESCRIPTOR_INDEX {
    LANG_ID = 0,
    VENDOR_NAME,
    PRODUCT_NAME,
    SERIAL_NUMBER,
    CONFIG0_NAME,
    INTERFACE0_NAME
};

void usb_main_task();

void usb_rx_task();

void tud_mount_cb(void);

void tud_umount_cb(void);

void tud_suspend_cb(bool remote_wakeup_en);

void tud_resume_cb(void);

// Invoked when received new data
TU_ATTR_WEAK void tud_vendor_rx_cb(uint8_t itf);

// Invoked when last rx transfer finished
TU_ATTR_WEAK void tud_vendor_tx_cb(uint8_t itf, uint32_t sent_bytes);