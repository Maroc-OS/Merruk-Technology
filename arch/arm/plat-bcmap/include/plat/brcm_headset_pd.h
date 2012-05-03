/* Header file for HEASET DRIVER */


struct brcm_headset_pd {
	int hsirq;
	int hsbirq;
	void (*check_hs_state) (int *);
	int hsgpio;
	int debounce_ms;
        int keypress_threshold_fp;
        int keypress_threshold_lp;
	int key_press_threshold;
	int key_3pole_threshold;
	int key1_threshold_l;
	int key1_threshold_u;
	int key2_threshold_l;
	int key2_threshold_u;
	int key3_threshold_l;
	int key3_threshold_u;	
};

