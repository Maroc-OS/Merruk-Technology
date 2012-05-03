#if !defined _TST200_DOWNLOAD_LUISA_H_
#define _TST200_DOWNLOAD_LUISA_H_

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
/////						issp_directives.h
/////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// ****************** USER ATTENTION REQUIRED: TARGET PSOC ********************
// ****************************************************************************
// The directives below enable support for various PSoC devices. The root part
// number to be programmed should be un-commented so that its value becomes
// defined.  All other devices should be commented out.
// Select one device to be supported below:
#define CY8CTST200_24

//-----------------------------------------------------------------------------
// The directives below are used for Krypton.
// See the Krypton programming spec 001-15870 rev *A for more details. (The
// spec uses "mnemonics" instead of "directives"
//-----------------------------------------------------------------------------
#define TSYNC

#define ID_SETUP_1					//PTJ: ID_SETUP_1 is similar to init1_v
#define ID_SETUP_2					//PTJ: ID_SETUP_2 is similar to init2_v
#define SET_BLOCK_NUM
#define CHECKSUM_SETUP				//PTJ: CHECKSUM_SETUP_20x66 is the same as CHECKSUM-SETUP in 001-15870
#define READ_CHECKSUM
#define PROGRAM_AND_VERIFY		//PTJ: PROGRAM_BLOCK_20x66 is the same as PROGRAM-AND-VERIFY in 001-15870
#define ERASE
#define SECURE
#define READ_SECURITY
#define READ_WRITE_SETUP
#define WRITE_BYTE
#define VERIFY_SETUP
#define READ_STATUS
#define READ_BYTE




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
/////						issp_defs.h
/////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Block-Verify Uses 128-Bytes of RAM
#define TARGET_DATABUFF_LEN    128  			// **** CY8C20x66 Device ****

// The number of Flash blocks in each part is defined here. This is used in
// main programming loop when programming and verifying the blocks.
#if defined(CY8C20x36)
    #define NUM_BANKS                     1
    #define BLOCKS_PER_BANK              64
    #define SECURITY_BYTES_PER_BANK      32
#else
    #define NUM_BANKS                     1
    #define BLOCKS_PER_BANK             256
    #define SECURITY_BYTES_PER_BANK      64
#endif




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
/////						issp_delays.h
/////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// The Delay() routine, in ISSP_Driver_Routines.c, has a delay of n+3 usec, 
// where n is the value passed to the routine.  This is true for the m8c micro-
// processor in the PSoC when it is running at a CPU clock of 24MHz.
//
// PROCESSOR_SPECIFIC
// If another processor is used, or if the m8c is running at a slower clock
// speed, then the delay parameters will be different. This file makes changing
// the delays simpiler when porting the program to other processors.

// DELAY_M is the slope of the Delay = Mx + B equation
#define DELAY_M    1
// DELAY_B is the offset of the delay in Delay = Mx + B.
#define DELAY_B    3

///////////////////////////////////////////////////////////////////////////////
// CAUTION:
// For the above parameters the minimum delay value is 3 (this would result in 
// 0 being passed for a minimum delay. A value less than 3 would actually 
// create a negative number, causing a very long delay
///////////////////////////////////////////////////////////////////////////////

// TRANSITION_TIMEOUT is a loop counter for a 100msec timeout when waiting for 
// a high-to-low transition. This is used in the polling loop of 
// fDetectHiLoTransition(). Each pass through the loop takes approximately 15
// usec. 100 msec is about 6740 loops. 13480
//#define TRANSITION_TIMEOUT     6740
//#define TRANSITION_TIMEOUT     (100*1000*100)
#define TRANSITION_TIMEOUT     (100*1000)


// XRES_DELAY is the time duration for which XRES is asserted. This defines
// a 63 usec delay.
// The minimum Xres time (from the device datasheet) is 10 usec.
#define XRES_CLK_DELAY    ((63 - DELAY_B) / DELAY_M)

// POWER_CYCLE_DELAY is the time required when power is cycled to the target
// device to create a power reset after programming has been completed. The
// actual time of this delay will vary from system to system depending on the
// bypass capacitor size.  A delay of 150 usec is used here.
#define POWER_CYCLE_DELAY ((150 - DELAY_B) / DELAY_M)

// DELAY_100us delays 100 usec. This is used in fXRESInitializeTargetForISSP to
// time the wait for Vdd to become stable after a power up.  A loop runs 10 of
// these for a total delay of 1 msec.
#define DELAY100us        ((100 - DELAY_B) / DELAY_M)




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
/////						issp_errors.h
/////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// The following are defines for error messages from the ISSP program.
#define PASS           0
// PASS is used to indicate that a function completed successfully.
#define ERROR         -1
// ERROR is a generic failure used within lower level functions before the
// error is reported.  This should not be seen as an error that is reported
// from main.
#define INIT_ERROR     1
// INIT_ERROR means a step in chip initialization failed.
#define SiID_ERROR     2
// SiID_ERROR means that the Silicon ID check failed. This happens if the
// target part does not match the device type that the ISSP program is
// configured for.
#define ERASE_ERROR    3
// ERASE_ERROR means that the bulk erase step failed. 
#define BLOCK_ERROR    4
// BLOCK_ERROR means that a step in programming a Flash block or the verify
// of the block failed.
#define VERIFY_ERROR   5
// VERIFY_ERROR means that the checksum verification failed.
#define SECURITY_ERROR 6
// SECURITY_ERROR means that the write of the security information failed.
#define STATUS_ERROR 7

#define CHECKSUM_ERROR 8

#define NO_FIRMWARE_ERROR	0x10
#define FILE_ACCESS_ERROR	0x11




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
/////						issp_vectors.h
/////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

unsigned char target_status00_v = 0x00;		//PTJ: Status = 00 means Success, the SROM function did what it was supposed to
unsigned char target_status01_v = 0x01;		//PTJ: Status = 01 means that function is not allowed because of block level protection, for test with verify_setup (VERIFY-SETUP)
unsigned char target_status03_v = 0x03;		//PTJ: Status = 03 is fatal error, SROM halted
unsigned char target_status04_v = 0x04;		//PTJ: Status = 04 means that ___ for test with ___ (PROGRAM-AND-VERIFY)
unsigned char target_status06_v = 0x06;		//PTJ: Status = 06 means that Calibrate1 failed, for test with id_setup_1 (ID-SETUP-1)

// ------------------------- PSoC CY8CTMA30x, CY8CTMG30x, CY8CTST30x Devices ---------------------------
// Modifying these tables is NOT recommendended. Doing so will all but
// guarantee an ISSP error, unless updated vectors have been recommended or
// provided by Cypress Semiconductor.
// ----------------------------------------------------------------------------
#ifdef CY8CTMG200_32
    unsigned char target_id_v[] = {0x07, 0x6e};
#endif

#ifdef CY8CTMG200_48
    unsigned char target_id_v[] = {0x07, 0x6f};
#endif

#ifdef CY8CTST200_24
    unsigned char target_id_v[] = {0x06, 0x6d};
#endif

#ifdef CY8CTST200_32
    unsigned char target_id_v[] = {0x06, 0x6e};
#endif

#ifdef CY8C20336_24
    unsigned char target_id_v[] = {0x00, 0xb4};
#endif


/********************************************************************************************************/

// ----------------------------------------------------------------------------
#ifdef TSYNC
	const unsigned int num_bits_tsync_enable = 110;
    const unsigned char tsync_enable[] =
    {
        0xDE, 0xE2, 0x1F, 0x7F, 0x02, 0x7D, 0xC4, 0x09,
        0xF7, 0x00, 0x1F, 0xDE, 0xE0, 0x1C
	};
	const unsigned int num_bits_tsync_disable = 110;
	const unsigned char tsync_disable[] =
	{
        0xDE, 0xE2, 0x1F, 0x71, 0x00, 0x7D, 0xFC, 0x01,
        0xF7, 0x00, 0x1F, 0xDE, 0xE0, 0x1C
	};
#endif

#ifdef ID_SETUP_1
    const unsigned int num_bits_id_setup_1 = 594;		//KIMC, 2009.08.11, PTJ: id_setup_1 with TSYNC enabled for MW and disabled for IOW
    const unsigned char id_setup_1[] =
    {
        0xCA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0xEE, 0x21, 0xF7,
        0xF0, 0x27, 0xDC, 0x40, 0x9F, 0x70, 0x01, 0xFD, 0xEE, 0x01,
        0xE7, 0xC1, 0xD7, 0x9F, 0x20, 0x7E, 0x7D, 0x88, 0x7D, 0xEE,
        0x21, 0xF7, 0xF0, 0x07, 0xDC, 0x40, 0x1F, 0x70, 0x01, 0xFD,
        0xEE, 0x01, 0xF7, 0xA0, 0x1F, 0xDE, 0xA0, 0x1F, 0x7B, 0x00,
        0x7D, 0xE0, 0x13, 0xF7, 0xC0, 0x07, 0xDF, 0x28, 0x1F, 0x7D,
        0x18, 0x7D, 0xFE, 0x25, 0xC0
    };
#endif

#ifdef SET_BLOCK_NUM
	const unsigned int num_bits_set_block_num = 11;		//PTJ:
    const unsigned char set_block_num[] =
	{
        0x9F, 0x40
    };
    const unsigned int num_bits_set_block_num_end = 3;		//PTJ: this selects the first three bits of set_block_num_end
    const unsigned char set_block_num_end = 0xE0;
#endif

#ifdef READ_WRITE_SETUP
	const unsigned int num_bits_read_write_setup = 66;		//PTJ:
    const unsigned char read_write_setup[] =
    {
        0xDE, 0xF0, 0x1F, 0x78, 0x00, 0x7D, 0xA0, 0x03, 0xC0
	};
#endif

#ifdef VERIFY_SETUP
	const unsigned int num_bits_my_verify_setup = 440;
    const unsigned char verify_setup[] =
    {
        0xDE, 0xE2, 0x1F, 0x7F, 0x02, 0x7D, 0xC4, 0x09,
        0xF7, 0x00, 0x1F, 0x9F, 0x07, 0x5E, 0x7C, 0x81,
        0xF9, 0xF7, 0x01, 0xF7, 0xF0, 0x07, 0xDC, 0x40,
        0x1F, 0x70, 0x01, 0xFD, 0xEE, 0x01, 0xF6, 0xA8,
        0x0F, 0xDE, 0x80, 0x7F, 0x7A, 0x80, 0x7D, 0xEC,
        0x01, 0xF7, 0x80, 0x0F, 0xDF, 0x00, 0x1F, 0x7C,
        0xA0, 0x7D, 0xF4, 0x61, 0xF7, 0xF8, 0x97

//        0b11011110, 0b11100010, 0b00011111, 0b01111111, 0b00000010, 0b01111101, 0b11000100, 0b00001001,
//        0b11110111, 0b00000000, 0b00011111, 0b10011111, 0b00000111, 0b01011110, 0b01111100, 0b10000001,
//        0b11111001, 0b11110111, 0b00000001, 0b11110111, 0b11110000, 0b00000111, 0b11011100, 0b01000000,
//        0b00011111, 0b01110000, 0b00000001, 0b11111101, 0b11101110, 0b00000001, 0b11110110, 0b10101000,
//        0b00001111, 0b11011110, 0b10000000, 0b01111111, 0b01111010, 0b10000000, 0b01111101, 0b11101100,
//        0b00000001, 0b11110111, 0b10000000, 0b00001111, 0b11011111, 0b00000000, 0b00011111, 0b01111100,
//        0b10100000, 0b01111101, 0b11110100, 0b01100001, 0b11110111, 0b11111000, 0b10010111
    };
#endif

#ifdef ERASE
	const unsigned int num_bits_erase = 396;		//PTJ: erase with TSYNC Enable and Disable
    const unsigned char erase[] =
	{
        0xDE, 0xE2, 0x1F, 0x7F, 0x02, 0x7D, 0xC4, 0x09, 0xF7, 0x00,
        0x1F, 0x9F, 0x07, 0x5E, 0x7C, 0x85, 0xFD, 0xFC, 0x01, 0xF7,
        0x10, 0x07, 0xDC, 0x00, 0x7F, 0x7B, 0x80, 0x7D, 0xE0, 0x0B,
        0xF7, 0xA0, 0x1F, 0xDE, 0xA0, 0x1F, 0x7B, 0x04, 0x7D, 0xF0,
        0x01, 0xF7, 0xC9, 0x87, 0xDF, 0x48, 0x1F, 0x7F, 0x89, 0x70

    };
#endif

#ifdef SECURE
	const unsigned int num_bits_secure = 440;		//PTJ: secure with TSYNC Enable and Disable
    const unsigned char secure[] =
	{
        0xDE, 0xE2, 0x1F, 0x7F, 0x02, 0x7D, 0xC4, 0x09,
        0xF7, 0x00, 0x1F, 0x9F, 0x07, 0x5E, 0x7C, 0x81,
        0xF9, 0xF7, 0x01, 0xF7, 0xF0, 0x07, 0xDC, 0x40,
        0x1F, 0x70, 0x01, 0xFD, 0xEE, 0x01, 0xF6, 0xA0,
        0x0F, 0xDE, 0x80, 0x7F, 0x7A, 0x80, 0x7D, 0xEC,
        0x01, 0xF7, 0x80, 0x27, 0xDF, 0x00, 0x1F, 0x7C,
        0xA0, 0x7D, 0xF4, 0x61, 0xF7, 0xF8, 0x97

//        0b11011110, 0b11100010, 0b00011111, 0b01111111, 0b00000010, 0b01111101, 0b11000100, 0b00001001,
//        0b11110111, 0b00000000, 0b00011111, 0b10011111, 0b00000111, 0b01011110, 0b01111100, 0b10000001,
//        0b11111001, 0b11110111, 0b00000001, 0b11110111, 0b11110000, 0b00000111, 0b11011100, 0b01000000,
//        0b00011111, 0b01110000, 0b00000001, 0b11111101, 0b11101110, 0b00000001, 0b11110110, 0b10100000,
//        0b00001111, 0b11011110, 0b10000000, 0b01111111, 0b01111010, 0b10000000, 0b01111101, 0b11101100,
//        0b00000001, 0b11110111, 0b10000000, 0b00100111, 0b11011111, 0b00000000, 0b00011111, 0b01111100,
//        0b10100000, 0b01111101, 0b11110100, 0b01100001, 0b11110111, 0b11111000, 0b10010111
    };
#endif


#ifdef READ_SECURITY
	const unsigned int num_bits_SPCTestMode_enable = 88;		//PTJ: READ-SECURITY-SETUP
	const unsigned char SPCTestMode_enable[] =
	{
        0xDE, 0xE2, 0x1F, 0x60, 0x88, 0x7D, 0x84, 0x21, 0xF7, 0xB8, 0x07

//	0b11011110, 0b11100010, 0b00011111, 0b01100000, 0b10001000, 0b01111101, 0b10000100, 0b00100001,
//	0b11110111, 0b10111000, 0b00000111
	};

	const unsigned int num_bits_read_security_pt1 = 78;		//PTJ: This sends the beginning of the Read Supervisory command
    const unsigned char read_security_pt1[] =
    {

        0xDE, 0xE2, 0x1F, 0x72, 0x87, 0x7D, 0xCA, 0x01,
        0xF7, 0x28
//		0b11011110, 0b11100010, 0b00011111, 0b01110010, 0b10000111, 0b01111101, 0b11001010, 0b00000001,
//		0b11110111, 0b00101000
    };

    const unsigned int num_bits_read_security_pt1_end = 25;		//PTJ: this finishes the Address Low command and sends the Address High command
    const unsigned char read_security_pt1_end[] =
    {
        0xFB, 0x94, 0x03, 0x80
//    0b11111011, 0b10010100, 0b00000011, 0b10000000
    };

   	const unsigned int num_bits_read_security_pt2 = 198;		//PTJ: load the test queue with the op code for MOV 1,E5h register into Accumulator A
    const unsigned char read_security_pt2[] =
    {
        0xDE, 0xE0, 0x1F, 0x7A, 0x01, 0xFD, 0xEA, 0x01,
        0xF7, 0xB0, 0x07, 0xDF, 0x0B, 0xBF, 0x7C, 0xF2,
        0xFD, 0xF4, 0x61, 0xF7, 0xB8, 0x87, 0xDF, 0xE2,
        0x5C
//    0b11011110, 0b11100000, 0b00011111, 0b01111010, 0b00000001, 0b11111101, 0b11101010, 0b00000001,
//    0b11110111, 0b10110000, 0b00000111, 0b11011111, 0b00001011, 0b10111111, 0b01111100, 0b11110010,
//    0b11111101, 0b11110100, 0b01100001, 0b11110111, 0b10111000, 0b10000111, 0b11011111, 0b11100010,
//    0b01011100
    };

    const unsigned int num_bits_read_security_pt3 = 122;		//PTJ:
    const unsigned char read_security_pt3[] =
    {
        0xDE, 0xE0, 0x1F, 0x7A, 0x01, 0xFD, 0xEA, 0x01,
        0xF7, 0xB0, 0x07, 0xDF, 0x0A, 0x7F, 0x7C, 0xC0
//    	0b11011110, 0b11100000, 0b00011111, 0b01111010, 0b00000001, 0b11111101, 0b11101010, 0b00000001,
//    	0b11110111, 0b10110000, 0b00000111, 0b11011111, 0b00001010, 0b01111111, 0b01111100, 0b11000000
    };

    const unsigned int num_bits_read_security_pt3_end = 47;		//PTJ:
    const unsigned char read_security_pt3_end[] =
    {
        0xFB, 0xE8, 0xC3, 0xEF, 0xF1, 0x2E
//		0b11111011, 0b11101000, 0b11000011, 0b11101111, 0b11110001, 0b00101110
    };

#endif


#ifdef CHECKSUM_SETUP
    const unsigned int num_bits_checksum_setup = 418;		//PTJ: Checksum with TSYNC Enable and Disable
    const unsigned char checksum_setup[] =
    {
        0xDE, 0xE2, 0x1F, 0x7F, 0x02, 0x7D, 0xC4, 0x09, 0xF7, 0x00,
        0x1F, 0x9F, 0x07, 0x5E, 0x7C, 0x81, 0xF9, 0xF4, 0x01, 0xF7,
        0xF0, 0x07, 0xDC, 0x40, 0x1F, 0x70, 0x01, 0xFD, 0xEE, 0x01,
        0xF7, 0xA0, 0x1F, 0xDE, 0xA0, 0x1F, 0x7B, 0x00, 0x7D, 0xE0,
        0x0F, 0xF7, 0xC0, 0x07, 0xDF, 0x28, 0x1F, 0x7D, 0x18, 0x7D,
        0xFE, 0x25, 0xC0

    };
#endif

// ----------------------------------------------------------------------------
#ifdef PROGRAM_AND_VERIFY
    const unsigned int num_bits_program_and_verify = 440;		//KIMC, PTJ: length of program_block[], not including zero padding at end
    const unsigned char program_and_verify[] =
    {
        0xDE, 0xE2, 0x1F, 0x7F, 0x02, 0x7D, 0xC4, 0x9,
        0xF7, 0x00, 0x1F, 0x9F, 0x07, 0x5E, 0x7C, 0x81,
        0xF9, 0xF7, 0x01, 0xF7, 0xF0, 0x07, 0xDC, 0x40,
        0x1F, 0x70, 0x01, 0xFD, 0xEE, 0x01, 0xF6, 0xA0,
        0x0F, 0xDE, 0x80, 0x7F, 0x7A, 0x80, 0x7D, 0xEC,
        0x01, 0xF7, 0x80, 0x57, 0xDF, 0x00, 0x1F, 0x7C,
        0xA0, 0x7D, 0xF4, 0x61, 0xF7, 0xF8, 0x97
    };
#endif

#ifdef ID_SETUP_2
	const unsigned int num_bits_id_setup_2 = 418; 		//PTJ: id_setup_2 with TSYNC Disable (TSYNC enabled before with SendVector(tsync_enable....)
    const unsigned char id_setup_2[] =
	{
        0xDE, 0xE2, 0x1F, 0x7F, 0x02, 0x7D, 0xC4, 0x09, 0xF7, 0x00,
        0x1F, 0x9F, 0x07, 0x5E, 0x7C, 0x81, 0xF9, 0xF4, 0x01, 0xF7,
        0xF0, 0x07, 0xDC, 0x40, 0x1F, 0x70, 0x01, 0xFD, 0xEE, 0x01,
        0xF7, 0xA0, 0x1F, 0xDE, 0xA0, 0x1F, 0x7B, 0x00, 0x7D, 0xE0,
        0x0D, 0xF7, 0xC0, 0x07, 0xDF, 0x28, 0x1F, 0x7D, 0x18, 0x7D,
        0xFE, 0x25, 0xC0
    };
#endif

    const unsigned char read_id_v[] =
    {
		0xBF, 0x00, 0xDF, 0x90, 0x00, 0xFE, 0x60, 0xFF, 0x00
//		0b10111111,0b00000000,0b11011111,0b10010000,0b00000000,0b11111110,0b0110000,0b11111111,0b00000000
    };


    const unsigned char Switch_Bank1[] =				//PTJ: use this to switch between register banks
    {
    	0xDE, 0xE2, 0x1C
//    	0b11011110, 0b11100010, 0b00011100
    };

    const unsigned char Switch_Bank0[] =				//PTJ: use this to switch between register banks
    {
    	0xDE, 0xE0, 0x1C
//    	0b11011110, 0b11100000, 0b00011100
    };

    const unsigned char read_IMOtrim[] =				//PTJ: read the 1,E8h register after id__setup_1 to see if the cal data was loaded properly.
    {
    	0xFD, 0x00, 0x10
//    	0b11111101, 0b00000000, 0b00010000
    };
    const unsigned char read_SPCtrim[] =				//PTJ: read the 1,E7h register after id__setup_1 to see if the cal data was loaded properly.
    {
    	0xFC, 0xE0, 0x10
//    	0b11111100, 0b11100000, 0b00010000
    };
    const unsigned char read_VBGfinetrim[] =			//PTJ: read the 1,D7h register after id__setup_1 to see if the cal data was loaded properly.
    {
    	0xFA, 0xE0, 0x08
//    	0b11111010, 0b11100000, 0b0001000
    };
    const unsigned char read_reg_end = 0x80;			//PTJ: this is the final '1' after a MR command


    const unsigned char    write_byte_start = 0x90;			//PTJ: this is set to SRAM 0x80
    const unsigned char    write_byte_end = 0xE0;

    const unsigned char    set_block_number[] = {0x9F, 0x40, 0xE0};
    const unsigned char    set_block_number_end = 0xE0;

    const unsigned char    num_bits_wait_and_poll_end = 40;
    const unsigned char    wait_and_poll_end[] =
    {
        0x00, 0x00, 0x00, 0x00, 0x00
    };    // forty '0's per the spec

    const unsigned char read_checksum_v[] =
    {
        0xBF, 0x20, 0xDF, 0x80, 0x80

//          0b10111111, 0b00100000,0b11011111,0b10000000,0b10000000
    };


    const unsigned char read_byte_v[] =
    {
        0xB0, 0x80
//          0b10110000, 0b10000000
    };




//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/////
/////						issp_extern.h
/////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

extern signed char fXRESInitializeTargetForISSP(void);
extern signed char fPowerCycleInitializeTargetForISSP(void);
extern signed char fEraseTarget(void);
extern unsigned int iLoadTarget(void);
extern void ReStartTarget(void);
extern signed char fVerifySiliconID(void);
extern signed char fAccTargetBankChecksum(unsigned int*);
extern void SetBankNumber(unsigned char);
extern signed char fProgramTargetBlock(unsigned char, unsigned char);
extern signed char fVerifyTargetBlock(unsigned char, unsigned char);
extern signed char fVerifySetup(unsigned char, unsigned char);	//PTJ: VERIFY-SETUP
extern signed char fReadByteLoop(void);							//PTJ: read bytes after VERIFY-SETUP
extern signed char fSecureTargetFlash(void);

extern signed char fReadStatus(void);									//PTJ: READ-STATUS
extern signed char fReadCalRegisters(void);
extern signed char fReadWriteSetup(void);								//PTJ: READ-WRITE-SETUP
extern signed char fReadSecurity(void);									//PTJ: READ-SECURITY

extern void InitTargetTestData(void);

extern void LoadProgramData(unsigned char, unsigned char);
extern signed char fLoadSecurityData(unsigned char);
extern void Delay(unsigned char);
extern unsigned char fSDATACheck(void);
extern void SCLKHigh(void);
extern void SCLKLow(void);
#ifndef RESET_MODE  //only needed when power cycle mode
  extern void SetSCLKHiZ(void);
#endif
extern void SetSCLKStrong(void);
extern void SetSDATAHigh(void);
extern void SetSDATALow(void);
extern void SetSDATAHiZ(void);
extern void SetSDATAStrong(void);
extern void AssertXRES(void);
extern void DeassertXRES(void);
extern void SetXRESStrong(void);
extern void ApplyTargetVDD(void);
extern void RemoveTargetVDD(void);
extern void SetTargetVDDStrong(void);

extern unsigned char   fIsError;


#endif //_TST200_DOWNLOAD_LUISA_H_

