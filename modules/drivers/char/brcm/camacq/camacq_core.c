/*.......................................................................................................
. COPYRIGHT (C)  SAMSUNG Electronics CO., LTD (Suwon, Korea). 2009           
. All rights are reserved. Reproduction and redistiribution in whole or 
. in part is prohibited without the written consent of the copyright owner.
. 
.   Developer:
.   Date:
.   Description:  
..........................................................................................................
*/

#if !defined(_CAMACQ_CORE_C_)
#define _CAMACQ_CORE_C_

#include "camacq_model.h"
#include "camacq_core.h"
#include "camacq_api.h"

extern int ccic_sensor_attach( struct stCamacqSensorManager_t *pSensorManager );

static int __devinit CamacqCoreProbe( struct i2c_client *pClient, const struct i2c_device_id * d )
{
    int iRet;
    _stCamacqSensor* pstSensor = g_pstCamacqSensorManager->GetSensor( g_pstCamacqSensorManager, 
                                                                    g_pstCamacqSensorManager->m_uiSelectedSensor );

    CamacqTraceIN();

    // 0. set up i2c client
    pstSensor->m_pI2cClient = pClient;
    CamacqTraceDbg( "pClient : %x, name : %s", (U32)pClient, pClient->name );

    // 1. detect sensor	
    iRet = pstSensor->m_pstAPIs->Detect( pstSensor );
    if( iRet )
    {
        CamacqTraceErr( " : fail to detect sensor !!!!!" );
        iRet = -ENODEV;
    }
    else 
    {
        CamacqTraceDbg( " : success to detect sensor !!!!!" );
    }

    CamacqTraceOUT();

    return 0;
}


static int CamacqCoreRemove( struct i2c_client *pClient )
{
	CamacqTraceIN();
	CamacqTraceOUT();
	return 0;	//TODO
}

struct i2c_device_id CamacqMainIdtable[] = {
	{ CAMACQ_MAIN_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, CamacqMainIdtable);

static struct i2c_driver CamacqMainDriver = {
	.driver = {
		.name	= CAMACQ_MAIN_NAME,
	},
	.id_table   = CamacqMainIdtable,
	.probe		= CamacqCoreProbe,
	.remove		= CamacqCoreRemove,
};

#if (CAMACQ_SENSOR_MAX==2)
struct i2c_device_id CamacqSubIdtable[] = {
	{ CAMACQ_SUB_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, CamacqSubIdtable);

static struct i2c_driver CamacqSubDriver = {
	.driver = {
		.name	= CAMACQ_SUB_NAME,
	},
	.id_table   = CamacqSubIdtable,
	.probe		= CamacqCoreProbe,
	.remove		= CamacqCoreRemove,
};
#endif /* CAMACQ_SENSOR_MAX==2 */

/*
 * Module initialization
 */
static S32 __init CamacqCoreInit(void)
{
    int iRet = 0;
    CamacqTraceIN();

    CreateCamacqSensorManager( &g_pstCamacqSensorManager );

#if (CAMACQ_SENSOR_MAX==2)
    g_pstCamacqSensorManager->SetSensor( g_pstCamacqSensorManager, CAMACQ_SENSOR_SUB );
    iRet = i2c_add_driver( &CamacqSubDriver );
    CamacqTraceDbg( " : iRet[%d]", iRet );
#endif /* CAMACQ_SENSOR_MAX==2 */

    g_pstCamacqSensorManager->SetSensor( g_pstCamacqSensorManager, CAMACQ_SENSOR_MAIN );
    iRet = i2c_add_driver( &CamacqMainDriver );
    CamacqTraceDbg( " : iRet[%d]", iRet );

	// attach pxa950_camera.c
    ccic_sensor_attach( g_pstCamacqSensorManager );
	
    CamacqTraceOUT();

    return iRet;
}

static void __exit CamacqCoreExit(void)
{
    CamacqTraceIN();

    i2c_del_driver(&CamacqMainDriver);

#if (CAMACQ_SENSOR_MAX==2)
    i2c_del_driver(&CamacqSubDriver);
#endif /* CAMACQ_SENSOR_MAX==2 */

    CamacqTraceOUT();
}

/* Module parameters */
// module_init(CamacqCoreInit);
late_initcall(CamacqCoreInit);
module_exit(CamacqCoreExit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR("SAMSUNG");
MODULE_DESCRIPTION("Camera acquisition driver management");
MODULE_VERSION("1.00");

#undef _CAMACQ_CORE_C_
#endif /* _CAMACQ_CORE_C_ */

