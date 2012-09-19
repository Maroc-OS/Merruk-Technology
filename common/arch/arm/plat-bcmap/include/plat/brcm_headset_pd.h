/* Header file for HEASET DRIVER */

#ifndef __BCM_HEADSET_PD__
#define __BCM_HEADSET_PD__

struct brcm_headset_pd {
	int hsirq;
	int hsbirq;
	void (*check_hs_state) (int *);
	int hsgpio;
	int debounce_ms;
    short keypress_threshold_fp;
    short keypress_threshold_lp;
	short key_press_threshold;
	short key_3pole_threshold;
	short key1_threshold_l;
	short key1_threshold_u;
	short key2_threshold_l;
	short key2_threshold_u;
	short key3_threshold_l;
	short key3_threshold_u;	
};

#endif /*  __BCM_HEADSET_PD__ */
