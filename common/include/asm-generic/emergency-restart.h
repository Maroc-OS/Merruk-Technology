#ifndef _ASM_GENERIC_EMERGENCY_RESTART_H
#define _ASM_GENERIC_EMERGENCY_RESTART_H

static inline void machine_emergency_restart(void)
{
#if defined(CONFIG_SEC_DEBUG)
	machine_restart("upload");
#else
	machine_restart(NULL);
#endif
}

#endif /* _ASM_GENERIC_EMERGENCY_RESTART_H */
