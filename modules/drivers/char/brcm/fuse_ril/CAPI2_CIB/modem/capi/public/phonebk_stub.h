/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a license
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   phonebk_stub.h
*
*   @brief  This file contains definitions for phonebook stub functions.
*
****************************************************************************/

#ifndef _PHONEBK_STUB_H_
#define _PHONEBK_STUB_H_


//******************************************************************************
//	 			include block
//******************************************************************************


//******************************************************************************
//						Global Varilable Declaration
//******************************************************************************
#define MAX_NUMBER_OF_PBK_ENTRY 1000	///< Maximum number of phonebook entries

/* If USE_PBK_STUB_FUNC is defined, bypass the platform phonebook implementation 
 * and use the phonebook stub functions defined by customers in stubs/src/phonebk_stubs.c.
 */
//#define USE_PBK_STUB_FUNC


//******************************************************************************
//					           Type Definitions
//******************************************************************************

/* Phonebook information */
typedef struct
{
	UInt8 mx_alpha_size;	///< Max number of bytes for alpha data
	UInt8 mx_digit_size;	///< Max number of digits
	UInt16 total_entries;	///< Total number of phonebook records
	UInt16 free_entries;	///< Number of free entries in phonebook records
	UInt16 first_free_entry;	///< 0-based index of the first empty phonebook entry
	UInt16 first_used_entry;	///< 0-based index of the first filled phonebook entry
} PBK_STUB_INFO_t;

typedef struct
{
	UInt16 number_of_entry;					///< Number of entries in "index"
	UInt16 index[MAX_NUMBER_OF_PBK_ENTRY];	///< Array of phonebook list
} PBK_STUB_ENTRY_LIST_t;

typedef enum
{
	PBK_SUCCESS,			///< Success in accessing phonebook 
	PBK_NOT_READY,			///< PBK is still being initialized, not ready yet 
	PBK_NOT_SUPPORTED,		///< The requested phonebook type is not supported 
	PBK_NO_ENTRY_FOUND,		///< No appropriate phonebook entry is found 
	PBK_INVALID_INDEX		///< The requested index is invalid 
} PBK_STUB_Result_t;


//******************************************************************************
//						Macro Definition
//******************************************************************************


//******************************************************************************
//						Function Prototype
//******************************************************************************

//**************************************************************************************
/**
    This function returns the configuration information for a phonebook type. 

    @param pbk_id (in) Specify phonebook type
	@param pbk_info (out) Buffer to store the returned phonebook configuration information
	
	@return	Phonebook access result
	@note
**/
PBK_STUB_Result_t PBK_STUB_GetInfo(PBK_Id_t pbk_id, PBK_STUB_INFO_t *pbk_info);


//**************************************************************************************
/**
    This function returns the alpha data of a phonebook record that matches the passed
	phone number. 

    @param pbk_id (in) Specify phonebook type
	@param number (in) NULL terminated phone number in ASCII format
	@param alpha (out) Buffer to store the returned alpha data
	
	@return	Phonebook access result
	@note
**/
PBK_STUB_Result_t PBK_STUB_GetAlphaForMatchedNumber( PBK_Id_t pbk_id, char *number, 
													 PBK_API_Name_t *alpha );


//**************************************************************************************
/**
    This function returns the list of indices of entries that match the passed phonebook
	alpha data.

    @param pbk_id (in) Specify phonebook type
	@param alpha (in) Phonebook alpha data to be matched
	@param entry_list (out) Buffer to store the returned matching 0-based entry indices

	@return	Phonebook access result
	@note
**/
PBK_STUB_Result_t PBK_STUB_GetMatchedAlphaIndexList( PBK_Id_t pbk_id, 
					PBK_API_Name_t *alpha, PBK_STUB_ENTRY_LIST_t *entry_list );


//**************************************************************************************
/**
    This function returns the first index of the phonebook record that matches the passed
	alpha data. 

    @param pbk_id (in) Specify phonebook type
	@param alpha (in) Phonebook alpha data to be matched
	@param index (out) To return the 0-based phonebook index
	
	@return	Phonebook access result
	@note
**/
PBK_STUB_Result_t PBK_STUB_GetMatchedAlphaFirstIndex( PBK_Id_t pbk_id, 
										PBK_API_Name_t *alpha, UInt16 *index );


//**************************************************************************************
/**
    This function returns the contents of a phonebook record

    @param pbk_id (in) Specify phonebook type
	@param index (in) 0-based phonebook index
	@param pbk_rec (out) Buffer to store the returned phonebook contents
	
	@return	Phonebook access result
	@note
**/
PBK_STUB_Result_t PBK_STUB_GetEntryContent( PBK_Id_t pbk_id, UInt16 index, 
										    PBK_Record_t *pbk_rec );


//**************************************************************************************
/**
    This function sends the request to update a phonebook record. A MSG_WRT_PBK_ENTRY_RSP
	message needs to be posted back to the passed callback function.

	@param *inClientInfoPtr (in) ClientInfo_t pointer
	@param pbk_id (in) Specify phonebook type
	@param pbk_rec (in) Phonebook contents to be updated
	@param pbk_access_cb (in) Callback function to post the MSG_WRT_PBK_ENTRY_RSP message
	
	@return	Phonebook access result
	@note
**/
PBK_STUB_Result_t PBK_STUB_SendWriteEntryReq( ClientInfo_t* inClientInfoPtr, PBK_Id_t pbk_id, 
						PBK_Record_t *pbk_rec, CallbackFunc_t* pbk_access_cb );


//**************************************************************************************
/**
    This function whether the passed phone number is in the FDN phonebook.

    @param number (in) NULL terminated phone number in ASCII format to be checked
	@param is_voice_call (in) TRUE if the number is for voice call; FALSE otherwise
	
	@return	TRUE the passed number is in FDN list; FALSE otherwise.
	@note
**/
Boolean	PBK_STUB_IsFdnNumber(const char *number, Boolean is_voice_call);


//**************************************************************************************
/**
    This function checks whether the passed USSD number can be dialled according to FDN setting.

    @param data (in) USSD data to be checked
	@param dcs (in) Data Coding Scheme byte in CB DCS format
	@param len (in) Number of bytes in "data" buffer
	
	@return	TRUE USSD data is diallable; FALSE otherwise
	@note
**/
Boolean PBK_STUB_IsUssdDiallable(const UInt8 *data, UInt8 dcs, UInt8 len);


//**************************************************************************************
/**
    This function checks whether the passed phone number is Emergency Call number.

    @param number (in) NULL terminated phone number in ASCII format
	
	@return	TRUE if the passed phone number is Emergency Call number; FALSE otherwise
	@note
**/
Boolean PBK_STUB_IsEmergencyNumber(const char *number);


//**************************************************************************************
/**
    This function returns the additional Emergency Call numbers in addition to 112, 911 and
	those specified in SIM. 

    @param ecc_num (out) Buffer to store the additional Emergency Call number list
	@param sim_present (in) TRUE if SIM is inserted; FALSE otherwise
	
	@return	Number of additional Emeregency Call numbers returned
	@note
**/
UInt8 PBK_STUB_GetAdditionalEcc(const ECC_NUM_t **ecc_num, Boolean sim_present);

#endif

