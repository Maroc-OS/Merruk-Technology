/* Definations used for  Unified Logging */
if (bcmlog_mtt_on == 1 && bcmlog_log_ulogging_id > 0)
	BCMLOG_LogString(printk_buf, bcmlog_log_ulogging_id);
