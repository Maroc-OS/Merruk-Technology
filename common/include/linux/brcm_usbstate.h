#ifndef _LINUX_USBSTATE_H_
#define _LINUX_USBSTTE_H_ 1

#include <linux/completion.h>


struct android_usbconn_callbacks 
{
	/** notify usb connection event */
	int (*notify) (struct completion *);
	struct completion	usb_complete;
	bool	connected;
};


typedef enum
{
    AT_SERVER_MODE_RNDIS,         ///< Start server in RNDIS mode.
    AT_SERVER_MODE_UART,          ///< Start server in local mode (UART).
    AT_SERVER_MODE_FILE_DIRECT,    ///< Start server in local mode and handle a file redirect.
    AT_SERVER_MODE_USB_SERIEL,
    AT_SERVER_MODE_USB_SERIEL_UART

} ATServerMode_t;


#endif /* _LINUX_CONSOLE_H */
