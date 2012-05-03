#ifndef _wl_iw_wapi_h_
#define _wl_iw_wapi_h_

#ifndef IW_ENCODE_ALG_SM4
#define IW_ENCODE_ALG_SM4 0x20
#endif

#ifndef IW_AUTH_WAPI_ENABLED
#define IW_AUTH_WAPI_ENABLED 0x20
#endif

#ifndef IW_AUTH_WAPI_VERSION_1
#define IW_AUTH_WAPI_VERSION_1	0x00000008
#endif

#ifndef IW_AUTH_CIPHER_SMS4
#define IW_AUTH_CIPHER_SMS4	0x00000020
#endif

#ifndef IW_AUTH_KEY_MGMT_WAPI_PSK
#define IW_AUTH_KEY_MGMT_WAPI_PSK 4
#endif

#ifndef IW_AUTH_KEY_MGMT_WAPI_CERT
#define IW_AUTH_KEY_MGMT_WAPI_CERT 8
#endif

#define WAPI_AUTH_NONE          WPA_AUTH_NONE   /* none (IBSS) */
#define WAPI_AUTH_UNSPECIFIED   0x0400          /* over AS */
#define WAPI_AUTH_PSK           0x0800          /* Pre-shared key */

#endif