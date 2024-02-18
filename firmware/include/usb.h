
/**
 * @brief Hold the Indices for string used by USB
 * The actual strings are stored in @ref stringDescriptorArray
 */
enum STRING_DESCRIPTOR_INDEX {
    LANG_ID = 0,
    VENDOR_NAME,
    PRODUCT_NAME,
    SERIAL_NUMBER,
    CONFIG0_NAME,
    INTERFACE0_NAME
};

/**
 * @brief Task, that runs tusb_task() function every 100ms
 * Just tinyusb stuff
 */
void usb_main_task();

/**
 * @brief Handles Incoming USB data every 50ms
 * 
 */
void usb_rx_task();

/**
 * @brief Callback, invoked on plugging in usb  
 * This is invoked by tinyusb
 */
void tud_mount_cb(void);

/**
 * @brief Callback, invoked on unplugging usb
 * This is invoked by tinyusb
 */
void tud_umount_cb(void);

/**
 * @brief Callback, invoked on suspend request by the usb host
 * This is invoked by tinyusb
 * @param remote_wakeup_en If true an resume call is to be expected
 */
void tud_suspend_cb(bool remote_wakeup_en);

/**
 * @brief Callback, invoked on resume request by the usb host
 * This is invoked by tinyusb
 */
void tud_resume_cb(void);
