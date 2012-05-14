//******************************************************************************
// Copyright 2011 SAMSUNG Corp. All Right Reserved
//
// Description:	This file contains the (MCC, MNC) <--> (PLMN Long Name, PLMN Short Name)
//				lookup table and the relevant lookup functions for SMASUNG Mobile. 
//
//******************************************************************************
/* START : Irine_SunnyVale */

typedef enum samsung_network_type_e
{
   NETWK_TYPE_UNKNOWN_TYPE,
   NETWK_TYPE_GSM_900,
   NETWK_TYPE_DCS_1800,
   NETWK_TYPE_PCS_1900,
   NETWK_TYPE_GSM_SAT,
   NETWK_TYPE_GSM_850,
   NETWK_TYPE_UMTS
} network_type_e_type;

/*
** Define a type that contains the networks, country code, network code
** and name.
*/
typedef struct samsung_network_info_s
{

   UInt16                  mcc;
     /* Mobile Network Code                                */

   UInt16                  mnc;
    /* Mobile Country Code                                 */

   network_type_e_type   network_type;

   char                         *short_name_ptr;
     /* Pointer to a null terminated string containing the */
     /* network's short name.                              */

   char                         *full_name_ptr;
     /* Pointer to a null terminated string containing the */
     /* network's full name.                               */

} network_info_s_type;



static const network_info_s_type Plmn_Table_Samsung[] =
{

/***********************
 **** Test PLMN 1-1 ****
 ***********************/
/* Test PLMN 1-1 */
{ 001,   1, NETWK_TYPE_GSM_900, "Test1-1", "Test PLMN 1-1" },

/***********************
 **** Test PLMN 1-2 ****
 ***********************/
/* Test PLMN 1-2 */
{ 001,   2, NETWK_TYPE_GSM_900, "Test1-2", "Test PLMN 1-2" },

/***********************
 **** Test PLMN 2-1 ****
 ***********************/
/* Test PLMN 2-1 */
{ 002,   1, NETWK_TYPE_GSM_900, "Test2-1", "Test PLMN 2-1" },

#if !defined (FEATURE_SAMSUNG_SOUTHEAST_ASIA)
/****************
 **** Greece ****
 ****************/
/* COSMOTE */
{ 202,   1, NETWK_TYPE_GSM_900, "COSMOTE", "COSMOTE" },//[2007.10.02]sungeun.lim 수출부에서 입수한 사업자 요청 및 현채인 확인 최종 PLMN name으로 update.
/* COSMOTE */
{ 202,   1, NETWK_TYPE_DCS_1800, "COSMOTE", "COSMOTE" },//[2007.10.02]sungeun.lim 수출부에서 입수한 사업자 요청 및 현채인 확인 최종 PLMN name으로 update.
/* Vodafone */
{ 202,   5, NETWK_TYPE_GSM_900, "vodafone GR", "vodafone GR" },
/* Info Quest - Commercial & Industrial SA */
{ 202,   9, NETWK_TYPE_DCS_1800, "n/a", "n/a" }, //[2007.10.02]sungeun.lim 수출부에서 입수한 사업자 요청 및 현채인 확인 최종 PLMN name으로 update.
/* STET HELLAS */
{ 202,  10, NETWK_TYPE_GSM_900, "WIND GR", "WIND GR" }, //[2007.10.02]sungeun.lim 수출부에서 입수한 사업자 요청 및 현채인 확인 최종 PLMN name으로 update.

/*********************
 **** Netherlands ****
 *********************/
/* Vodafone Libertel N.V */
{ 204,   4, NETWK_TYPE_GSM_900, "vodafone NL", "vodafone NL" },
/* Vodafone Libertel N.V */
{ 204,   4, NETWK_TYPE_DCS_1800, "vodafone NL", "vodafone NL" },
/* KPN Mobile The Netherlands BV */
{ 204,   8, NETWK_TYPE_GSM_900, "NL KPN", "NL KPN" },
/* KPN Mobile The Netherlands BV */
{ 204,   8, NETWK_TYPE_DCS_1800, "NL KPN", "NL KPN" },
/* Telfort Mobiel B.V. */
{ 204,  12, NETWK_TYPE_DCS_1800, "NL Telfort", "NL Telfort" },
/* T-Mobile Netherlands */
{ 204,  16, NETWK_TYPE_DCS_1800, "T-Mobile NL", "T-Mobile NL" },
/* Orange Nederland N.V. */
{ 204,  20, NETWK_TYPE_DCS_1800, "Orange NL", "Orange NL" },

/*****************
 **** Belgium ****
 *****************/
/* Belgacom Mobile */
{ 206,   1, NETWK_TYPE_GSM_900, "PROXIMUS", "PROXIMUS" },
/* Belgacom Mobile */
{ 206,   1, NETWK_TYPE_DCS_1800, "PROXIMUS", "PROXIMUS" },
/* Mobistar S.A. */
{ 206,  10, NETWK_TYPE_GSM_900, "MOBISTAR", "MOBISTAR" },
/* Mobistar S.A. */
{ 206, 10, NETWK_TYPE_DCS_1800, "MOBISTAR", "MOBISTAR" },
/* Mobistar S.A. */
{ 206, 10, NETWK_TYPE_UMTS, "MOBISTAR", "MOBISTAR" }, 
/* KPN Orange Belgium NV/SA */
{ 206,  20, NETWK_TYPE_DCS_1800, "BASE", "BASE" },

/****************
 **** France ****
 ****************/
/* Orange France */
{ 208,   1, NETWK_TYPE_GSM_900, "Orange F", "Orange F" },
/* Orange France */
{ 208,   1, NETWK_TYPE_DCS_1800, "Orange F", "Orange F" },
/* Orange France (Contact)*/
{ 208,   2, NETWK_TYPE_GSM_900, "F - Contact", "F - Contact" },
/* Orange France (Contact)*/
{ 208,   2, NETWK_TYPE_DCS_1800, "F - Contact", "F - Contact" },
/* SFR */
{ 208,  10, NETWK_TYPE_GSM_900, "F SFR", "F SFR" },
/* SFR */
{ 208,  10, NETWK_TYPE_DCS_1800, "F SFR", "F SFR" },
/* Orange France (Contact)*/
{ 208,   13, NETWK_TYPE_DCS_1800, "F - Contact", "F - Contact" },
/* Bouygues Telecom */
{ 208,  20, NETWK_TYPE_GSM_900, "BOUYGTEL", "BOUYGTEL" },
/* Bouygues Telecom */
{ 208,  20, NETWK_TYPE_DCS_1800, "BOUYGTEL", "BOUYGTEL" },
/* Orange France (Contact)*/
{ 208,   88, NETWK_TYPE_DCS_1800, "F - Contact", "F - Contact" },

/*****************
 **** Andorra ****
 *****************/
/* Mobiland */
{ 213,   3, NETWK_TYPE_GSM_900, "STA-MOBILAND", "STA-MOBILAND" },

/***************
 **** Spain ****
 ***************/
/* Airtel Movil S.A */
{ 214,  1, NETWK_TYPE_GSM_900, "vodafone ES", "vodafone ES" },
/* Airtel Movil S.A */
{ 214,   1, NETWK_TYPE_DCS_1800, "vodafone ES", "vodafone ES" },
/* Telefonica Moviles */

{ 214,   2, NETWK_TYPE_GSM_900, "movistar", "movistar" },
/* Telefonica Moviles */
{ 214,   2, NETWK_TYPE_DCS_1800, "movistar", "movistar" },

/* Retevision Movil S.A */
{ 214,   3, NETWK_TYPE_GSM_900, "Orange", "Orange" },
/* Retevision Movil S.A */
{ 214,   3, NETWK_TYPE_DCS_1800, "Orange", "Orange" },
/* Retevision Movil S.A */
{ 214,   3, NETWK_TYPE_UMTS, "Orange", "Orange" },  /* refer to selist20080115.xls for Orange [jm0122.yoon 20080122] */
/* Xfera Moviles SA */
{ 214,   4, NETWK_TYPE_UMTS, "Yoigo", "Yoigo" },
/* Euskaltel */
{ 214,   6, NETWK_TYPE_GSM_900, "EUSKALTEL", "EUSKALTEL" },/* added for 214 8 display problem for Euskaltel*/
/* Telefonica Moviles */
{ 214,   7, NETWK_TYPE_GSM_900, "movistar", "movistar" },
/* Telefonica Moviles */
{ 214,   7, NETWK_TYPE_DCS_1800, "movistar", "movistar" },
/* Euskaltel */
{ 214,   8, NETWK_TYPE_GSM_900, "EUSKALTEL", "EUSKALTEL" },/* added for 214 8 display problem for Euskaltel*/
/* Telefonica Moviles */
{ 214,  77, NETWK_TYPE_DCS_1800, "movistar", "movistar" },/* added for 214 77 display problem for Movistar*/

/*****************
 **** Hungary ****
 *****************/
/* Mobile Telecommunications */  //2010.05.10 The PLMN name is changed as the operator request.
{ 216,   1, NETWK_TYPE_GSM_900, "Telenor HU", "Telenor HU" },
/* Mobile Telecommunications */  //2010.05.10 The PLMN name is changed as the operator request.
{ 216,   1, NETWK_TYPE_DCS_1800, "Telenor HU", "Telenor HU" },
/* T-Mobile Hungary Telecommunications Co. Ltd */
{ 216,  30, NETWK_TYPE_GSM_900, "T-Mobile H", "T-Mobile H" },
/* T-Mobile Hungary Telecommunications Co. Ltd */
{ 216,  30, NETWK_TYPE_DCS_1800, "T-Mobile H", "T-Mobile H" },
/* Vodafone Hungary Ltd */
{ 216,  70, NETWK_TYPE_GSM_900, "vodafone HU", "vodafone HU" },
/* Vodafone Hungary Ltd */
{ 216,  70, NETWK_TYPE_DCS_1800, "vodafone HU", "vodafone HU" },

/****************
 **** Bosnia ****
 ****************/
/* Eronet Mobile Communications Ltd */
{ 218,   3, NETWK_TYPE_GSM_900, "HT Eronet", "HT Eronet" },//[2007.10.02]sungeun.lim 수출부에서 입수한 사업자 요청 및 현채인 확인 최종 PLMN name으로 update.
/* RS Telecommunications  JSC Banja Luka */
{ 218,   5, NETWK_TYPE_GSM_900, "m:tel", "m:tel" },//[2007.10.02]sungeun.lim 수출부에서 입수한 사업자 요청 및 현채인 확인 최종 PLMN name으로 update.
/* Public Enterprise BH Telecom Sarajevo */
{ 218,  90, NETWK_TYPE_GSM_900, "BH Mobile", "BH Mobile" },//[2007.10.02]sungeun.lim 수출부에서 입수한 사업자 요청 및 현채인 확인 최종 PLMN name으로 update.

/*****************
 **** Croatia ****
 *****************/
/* HT Mobile Communications, LLC */
{ 219,   1, NETWK_TYPE_GSM_900, "T-Mobile HR", "T-Mobile HR" },
/* Tele2 d.o.o za telekomunikacijske uluge */
{ 219, 2, NETWK_TYPE_DCS_1800, "TELE2", "TELE2" }, 
/* Vip-NET GSM d.o.o. */
{ 219,  10, NETWK_TYPE_GSM_900, "HR-VIP", "HR-VIP" },//[2009.01.21]sun.mi.jung 왑사양서의 PLMN name으로 수정.

/********************
 **** Yugoslavia ****
 ********************/
/* Mobile Telecommunications */  //[proto]yjm0122 사업자 요청으로 diplay변경
{ 220,   1, NETWK_TYPE_GSM_900, "Telenor", "Telenor" },
/* Mobile Telecommunications */
{ 220,   1, NETWK_TYPE_DCS_1800, "Telenor", "Telenor" },
/* ProMonte GSM */
{ 220,   2, NETWK_TYPE_GSM_900, "Promonte", "Promonte" },
/* ProMonte GSM */
{ 220,   2, NETWK_TYPE_DCS_1800, "Promonte", "Promonte" },
/* Telekom Srbija a.d. */
{ 220,   3, NETWK_TYPE_GSM_900, "mt:s", "mt:s" },
/* Monet D.O.O */
{ 220,   4, NETWK_TYPE_GSM_900, "T-Mobile", "T-Mobile" },
/* Vip mobile d.o.o. */
{ 220,   5, NETWK_TYPE_GSM_900, "Vip SRB", "Vip SRB" },
/* Vip mobile d.o.o. */
{ 220,   5, NETWK_TYPE_DCS_1800, "Vip SRB", "Vip SRB" },
/* Vip mobile d.o.o. */
{ 220,   5, NETWK_TYPE_UMTS, "Vip SRB", "Vip SRB" },
/***************
 **** Italy ****
 ***************/
/* Telecom Italia Mobile */
{ 222,   1, NETWK_TYPE_GSM_900, "I TIM", "I TIM" },
/* Telecom Italia Mobile */
{ 222,   1, NETWK_TYPE_DCS_1800, "I TIM", "I TIM" },
/* Omnitel Pronto Italia is Vodafone IT now */
{ 222,  10, NETWK_TYPE_GSM_900, "vodafone IT", "vodafone IT" },
/* Omnitel Pronto Italia is Vodafone IT now*/
{ 222,  10, NETWK_TYPE_DCS_1800, "vodafone IT", "vodafone IT" },
/* Wind Telecomunicazioni SpA */
{ 222,  88, NETWK_TYPE_GSM_900, "I WIND", "I WIND" },
/* Wind Telecomunicazioni SpA*/
{ 222,  88, NETWK_TYPE_DCS_1800, "I WIND", "I WIND" },
/* Wind Telecomunicazioni SpA*/
{ 222,  88, NETWK_TYPE_UMTS, "I WIND", "I WIND" },
/* Blu SpA */
{ 222,  98, NETWK_TYPE_DCS_1800, "IT Blu", "IT Blu" },
/* Hutchison 3 */
{ 222,  99, NETWK_TYPE_UMTS, "3 ITA", "3 ITA" },//2009.01.19 ECIM 2141589 H3G -> 3 ITA

/*****************
 **** Romania ****
 *****************/
/* MobiFon S.A */
{ 226,   1, NETWK_TYPE_GSM_900, "Vodafone RO", "Vodafone RO" },
/* Cosmote Romanian Mobile Telecommunications S.A. */
{ 226,   3, NETWK_TYPE_GSM_900, "COSMOTE", "COSMOTE" },
/* Cosmote Romanian Mobile Telecommunications S.A. */
{ 226,   3, NETWK_TYPE_DCS_1800, "COSMOTE", "COSMOTE" },
/* Orange Romania SA */
{ 226,	10, NETWK_TYPE_GSM_900, "RO ORANGE", "RO ORANGE" },  /* 2010. 07. 20 Request from Ociavian Vlad via PL. */
/* Orange Romania SA */
{ 226,	10, NETWK_TYPE_DCS_1800, "RO ORANGE", "RO ORANGE" }, /* 2010. 07. 20 Request from Ociavian Vlad via PL. */

/*********************
 **** Switzerland ****
 *********************/
/* Swisscom Mobile Ltd */
{ 228,   1, NETWK_TYPE_GSM_900, "Swisscom", "Swisscom" },//[2007.09.19]wonki.ha 검증팀 요청에 의해 소문자로 수정
/* Swisscom Mobile Ltd */
{ 228,   1, NETWK_TYPE_DCS_1800, "Swisscom", "Swisscom" },//[2007.09.19]wonki.ha 검증팀 요청에 의해 소문자로 수정
/* TDC Switzerland AG */
{ 228,   2, NETWK_TYPE_GSM_900, "Sunrise", "Sunrise" }, //2008.12.02 sunrise->Sunrise 현채인Yves Piccand [y.piccand@samsung.com] 요청사항
/* TDC Switzerland AG */
{ 228,   2, NETWK_TYPE_DCS_1800, "Sunrise", "Sunrise" }, //2008.12.02 sunrise->Sunrise 현채인Yves Piccand [y.piccand@samsung.com] 요청사항
/* Orange Communications S.A */
{ 228,   3, NETWK_TYPE_DCS_1800, "orange CH", "orange CH" },//[2007.09.19]wonki.ha 검증팀 요청에 의해 대문자로  수정
/* In &Phone SA */
{ 228, 7, NETWK_TYPE_DCS_1800, "In&Phone", "In&Phone" }, 
/* Tele2 Telecommunications Services AG */
{ 228, 8, NETWK_TYPE_DCS_1800, "Tele2", "Tele2" }, //[2007.09.19]wonki.ha 검증팀 요청에 의해 T2 에서 수정 

/************************
 **** Czech Republic ****
 ************************/
/* T- Mobile Networks */
{ 230,   1, NETWK_TYPE_GSM_900, "T-Mobile CZ", "T-Mobile CZ" },
/* T- Mobile Networks */
{ 230,   1, NETWK_TYPE_DCS_1800, "T-Mobile CZ", "T-Mobile CZ" },
/* EuroTel Praha Spol. sro */
{ 230,   2, NETWK_TYPE_GSM_900, "O2 - CZ", "O2 - CZ" },
/* EuroTel Praha Spol. sro */
{ 230,   2, NETWK_TYPE_DCS_1800, "O2 - CZ", "O2 - CZ" },
/* Cesky Mobil a.s */
{ 230,   3, NETWK_TYPE_GSM_900, "Vodafone CZ", "Vodafone CZ" }, //[2007.10.02]sungeun.lim 수출부에서 입수한 사업자 요청 및 현채인 확인 최종 PLMN name으로 update.
/* Cesky Mobil a.s */
{ 230,   3, NETWK_TYPE_DCS_1800, "Vodafone CZ", "Vodafone CZ" }, //[2007.10.02]sungeun.lim 수출부에서 입수한 사업자 요청 및 현채인 확인 최종 PLMN name으로 update.

/*************************
 **** Slovak Republic ****
 *************************/
/* Orange Slovensko a.s. */
{ 231,   1, NETWK_TYPE_GSM_900, "Orange", "Orange" },  //2008.12.02 현채인 요청사항 Orange SK->Orange
/* Orange Slovensko a.s. */
{ 231,   1, NETWK_TYPE_DCS_1800, "Orange", "Orange" },    //2008.12.02 /* refer to selist20080115.xls for Orange [jm0122.yoon 20080122] */
/* EuroTel Bratislava a.s */
{ 231,   2, NETWK_TYPE_GSM_900, "T-Mobile SK", "T-Mobile SK" },
/* EuroTel Bratislava a.s */
{ 231,   2, NETWK_TYPE_DCS_1800, "T-Mobile SK", "T-Mobile SK" },
/* EuroTel Bratislava a.s */
{ 231,   4, NETWK_TYPE_UMTS, "T-Mobile SK", "T-Mobile SK" },
/* Orange Slovensko a.s. */
{ 231,   5, NETWK_TYPE_DCS_1800, "Orange", "Orange" }, //2008.12.02 현채인 요청사항 Orange SK->Orange  from Peter Makai [peter.makai@samsung.com]
/* Telefonica O2 Slovakia s.r.o. */
{ 231,   6, NETWK_TYPE_GSM_900, "O2 - SK", "O2 - SK" },
/* Telefonica O2 Slovakia s.r.o. */
{ 231,   6, NETWK_TYPE_DCS_1800, "O2 - SK", "O2 - SK" },

/*****************
 **** Austria ****
 *****************/
/* MobilKom Austria AG */
{ 232,   1, NETWK_TYPE_GSM_900, "A1", "A1" },
/* MobilKom Austria AG */
{ 232,   1, NETWK_TYPE_DCS_1800, "A1", "A1" },
/* MobilKom Austria AG */
{ 232,   1, NETWK_TYPE_UMTS, "A1", "A1" },
/* T-Mobile Austria GmbH */
{ 232,   3, NETWK_TYPE_GSM_900, "T-Mobile A", "T-Mobile A" },
/* T-Mobile Austria GmbH */
{ 232,   3, NETWK_TYPE_DCS_1800, "T-Mobile A", "T-Mobile A" },
/* Connect Austria */
{ 232,   5, NETWK_TYPE_GSM_900, "Orange AT", "Orange AT" },
/* Connect Austria */
{ 232,   5, NETWK_TYPE_DCS_1800, "Orange AT", "Orange AT" },
/* Connect Austria */
{ 232,   5, NETWK_TYPE_UMTS, "Orange AT", "Orange AT" },
/* Tele.ring Telekom Service GmbH & Co KG */
{ 232,   7, NETWK_TYPE_DCS_1800, "tele.ring", "tele.ring" },
/* Hutchison 3G Austria GmbH */ 
{ 232,   10, NETWK_TYPE_UMTS, "3", "3" },
/* bob */
{ 232,	 11, NETWK_TYPE_GSM_900, "bob", "bob" }, // 2009.04.09 Requirement Austria
/* bob */
{ 232,	 11, NETWK_TYPE_DCS_1800, "bob", "bob" }, // 2009.04.09 Requirement Austria
/* bob */
{ 232,	 11, NETWK_TYPE_UMTS, "bob", "bob" }, // 2009.04.09 Requirement Austria
/* Connect Austria */
{ 232,   12, NETWK_TYPE_GSM_900, "Orange A", "Orange AT" },// D0100306029 2011/9/16 Orange 사업자 요청
/* Connect Austria */
{ 232,   12, NETWK_TYPE_DCS_1800, "Orange A", "Orange AT" },// D0100306029 2011/9/16 Orange 사업자 요청
/* Connect Austria */
{ 232,   12, NETWK_TYPE_UMTS, "Orange A", "Orange AT" },// D0100306029 2011/9/16 Orange 사업자 요청


/************************
 **** United Kingdom ****
 ************************/
/* BTCellnet */
{ 234,  10, NETWK_TYPE_GSM_900, "O2 - UK", "O2 - UK" },
/* BTCellnet */
{ 234,  10, NETWK_TYPE_DCS_1800, "O2 - UK", "O2 - UK" },
/* O2 (UK) Limited */
{ 234, 10, NETWK_TYPE_UMTS, "O2 - UK ", "O2 - UK" }, 
/* Vodafone Ltd */
{ 234,  15, NETWK_TYPE_GSM_900, "vodafone UK", "vodafone UK" },/* changed to vodafone UK */
/* Vodafone Ltd */
{ 234,  15, NETWK_TYPE_DCS_1800, "vodafone UK", "vodafone UK" },
/*  Hutchinson UK */
{ 234,  20, NETWK_TYPE_UMTS, "3 UK", "3 UK" },
/*  T-Mobile (UK) Limited */
{ 234,  30, NETWK_TYPE_DCS_1800, "T-Mobile UK", "T-Mobile UK" },
/*  T-Mobile (UK) Limited */
{ 234,  31, NETWK_TYPE_DCS_1800, "T-Mobile UK", "T-Mobile UK" },
/*  T-Mobile (UK) Limited */
{ 234,  32, NETWK_TYPE_DCS_1800, "T-Mobile UK", "T-Mobile UK" },
/* Orange PCS Ltd */
{ 234,  33, NETWK_TYPE_DCS_1800, "Orange", "Orange" },
/* Jersey Telecoms */
{ 234,  50, NETWK_TYPE_GSM_900, "JT GSM", "JT GSM" },
/* Wave Telecom */
{ 234, 50, NETWK_TYPE_DCS_1800, "JT GSM", "JT GSM" }, 
/* Wave Telecom */
{ 234, 50, NETWK_TYPE_UMTS, "JT-GSM", "JT-GSM" }, 
/* Cable & Wireless Guernsey Ltd */
{ 234,  55, NETWK_TYPE_GSM_900, "Cable & Wireless Guernsey", "Cable & Wireless Guernsey" },
/* Manx Telecom */
{ 234,  58, NETWK_TYPE_GSM_900, "Manx Pronto", "Manx Pronto" },

/*****************
 **** Denmark ****
 *****************/
/* TDC Mobil A/S */
{ 238,   1, NETWK_TYPE_GSM_900, "TDC MOBIL", "TDC MOBIL" },
/* TDC Mobil A/S */
{ 238,   1, NETWK_TYPE_DCS_1800, "TDC MOBIL", "TDC MOBIL" },
/* Telenor DK */
{ 238,   2, NETWK_TYPE_GSM_900, "Telenor DK", "Telenor DK" },
/* Telenor DK */
{ 238,   2, NETWK_TYPE_DCS_1800, "Telenor DK", "Telenor DK" },
/* HiNETWK_TYPE_UMTS Denmark ApS */
{ 238, 6, NETWK_TYPE_UMTS, "3", "3" }, 
/* Telia A/S Denmark */
{ 238,  20, NETWK_TYPE_DCS_1800, "TELIA DK", "TELIA DK" },
/* Orange A/S */
{ 238,  30, NETWK_TYPE_GSM_900, "Orange", "Orange" },
/* Orange A/S */
{ 238,  30, NETWK_TYPE_DCS_1800, "Orange", "Orange" },
/* Telenor DK */
{ 238,   77, NETWK_TYPE_GSM_900, "Telenor DK", "Telenor DK" }, // 2009.06.18 Updated issuelist for Jet Qubick 31
/* Telenor DK */
{ 238,   77, NETWK_TYPE_DCS_1800, "Telenor DK", "Telenor DK" }, // 2009.06.18 Updated issuelist for Jet Qubick 31
/* Telenor DK */
{ 238,	 77, NETWK_TYPE_UMTS, "Telenor DK", "Telenor DK" }, // 2009.06.18 Updated issuelist for Jet Qubick 31


/****************
 **** Sweden ****
 ****************/
/* Telia AB */
{ 240,   1, NETWK_TYPE_GSM_900, "S Telia", "S Telia" },
/* Telia AB */
{ 240,   1, NETWK_TYPE_DCS_1800, "S Telia", "S Telia" },
/* Hi3G Access AB */
{ 240,   2, NETWK_TYPE_UMTS, "3", "3" },
/* Orange Sverige AB */
{ 240,   3, NETWK_TYPE_UMTS, "Orange", "Orange" },
/* HI3G Access AB */
{ 240,   4, NETWK_TYPE_UMTS, "vodafone SE", "vodafone SE" },
/* COMVIQ GSM shared with TeliaSonera MobileNetworks */
{ 240,   5, NETWK_TYPE_UMTS, "Sweden 3G", "Sweden 3G" },
/* Tele 2 AB */
{ 240,   7, NETWK_TYPE_GSM_900, "Tele2comviq", "Tele2comviq" },   //2008.12.02 S COMVIQ-> Tele2comviq 현채인Andreas Bergqvist [andreas.bergqvist@samsung.se] 요청
/* Tele 2 AB */
{ 240,   7, NETWK_TYPE_DCS_1800, "Tele2comviq", "Tele2comviq" },  //2008.12.02 
/* Europolitan AB */
{ 240,   8, NETWK_TYPE_GSM_900, "Telenor SE", "Telenor SE" },
/* Europolitan AB */
{ 240,   8, NETWK_TYPE_DCS_1800, "Telenor SE", "Telenor SE" },
/* Europolitan AB */
{ 240,   8, NETWK_TYPE_UMTS, "Telenor SE", "Telenor SE" },
/* Swefour AB */
{ 240,   10, NETWK_TYPE_GSM_900, "SpringMobil SE", "SpringMobil SE" },
/* Swefour AB */
{ 240,   10, NETWK_TYPE_DCS_1800, "SpringMobil SE", "SpringMobil SE" },

/****************
 **** Norway ****
 ****************/
/* Telenor Mobil AS */
{ 242,   1, NETWK_TYPE_GSM_900, "N Telenor", "N Telenor" },
/* Telenor Mobil AS */
{ 242,   1, NETWK_TYPE_DCS_1800, "N Telenor", "N Telenor" },
/* NETCOM AS */
{ 242,   2, NETWK_TYPE_GSM_900, "N NetCom GSM", "N NetCom GSM" },
/* NETCOM AS */
{ 242,   2, NETWK_TYPE_DCS_1800, "N NetCom GSM", "N NetCom GSM" },
/* Teletopia Mobile Communications AS */
{ 242, 3, NETWK_TYPE_DCS_1800, "T3", "T3" }, 
/* Netwrok Norway */
{ 242, 5, NETWK_TYPE_GSM_900, "Netwrok Norway", "Netwrok Norway" }, 
/* Netwrok Norway */
{ 242, 5, NETWK_TYPE_DCS_1800, "Netwrok Norway", "Netwrok Norway" }, 
/* Netwrok Norway */
{ 242, 5, NETWK_TYPE_UMTS, "Netwrok Norway", "Netwrok Norway" },  

/*****************
 **** Finland ****
 *****************/
/* Telia Mobile AB */
{ 244,   3, NETWK_TYPE_DCS_1800, "FI TELIA", "FI TELIA" },
/* Elisa Corporation */
{ 244,   5, NETWK_TYPE_GSM_900, "Elisa", "Elisa" },
/* Elisa Corporation */
{ 244,   5, NETWK_TYPE_DCS_1800, "Elisa", "Elisa" },
/* Elisa Corporation */
{ 244, 5, NETWK_TYPE_UMTS, "Elisa", "Elisa" }, 
/* Finnet Group */
{ 244,   9, NETWK_TYPE_DCS_1800, "Finnet", "Finnet" },
/* Finnet Networks Ltd */
{ 244,  12, NETWK_TYPE_GSM_900, "FI 2G", "FI 2G" },
/* Finnet Networks Ltd */
{ 244,  12, NETWK_TYPE_DCS_1800, "FI 2G", "FI 2G" },
/* Finnet Networks Ltd */
{ 244,  12, NETWK_TYPE_UMTS, "FI 2G", "FI 2G" },
/* Alands Mobiltelefon A.B */
{ 244,  14, NETWK_TYPE_GSM_900, "FI AMT", "FI AMT" },
/* Sonera Mobile Networks Limited */
{ 244,  91, NETWK_TYPE_GSM_900, "FI SONERA", "FI SONERA" },
/* Sonera Mobile Networks Limited */
{ 244,  91, NETWK_TYPE_DCS_1800, "FI SONERA", "FI SONERA" },
/* Sonera Mobile Networks Limited */
{ 244,  91, NETWK_TYPE_UMTS, "FI SONERA", "FI SONERA" },

/*******************
 **** Lithuania ****
 *******************/
/* Omnitel */
{ 246,   1, NETWK_TYPE_GSM_900, "LT Omnitel", "LT Omnitel" },
/* UAB Bite GSM */
{ 246,   2, NETWK_TYPE_GSM_900, "LT BITE", "LT BITE" },		// 2008.06.11 kbs@CIS
/* UAB Bite GSM */
{ 246,   2, NETWK_TYPE_DCS_1800, "LT BITE", "LT BITE" },		// 2008.06.11 kbs@CIS
/* UAB TELE2 */
{ 246,   3, NETWK_TYPE_GSM_900, "LT TELE2", "LT TELE2" },		// 2008.06.11 kbs@CIS
/* UAB TELE2 */
{ 246,   3, NETWK_TYPE_DCS_1800, "LT TELE2", "LT TELE2" },		// 2008.06.11 kbs@CIS

/****************
 **** Latvia ****
 ****************/
/* Latvian Mobile Tel. Co. */
{ 247,   1, NETWK_TYPE_GSM_900, "LMT GSM", "LMT GSM" },
/* Latvian Mobile Tel. Co. */
{ 247,   1, NETWK_TYPE_DCS_1800, "LMT GSM", "LMT GSM" },
/* TELE2 */
{ 247,   2, NETWK_TYPE_GSM_900, "LV TELE2", "LV TELE2" },
/* TELE2 */
{ 247,   2, NETWK_TYPE_DCS_1800, "LV TELE2", "LV TELE2" },
/* LV BITE */
{ 247,   5, NETWK_TYPE_GSM_900, "LV BITE", "LV BITE"},
/* SIA Bite Latvija */
{ 247, 5, NETWK_TYPE_DCS_1800, "LV BITE", "LV BITE" }, 
/* SIA Bite Latvija */
{ 247, 5, NETWK_TYPE_UMTS, "LV BITE", "LV BITE" }, 

/*****************
 **** Estonia ****
 *****************/
/* AS EMT */
{ 248,   1, NETWK_TYPE_GSM_900, "EE EMT", "EE EMT" },
/* AS EMT */
{ 248,   1, NETWK_TYPE_DCS_1800, "EE EMT", "AS EMT" },			// 2008.06.11 kbs@CIS
/* Radiolinja Eesti AS */
{ 248,   2, NETWK_TYPE_GSM_900, "EE ELISA", "EE ELISA" },	// 2008.06.11 kbs@CIS
/* Radiolinja Eesti AS */
{ 248,   2, NETWK_TYPE_DCS_1800, "EE ELISA", "EE ELISA" },	// 2008.06.11 kbs@CIS
/* Tele2 Eesti AS */
{ 248,   3, NETWK_TYPE_GSM_900, "EE TELE2", "EE TELE2" },		// 2008.06.11 kbs@CIS
/* Tele2 Eesti AS */
{ 248,   3, NETWK_TYPE_DCS_1800, "EE TELE2", "EE TELE2" },	// 2008.06.11 kbs@CIS

/****************
 **** Russia ****
 ****************/
 //S/W 기술개발 그룹 이영훈 사원의 요청에 의해 2007.08.20 수정 - Hongs
/* Mobile Telesystems */
{ 250,   1, NETWK_TYPE_GSM_900, "MTS", "MTS" },
/* North-West GSM */
{ 250,   2, NETWK_TYPE_GSM_900, "Megafon", "Megafon" },
/* North-West GSM */
{ 250,   2, NETWK_TYPE_DCS_1800, "Megafon", "Megafon" },
/* Nizhegorodskaya Cellular Communications */
{ 250,   3, NETWK_TYPE_GSM_900, "NCC", "NCC" },
/* SIBCHALLENGE LTD */
{ 250,   4, NETWK_TYPE_GSM_900, "SIBCHALLENGE RUS", "SIBCHALLENGE RUS" },
/* Siberian Cellular Systems-900 */
{ 250,   5, NETWK_TYPE_GSM_900, "ETK", "ETK" },
/* Zao Smarts */
{ 250,   7, NETWK_TYPE_GSM_900, "SMARTS", "SMARTS" },
/* Zao Smarts */
{ 250,   7, NETWK_TYPE_DCS_1800, "SMARTS", "SMARTS" },
/* DonTeleCom */
{ 250,  10, NETWK_TYPE_GSM_900, "DTC", "DTC" },
/* Joint-stock company - Orensot */
{ 250,  11, NETWK_TYPE_GSM_900, "Orensot", "Orensot" },
/* Far Eastern Cellular Systems - 900 */
{ 250,  12, NETWK_TYPE_GSM_900, "Far East", "Far East" },
/* Kuban- GSM Closed JSC */
{ 250,  13, NETWK_TYPE_GSM_900, "Kuban GSM", "Kuban GSM" },
/* ZAO SMARTS */
{ 250, 15, NETWK_TYPE_DCS_1800, "SMARTS", "SMARTS" }, 
/* New Telephone Company */
{ 250,  16, NETWK_TYPE_GSM_900, "NTC", "NTC" },
/* Ermak RMS */
{ 250,  17, NETWK_TYPE_GSM_900, "Utel", "Utel" },
/* Bashcell, CJSC */
{ 250,  19, NETWK_TYPE_DCS_1800, "RUS_BASHCELL", "RUS_BASHCELL" },
/* CJSC Volgograd Mobile */
{ 250,  19, NETWK_TYPE_DCS_1800, "RUS INDIGO", "RUS INDIGO" },
/* Cellular Communications of Udmurtia */
{ 250,  20, NETWK_TYPE_DCS_1800, "TELE2", "TELE2" },
/* JSC Extel */
{ 250,  28, NETWK_TYPE_GSM_900, "Beeline", "Beeline" },
/* South Ural Cellular Telephone */
{ 250,  39, NETWK_TYPE_GSM_900, "Utel", "Utel" },
/* StavTeleSot */
{ 250,  44, NETWK_TYPE_GSM_900, "NC-GSM", "NC-GSM" },
/* Megafon */
{ 250,  91, NETWK_TYPE_GSM_900, "Megafon", "Megafon" },
/* Megafon */
{ 250,  91, NETWK_TYPE_DCS_1800, "Megafon", "Megafon" },
/* Primtelefon */
{ 250,  92, NETWK_TYPE_GSM_900, "Primtel", "Primtel" },
/* Primtelefon */
{ 250,  92, NETWK_TYPE_DCS_1800, "Primtel", "Primtel" },
/* Telecom XXI JSC */
{ 250,  93, NETWK_TYPE_GSM_900, "JSC Telecom XXI", "JSC Telecom XXI" },
/* Telecom XXI JSC */
{ 250,  93, NETWK_TYPE_DCS_1800, "JSC telecom XXI", "JSC telecom XXI" },
/* KB Impuls */
{ 250,  99, NETWK_TYPE_GSM_900, "Beeline", "Beeline" },	// 2008.06.11 kbs@CIS
/* KB Impuls */
{ 250,  99, NETWK_TYPE_DCS_1800, "Beeline", "Beeline" },// 2008.06.11 kbs@CIS

/*****************
 **** Ukraine ****
 *****************/
/* Ukrainian Mobile Comms */
{ 255,   1, NETWK_TYPE_GSM_900, "MTS UKR", "MTS UKR" },	// 2008.06.11 kbs@CIS
/* Ukrainian Mobile Comms */
{ 255,   1, NETWK_TYPE_DCS_1800, "MTS UKR", "MTS UKR" },// 2008.06.11 kbs@CIS
/* Ukrainian Radio Systems */
{ 255,   2, NETWK_TYPE_GSM_900, "Beeline UA", "Beeline UA" },// 2008.06.11 kbs@CIS
/* Ukrainian Radio Systems */
{ 255,   2, NETWK_TYPE_DCS_1800, "Beeline UA", "Beeline UA" },// 2008.06.11 kbs@CIS
/* Kyivstar GSM JSC */
{ 255,   3, NETWK_TYPE_GSM_900, "Kyivstar", "Kyivstar" },// 2008.06.11 kbs@CIS
/* Kyivstar GSM JSC */
{ 255,   3, NETWK_TYPE_DCS_1800, "Kyivstar", "Kyivstar" },// 2008.06.11 kbs@CIS
/* Golden Telecom LLC */
{ 255,   5, NETWK_TYPE_DCS_1800, "GT", "GT" },	// 2008.06.11 kbs@CIS
/* Astelit LLC */
{ 255, 6, NETWK_TYPE_DCS_1800, "Life:)", "life:)" }, 							// 2008.06.11 kbs@CIS
/* Ukrainian New 3G network provider */  //[20071121 jm0122.yoon]사업자요구사항으로 추가
{ 255, 7, NETWK_TYPE_UMTS, "Utel", "Utel" }, 

/*****************
 **** Belarus ****
 *****************/
/* Mobile Digital Communications */
{ 257,   1, NETWK_TYPE_GSM_900, "BY VELCOM", "BY VELCOM" },
/* Mobile Digital Communications */
{ 257,   1, NETWK_TYPE_DCS_1800, "BY VELCOM", "BY VELCOM" },
/* JLLC Mobile TeleSystems  */
{ 257,   2, NETWK_TYPE_GSM_900, "MTS BY", "MTS BY" },
/* JLLC Mobile TeleSystems  */
{ 257,   2, NETWK_TYPE_DCS_1800, "MTS BY", "MTS BY" },
/* Belarusskaya Set Telekommunkatsiy (BeST) */
{ 257, 25, NETWK_TYPE_GSM_900, "BeST", "BeST" }, 
/* Belarusskaya Set Telekommunkatsiy (BeST) */
{ 257, 25, NETWK_TYPE_DCS_1800, "BeST", "BeST" }, 

/*****************
 **** Moldova ****
 *****************/
/* Voxtel S.A */
{ 259,   1, NETWK_TYPE_GSM_900, "Orange MD", "Orange MD" },   /* refer to selist20080115.xls for Orange [jm0122.yoon 20080122] */
/* Moldcell SA */
{ 259,   2, NETWK_TYPE_GSM_900, "MD MOLDCELL", "MD MOLDCELL" },

/****************
 **** Poland ****
 ****************/
/* Polkomtel S.A */
{ 260,   1, NETWK_TYPE_GSM_900, "Plus", "Plus" },
/* Polkomtel S.A */
{ 260,   1, NETWK_TYPE_DCS_1800, "Plus", "Plus" },
/* Polkomtel S.A */
{ 260,   1, NETWK_TYPE_UMTS, "Plus", "Plus" },
/* Polska Telefonia Cyfrowa */
{ 260,   2, NETWK_TYPE_GSM_900, "T-Mobile.pl", "T-Mobile.pl" },
/* Polska Telefonia Cyfrowa */
{ 260,   2, NETWK_TYPE_DCS_1800, "T-Mobile.pl", "T-Mobile.pl" },
/* PTK Centertel */
{ 260,   3, NETWK_TYPE_GSM_900, "Orange", "Orange" },   // [2009.01.21] Orange PL에서 Orange로 변경
/* PTK Centertel */
{ 260,   3, NETWK_TYPE_DCS_1800, "Orange", "Orange" },  // [2009.01.21] Orange PL에서 Orange로 변경
/* PTK Centertel */
{ 260,   3, NETWK_TYPE_UMTS, "Orange", "Orange" },   // [2009.01.21] Orange PL에서 Orange로 변경
/* Poland */
{ 260,   6, NETWK_TYPE_UMTS, "Play", "Play" }, //[2007.10.02]sungeun.lim 수출부에서 입수한 사업자 요청 및 현채인 확인 최종 PLMN name으로 update.
/* Cyfrowy Polsat */
{ 260,  12, NETWK_TYPE_GSM_900, "Cyfrowy Polsat", "Cyfrowy Polsat" }, // S5230 수정요청.
/* Cyfrowy Polsat */
{ 260,  12, NETWK_TYPE_DCS_1800, "Cyfrowy Polsat", "Cyfrowy Polsat" }, // S5230 수정요청.
/* Cyfrowy Polsat */
{ 260,  12, NETWK_TYPE_UMTS, "Cyfrowy Polsat", "Cyfrowy Polsat" }, // S5230 수정요청.
/* Aero2 */
{ 260,  17, NETWK_TYPE_UMTS, "Aero2", "Aero2" }, // S5230 수정요청.


/*****************
 **** Germany ****
 *****************/
/* T-Mobile Deutschland GmbH */
{ 262,   1, NETWK_TYPE_GSM_900, "Telekom.de", "Telekom.de" },  // D0100199642 change request from operator 2010.09.15
/* T-Mobile Deutschland GmbH */
{ 262,   1, NETWK_TYPE_DCS_1800, "Telekom.de", "Telekom.de" },  // D0100199642 change request from operator 2010.09.15
/* Vodafone D2 GmbH */
{ 262,   2, NETWK_TYPE_GSM_900, "Vodafone.de", "Vodafone.de" },
/* Vodafone D2 GmbH */
{ 262,   2, NETWK_TYPE_DCS_1800, "Vodafone.de", "Vodafone.de" },
/* E-Plus Mobilfunk GmbH */
{ 262,   3, NETWK_TYPE_DCS_1800, "E-Plus", "E-Plus" },
/* Viag Interkom */
{ 262,   7, NETWK_TYPE_DCS_1800, "o2 - de", "o2 - de" },
/* O2 (Germany) GmbH & Co. OHG */
{ 262, 8, NETWK_TYPE_DCS_1800, "o2 - de", "o2 - de" }, 
/* Mobilcom Multimedia GMBH */
{ 262,  13, NETWK_TYPE_UMTS, "Mobilcom", "Mobilcom" },
/* Group 3G UMTS GmbH */
{ 262,  14, NETWK_TYPE_UMTS, "Group3G", "Group3G" },

/*******************
 **** Gibraltar ****
 *******************/
/* Gibraltar Telecoms Int'l */
{ 266,   1, NETWK_TYPE_GSM_900, "GIBTEL GSM", "GIBTEL GSM" },

/******************
 **** Portugal ****
 ******************/
/* Vodafone Telecel */
{ 268,   1, NETWK_TYPE_GSM_900, "vodafone P", "vodafone P" },
/* Vodafone Telecel */
{ 268,   1, NETWK_TYPE_DCS_1800, "vodafone P", "vodafone P" },
/* Optimus Telecomunicacoes, S.A */
{ 268,   3, NETWK_TYPE_GSM_900, "P OPTIMUS", "P OPTIMUS" },
/* Optimus Telecomunicacoes, S.A */
{ 268,   3, NETWK_TYPE_DCS_1800, "P OPTIMUS", "P OPTIMUS" },
/* Telecomunicacoes Moveis Nacionais S.A */
{ 268,   6, NETWK_TYPE_GSM_900, "P TMN", "P TMN" },
/* Telecomunicacoes Moveis Nacionais S.A */
{ 268,   6, NETWK_TYPE_DCS_1800, "P TMN", "P TMN" },
/* TMN */
{ 268, 6, NETWK_TYPE_UMTS, "P TMN", "P TMN" }, 

/********************
 **** Luxembourg ****
 ********************/
 /* P & T Luxembourg */
{ 270,   1, NETWK_TYPE_GSM_900, "LUXGSM", "LUXGSM" }, //[2009.01.21] L LUXGSM에서 LUXGSM으로 수정
 /* P & T Luxembourg */
{ 270,   1, NETWK_TYPE_DCS_1800, "LUXGSM", "LUXGSM" },//[2009.01.21] L LUXGSM에서 LUXGSM으로 수정
/* Tango S.A */
{ 270,  77, NETWK_TYPE_GSM_900, "Tango", "Tango" },  //2008.12.02 L Tango -> Tango 현채인 Loris Beerten [l.beerten@samsung.com]요청사항 
/* Tango S.A */
{ 270,  77, NETWK_TYPE_DCS_1800, "Tango", "Tango" }, //2008.12.02 L Tango -> Tango 현채인 Loris Beerten [l.beerten@samsung.com]요청사항 
/* VOXmobile S.A. */
{ 270, 99, NETWK_TYPE_DCS_1800, "Orange", "Orange" }, //2008.12.02 VOX.LU -> Vox Mobile 현채인 Loris Beerten [l.beerten@samsung.com]요청사항  
/* VOXmobile S.A. */
{ 270, 99, NETWK_TYPE_UMTS, "Orange", "Orange" },     //2008.12.02 VOX.LU -> Vox Mobile 현채인 Loris Beerten [l.beerten@samsung.com]요청사항  

/*****************
 **** Ireland ****
 *****************/
/* Vodafone Ireland Plc */
{ 272,   1, NETWK_TYPE_GSM_900, "vodafone IE", "vodafone IE" },
/* Vodafone Ireland Ltd. */
{ 272, 1, NETWK_TYPE_DCS_1800, "vodafone IE", "vodafone IE" }, 
/* Digifone MM02 Ltd */
{ 272,   2, NETWK_TYPE_GSM_900, "O2.ie", "O2.ie" },  /* Request from MMO2, S8300 20090226 */
/* Digifone MM02 Ltd */
{ 272,   2, NETWK_TYPE_DCS_1800, "O2.ie", "O2.ie" },  /* Request from MMO2, S8300 20090226 */
/* Meteor Mobile Telecommunications Limited */
{ 272,   3, NETWK_TYPE_GSM_900, "eMobile",  "eMobile" },  /* update EIR 20111010 */
/* Meteor Mobile Telecommunications Limited */
{ 272,   3, NETWK_TYPE_DCS_1800, "eMobile",  "eMobile" },  /* update EIR 20111010 */
/* Hutchison NETWK_TYPE_UMTS Ireland limited */
{ 272, 5, NETWK_TYPE_UMTS, "3", "3" }, //2008.12.02 3 IRL->3 최나현사원 요구사항

/*****************
 **** Iceland ****
 *****************/
/* Iceland Telecom Ltd */
{ 274,   1, NETWK_TYPE_GSM_900, "IS SIMINN", "IS SIMINN" },
/* Iceland Telecom Ltd */
{ 274,   1, NETWK_TYPE_DCS_1800, "IS SIMINN", "IS SIMINN" },
/* Og fjarskipti hf */
{ 274,   2, NETWK_TYPE_GSM_900, "Og Vodafone", "Og Vodafone" },
/* Og fjarskipti hf */
{ 274,   2, NETWK_TYPE_DCS_1800, "Og Vodafone", "Og Vodafone" },
/* Og fjarskipti hf */
{ 274,   3, NETWK_TYPE_GSM_900, "Og Vodafone", "Og Vodafone" },
/* Og fjarskipti hf */
{ 274,   3, NETWK_TYPE_DCS_1800, "Og Vodafone", "Og Vodafone" },
/* IMC Island ehf */
{ 274,   4, NETWK_TYPE_DCS_1800, "Viking", "Viking" },
/* Nova ehf*/
{ 274,  11, NETWK_TYPE_UMTS, "Nova", "Nova" },

/*****************
 **** Albania ****
 *****************/
 /* Albanian Mobile Communications */
{ 276,   1, NETWK_TYPE_GSM_900, "AMC - AL", "AMC - AL" },
/* Vodafone Albania */
{ 276,   2, NETWK_TYPE_GSM_900, "vodafone AL", "vodafone AL" },
/* Vodafone Albania */
{ 276,   2, NETWK_TYPE_DCS_1800, "vodafone AL", "vodafone AL" },

/***************
 **** Malta ****
 ***************/
/* Vodafone Malta Limited */
{ 278,   1, NETWK_TYPE_GSM_900, "vodafone MT", "vodafone MT" },
/* Mobisle Communications Limited */
{ 278,  21, NETWK_TYPE_DCS_1800, "gomobile", "gomobile" },

/****************
 **** Cyprus ****
 ****************/
/* Cytamobile-Vodafone */
{ 280,   1, NETWK_TYPE_GSM_900, "CYTAVODA", "CYTAVODA" },  //2009.01.15 
/* Cytamobile-Vodafone */
{ 280,   1, NETWK_TYPE_DCS_1800, "CYTAVODA", "CYTAVODA" }, //2009.01.15  
/* Areeba LTD */
{ 280, 10, NETWK_TYPE_GSM_900, "MTN", "MTN" }, //2008.12.02 Areeba-> MTN 현채인요청 Polys Hadjikyriakos 
/* Areeba LTD */
{ 280, 10, NETWK_TYPE_DCS_1800, "MTN", "MTN" }, //2008.12.02

/*****************
 **** Georgia ****
 *****************/
/* Geocell Limited */
{ 282,   1, NETWK_TYPE_GSM_900, "GEO-GEOCELL", "GEO-GEOCELL" },
/* Geocell Limited */
{ 282,   1, NETWK_TYPE_DCS_1800, "GEO-GEOCELL", "GEO-GEOCELL" },
/* Magti GSM */
{ 282,   2, NETWK_TYPE_GSM_900, "MAGTI-GSM-GEO", "MAGTI-GSM-GEO" },
/* Magti GSM */
{ 282,   2, NETWK_TYPE_DCS_1800, "MAGTI-GSM-GEO", "MAGTI-GSM-GEO" },

/*****************
 **** Armenia ****
 *****************/
/* ArmenTel */
{ 283,   1, NETWK_TYPE_GSM_900, "RA-ARMGSM", "RA-ARMGSM" },
/* Karabakh Telecom */
{ 283,   4, NETWK_TYPE_GSM_900, "RA 04", "RA 04" },
/* K Telecom CJSC */
{ 283, 5, NETWK_TYPE_GSM_900, "RA 05", "RA 05" }, 
/* K Telecom CJSC */
{ 283, 5, NETWK_TYPE_DCS_1800, "RA 05", "RA 05" }, 

/******************
 **** Bulgaria ****
 ******************/
/* MobilTEL AD */
{ 284,   1, NETWK_TYPE_GSM_900, "M-Tel BG", "M-Tel BG" },
/* BTC Mobile EOOD */
{ 284,   3, NETWK_TYPE_GSM_900, "vivatel", "vivatel" },//[2007.10.02]sungeun.lim 수출부에서 입수한 사업자 요청 및 현채인 확인 최종 PLMN name으로 update.
/* Cosmo Bulgaria Mobile EAD */
{ 284,   5, NETWK_TYPE_GSM_900, "BG GLOBUL", "BG GLOBUL" },//[2007.10.02]sungeun.lim 수출부에서 입수한 사업자 요청 및 현채인 확인 최종 PLMN name으로 update.
/* Cosmo Bulgaria Mobile EAD */
{ 284,   5, NETWK_TYPE_DCS_1800, "BG GLOBUL", "BG GLOBUL" },//[2007.10.02]sungeun.lim 수출부에서 입수한 사업자 요청 및 현채인 확인 최종 PLMN name으로 update.

/****************
 **** Turkey ****
 ****************/
/* TR Turkcell */
{ 286,   1, NETWK_TYPE_GSM_900, "TR Turkcell", "TR Turkcell" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* VODAFONE TR */
{ 286,   2, NETWK_TYPE_GSM_900, "VODAFONE TR", "VODAFONE TR" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* IS-TIM Telekomunikasyon Hizmetleri Anonim Sirketi */
{ 286,   3, NETWK_TYPE_DCS_1800, "TR AVEA", "TR AVEA" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Turk Telekomunikasyon A.S. */
{ 286,   4, NETWK_TYPE_DCS_1800, "TR AYCELL", "TR AYCELL" },

/***********************
 **** Faroe Islands ****
 ***********************/
/* Faroese Telecom GSM 900 */
{ 288,   1, NETWK_TYPE_GSM_900, "FROFT", "FROFT" },
/* P/F Kall (Kall Telecom) */
{ 288,   2, NETWK_TYPE_GSM_900, "KALL", "KALL" },

/*******************
 **** Greenland ****
 *******************/
/* Tele Greenland A/S */
{ 290,   1, NETWK_TYPE_GSM_900, "TELE Greenland", "TELE Greenland" },

/******************
 **** Slovenia ****
 ******************/
/* SI.Mobil d.d */
{ 293,	 30, NETWK_TYPE_GSM_900, "Si.mobil", "Si.mobil" }, //[2010.01.07] Delfi_Wifi PL통해 현채인 요청함.
/* SI.Mobil d.d */
{ 293,	 30, NETWK_TYPE_DCS_1800, "Si.mobil", "Si.mobil" }, //[2010.01.07] Delfi_Wifi PL통해 현채인 요청함.
/* Mobitel d.d */
{ 293,	 31, NETWK_TYPE_GSM_900, "Mobitel", "Mobitel" }, //[2010.01.07] Delfi_Wifi PL통해 현채인 요청함.
/* Mobitel d.d */
{ 293,	 31, NETWK_TYPE_DCS_1800, "Mobitel", "Mobitel" }, //[2010.01.07] Delfi_Wifi PL통해 현채인 요청함.
/* SI.Mobil d.d */
{ 293,  40, NETWK_TYPE_GSM_900, "Si.mobil", "Si.mobil" }, //[2009.02.09] Loches voda Opcos issue에서 수정 요청.
/* SI.Mobil d.d */
{ 293,  40, NETWK_TYPE_DCS_1800, "Si.mobil", "Si.mobil" }, //[2009.02.09] Loches voda Opcos issue에서 수정 요청.
/* Mobitel d.d */
{ 293,  41, NETWK_TYPE_GSM_900, "Mobitel", "Mobitel" }, //[2007.10.02]sungeun.lim 수출부에서 입수한 사업자 요청 및 현채인 확인 최종 PLMN name으로 update.
/* Mobitel d.d */
{ 293,  41, NETWK_TYPE_DCS_1800, "Mobitel", "Mobitel" }, //[2007.10.02]sungeun.lim 수출부에서 입수한 사업자 요청 및 현채인 확인 최종 PLMN name으로 update.
/* Mobitel d.d */
{ 293,	 51, NETWK_TYPE_GSM_900, "Mobitel", "Mobitel" }, //[2010.01.07] Delfi_Wifi PL통해 현채인 요청함.
/* Mobitel d.d */
{ 293,	 51, NETWK_TYPE_DCS_1800, "Mobitel", "Mobitel" }, //[2010.01.07] Delfi_Wifi PL통해 현채인 요청함.
/* Western Wireless International d.o.o. */
{ 293,	 64, NETWK_TYPE_DCS_1800, "T-2", "T-2" }, //[2010.01.07] Delfi_Wifi PL통해 현채인 요청함.
/* Western Wireless International d.o.o. */
{ 293,  70, NETWK_TYPE_DCS_1800, "Tusmobil", "Tusmobil" }, //[2010.01.07] Delfi_Wifi PL통해 현채인 요청함.

/*******************
 **** Macedonia ****
 *******************/
/* T-Mobile Macedonia */
{ 294,   1, NETWK_TYPE_GSM_900, "T-Mobile MK", "T-Mobile MK" },
/* COSMOFON AD Skopje */
{ 294,   2, NETWK_TYPE_GSM_900, "Cosmofon", "Cosmofon" }, //[2007.10.02]sungeun.lim 수출부에서 입수한 사업자 요청 및 현채인 확인 최종 PLMN name으로 update.
/*VIP OPERATOR DOOEL Skopje*/
{ 294,   3, NETWK_TYPE_GSM_900, "Vip MK", "Vip MK" }, //[2009.02.05] 현채인 요청에 의해 추가함.
/*VIP OPERATOR DOOEL Skopje*/
{ 294,   3, NETWK_TYPE_DCS_1800, "Vip MK", "Vip MK" }, //[2009.02.05] 현채인 요청에 의해 추가함.

/***********************
 **** Liechtenstein ****
 ***********************/
/* Swisscom Mobile Ltd (Liechtenstein) */
{ 295,   1, NETWK_TYPE_GSM_900, "FL GSM", "FL GSM" },
/* Swisscom Mobile Ltd (Liechtenstein) */
{ 295,   1, NETWK_TYPE_DCS_1800, "FL GSM", "FL GSM" },
/* Orange (Liechtenstein) AG */
{ 295,   2, NETWK_TYPE_DCS_1800, "Orange FL", "Orange FL" },
/* Mobilkom (Liechtenstein) AG */
{ 295,   5, NETWK_TYPE_DCS_1800, "FL1", "FL1" },
/* Tele 2 Aktiengesellschaft */
{ 295,  77, NETWK_TYPE_GSM_900, "LI TANGO", "LI TANGO" },

/********************
 **** Montenegro ****
 ********************/
/* ProMonte GSM */
{ 297,   1, NETWK_TYPE_GSM_900, "Promonte", "Promonte" },
/* ProMonte GSM */
{ 297,   1, NETWK_TYPE_DCS_1800, "Promonte", "Promonte" },
/* T-Mobile Montenegro LLC */
{ 297,   2, NETWK_TYPE_GSM_900, "T-Mobile", "T-Mobile" },
/* Telecom Montenegro  */
{ 297,   3, NETWK_TYPE_GSM_900, "m:tel", "m:tel" },
/* Telecom Montenegro  */
{ 297,   3, NETWK_TYPE_DCS_1800, "m:tel", "m:tel" },
/* Telecom Montenegro  */
{ 297,   3, NETWK_TYPE_UMTS, "m:tel", "m:tel" },

/****************
 **** Canada ****
 ****************/
/* Microcell Telecom Inc */
{ 302,  37, NETWK_TYPE_PCS_1900, "MICROCELL", "MICROCELL" },
/* Fido */
{ 302,  370, NETWK_TYPE_PCS_1900, "Fido", "Fido" },
/* Rogers Wireless */
{ 302,  720, NETWK_TYPE_PCS_1900, "Rogers", "Rogers" },


/* SPM Telecom  */
{ 308, 1, NETWK_TYPE_GSM_900, "AMERIS", "AMERIS" }, 

/**************************
 **** Papua New Guinea ****
 **************************/
/* Pacific Mobile Comms */
{ 310,   1, NETWK_TYPE_GSM_900, "Pacific", "Pacific" },

/****************
 **** U.S.A. ****
 ****************/
/* Voicestream Wireless Corporation */
{ 310,  2, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* PA Cellnet */
{ 310,  10, NETWK_TYPE_PCS_1900, "PA Cellnet", "PA Cellnet" }, /* M625000000623 */
/* Third Kentucky Cellular Corporation */
{ 310,  11, NETWK_TYPE_PCS_1900, "USA 11", "USA 11" },
/* Cingular Wireless => AT&T */
{ 310,  15, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" },
/* Voicestream Wireless Corporation */
{ 310,  16, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* Cingular Wireless  => AT&T */
{ 310,  17, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" },
/* Dutch Harbor */
{ 310,  19, NETWK_TYPE_PCS_1900, "Dutch Harbor", "Dutch Harbor" },
/* T-Mobile USA, Inc */
{ 310,  20, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* T-Mobile USA, Inc */
{ 310,  21, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* T-Mobile USA, Inc */
{ 310,  22, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* T-Mobile USA, Inc */
{ 310,  23, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* T-Mobile USA, Inc */
{ 310,  24, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* T-Mobile USA, Inc */
{ 310,  25, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* T-Mobile USA, Inc */
{ 310,  26, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* T-Mobile USA, Inc */
{ 310,  27, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* Centennial Wireless */
{ 310,  30, NETWK_TYPE_PCS_1900, "Centennial Wireless", "Centennial Wireless" },
/* T-Mobile USA, Inc */
{ 310,  31, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* High Plains/Midwest LLC */
{ 310,  34, NETWK_TYPE_PCS_1900, "WestLink", "WestLink" },
/* Cingular Wireless  => AT&T */
{ 310,  38, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" },  /*M627500000468*/
/* Cellular One */
{ 310,  40, NETWK_TYPE_PCS_1900, "Cellular One", "Cellular One" }, /* M625000000623 */
/* Raju:Cingular Wireless  => AT&T */
/* Added new entry for the cingular  => AT&T */
{ 310,  41, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" }, /*M627500000468*/
/* TMP Corp */
{ 310,  46, NETWK_TYPE_PCS_1900, "TMP", "TMP" },
/* DIGICEL */
{ 310,  50, NETWK_TYPE_PCS_1900, "JAM DC", "JAM DC" }, /* M625000000623 */
/* Key Communications, LLC */
{ 310,  53, NETWK_TYPE_PCS_1900, "WVW", "WVW" },
/* PCS One Inc. */
{ 310,  58, NETWK_TYPE_PCS_1900, "VSTREAM", "VSTREAM" },
/* Choice Wireless L.C. */
{ 310,  63, NETWK_TYPE_PCS_1900, "Choice", "Choice" },
/* Airadigm Communications */
{ 310,  64, NETWK_TYPE_PCS_1900, "ARDGMC", "ARDGMC" },
/* AT&T */
{ 310,  70, NETWK_TYPE_GSM_850, "AT&T", "AT&T" },
/* Corr Wireless */
{ 310,  80, NETWK_TYPE_PCS_1900, "Corr Wireless", "Corr Wireless" }, /* M625000000623 */
/* AT&T */
{ 310,  90, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" },
/* Plateau Wireless */
{ 310, 100, NETWK_TYPE_PCS_1900, "Plateau Wireless", "Plateau Wireless" }, /* M625000000623 */
/* WTTCKy */
{ 310, 110, NETWK_TYPE_PCS_1900, "USA 110", "USA 110" }, /* TMO USA Requirement... 2008.02.07 */
/* Cingular Wireless  => AT&T */
{ 310, 150, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" },
/* T-Mobile USA, Inc */
{ 310, 160, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" }, /* M625000000623 */
/* Cingular Wireless  => AT&T */
{ 310, 170, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" },
/* West Central */
{ 310, 180, NETWK_TYPE_PCS_1900, "West Central", "West Central" }, /* M625000000623 */
/* Alaska Wireless */
{ 310, 190, NETWK_TYPE_PCS_1900, "Alaska Wireless", "Alaska Wireless" }, /* M625000000623 */
/* T-Mobile USA, Inc */
{ 310, 200, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* T-Mobile USA, Inc */
{ 310, 210, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* T-Mobile USA, Inc */
{ 310, 220, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* T-Mobile USA, Inc */
{ 310, 230, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* T-Mobile USA, Inc */
{ 310, 240, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* T-Mobile USA, Inc */
{ 310, 250, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* T-Mobile USA, Inc */
{ 310, 260, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* T-Mobile USA, Inc */
{ 310, 270, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* T-Mobile USA, Inc */
{ 310, 310, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" }, /* M625000000623 */
/* Farmers Cellular Telephone Inc */
{ 310, 311, NETWK_TYPE_PCS_1900, "FARMERS", "FARMERS" },
/* Farmers Cellular Telephone Inc */
{ 310,  311, NETWK_TYPE_GSM_850, "FARMERS", "FARMERS" },
/* Smith Bagley, Inc. */
{ 310,  320, NETWK_TYPE_GSM_850, "USA - CellularOne", "USA - CellularOne" },
/* Smith Bagley, Inc.*/
{ 310, 320, NETWK_TYPE_PCS_1900, "USA - CellularOne", "USA - CellularOne" },
/* High Plains/Midwest LLC */
{ 310, 340, NETWK_TYPE_PCS_1900, "WestLink Comm", "Westlink Comm" },
/* Carolina Phone */
{ 310, 350, NETWK_TYPE_PCS_1900, "Carolina Phone", "Carolina Phone" }, /* M625000000623 */
/* Cingular Wireless  => AT&T */
{ 310, 380, NETWK_TYPE_GSM_850, "AT&T", "AT&T" }, /* M627500000468 */
/* Cingular Wireless  => AT&T */
{ 310, 380, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" }, /* M627500000468 */
/* Yorkville */
{ 310, 390, NETWK_TYPE_PCS_1900, "Yorkville", "Yorkville" }, /* M625000000623 */
/* Cingular Wireless  => AT&T */
{ 310, 410, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" }, /* M625000000623 */
/* Cincinnati Bell Wireless */
{ 310, 420, NETWK_TYPE_PCS_1900, "Cincinnati Bell Wireless", "Cincinnati Bell Wireless" }, /* M625000000623 */
/* Viaero Wireless */
{ 310, 450, NETWK_TYPE_PCS_1900, "Viaero Wireless", "Viaero Wireless" }, /* M625000000623 */
/* USA ONELINK */
{ 310, 460, NETWK_TYPE_PCS_1900, "USA ONELINK", "USA ONELINK" }, /* M625000000623 */
/* SunCom */
{ 310, 490, NETWK_TYPE_PCS_1900, "SunCom", "SunCom" }, /* M625000000623 */
/* Public Service Cellular, Inc */
{ 310, 500, NETWK_TYPE_GSM_850, "PSC Wireless", "PSC Wireless" },
/* Key Communications, LLC */
{ 310, 530, NETWK_TYPE_PCS_1900, "WVW", "WVW" },
/* AT&T */
{ 310, 560, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" },
/* T-Mobile USA, Inc */
{ 310, 580, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" }, /* M625000000623 */
/* WWC Holding Co., Inc */
{ 310, 590, NETWK_TYPE_GSM_850, "USA - Extended Area", "USA - Extended Area" },
/* Epic Touch */
{ 310, 610, NETWK_TYPE_PCS_1900, "Epic Touch", "Epic Touch" }, /* M625000000623 */
/* AmeriLink PCS */
{ 310, 630, NETWK_TYPE_PCS_1900, "AmeriLink PCS", "AmeriLink PCS" }, /* M625000000623 */
/* Einstein PCS */
{ 310, 640, NETWK_TYPE_PCS_1900, "Einstein PCS", "Einstein PCS" }, /* M625000000623 */
/* T-Mobile USA, Inc */
{ 310, 660, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" },
/* W 2000 PCS */
{ 310, 670, NETWK_TYPE_PCS_1900, "W 2000 PCS", "W 2000 PCS" }, /* M625000000623 */
/* AT&T */
{ 310, 680, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" }, 
/* Immix Wireless */
{ 310, 690, NETWK_TYPE_PCS_1900, "Immix Wireless", "Immix Wireless" }, /* M625000000623 */
/* HIGHLAND */
{ 310, 700, NETWK_TYPE_PCS_1900, "HIGHLAND", "HIGHLAND" }, /* M625000000623 */
/* Rogers */
{ 310, 720, NETWK_TYPE_PCS_1900, "Rogers", "Rogers" }, /* M625000000623 */
/* Telemetrix */
{ 310, 740, NETWK_TYPE_PCS_1900, "Telemetrix", "Telemetrix" }, /* M625000000623 */
/* PTSI */
{ 310, 760, NETWK_TYPE_PCS_1900, "PTSI", "PTSI" }, /* M625000000623 */
/* i wireless */
{ 310, 770, NETWK_TYPE_PCS_1900, "i wireless", "i wireless" },  /* M625000000623 */
/* AirLink PCS */
{ 310, 780, NETWK_TYPE_PCS_1900, "AirLink PCS", "AirLink PCS" }, /* M625000000623 */
/* PinPoint */
{ 310, 790, NETWK_TYPE_PCS_1900, "Pinpoint", "Pinpoint" }, /* M625000000623 */
/* T-Mobile USA, Inc */
{ 310, 800, NETWK_TYPE_PCS_1900, "T-Mobile USA", "T-Mobile USA" }, /* M625000000623 */
/* Advantage Cellular Systems Inc */
{ 310,  880, NETWK_TYPE_GSM_850, "USAACSI", "USAACSI" },
/* Southern Illinois RSA Partnership */
{ 310,  910, NETWK_TYPE_GSM_850, "USAFC", "USAFC" },
/* Cingular Wireless  => AT&T */
{ 310,  930, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" },
/* Lynn County Cellular Limited Partnership */
{ 310,  940, NETWK_TYPE_GSM_850, "Digital Cellular", "Digital Cellular" },
/* Texas RSA 8 West Limited */
{ 310,  940, NETWK_TYPE_GSM_850, "Digital Cellular", "Digital Cellular" },
/* XIT Cellular */
{ 310,  950, NETWK_TYPE_GSM_850, "USA XIT Cellular", "USA XIT Cellular" },
/* Cingular Wireless  => AT&T */
{ 310,  980, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" }, /* M625000000623 */
/* Cingular  => AT&T */
{ 310,  990, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" },


/* Mid-Tex Cellular, Ltd. */
{ 311,  0, NETWK_TYPE_GSM_850, "USA Mid-Tex Cellular, Ltd", "USA Mid-Tex Cellular, Ltd" },
/* Wilkes Cellular Inc */
{ 311, 1, NETWK_TYPE_GSM_850, "WILKES", "WILKES" }, 
/* Wilkes Cellular Inc */
{ 311, 5, NETWK_TYPE_GSM_850, "WILKES", "WILKES" }, 
/* "Indigo Wireless, Inc" */
{ 311, 30, NETWK_TYPE_PCS_1900, "Indigo", "Indigo" }, 
/* Easterbrooke Cellular Corporation */
{ 311, 70, NETWK_TYPE_GSM_850, "EASTER", "EASTER" }, 
/* Pine Telephone Company */
{ 311,  80, NETWK_TYPE_GSM_850, "Pine Cellular", "Pine Cellular" },
/* Long Lines Wireless LLC */
{ 311, 90, NETWK_TYPE_PCS_1900, "SXLP", "SXLP" }, 
/* "High Plains Wireless, L.P" */
{ 311, 110, NETWK_TYPE_PCS_1900, "HPW", "HPW" }, 
/* "Amarillo License, L.P" */
{ 311, 130, NETWK_TYPE_GSM_850, "C1AMARIL", "C1AMARIL" }, 
/* Sprocket */
{ 311, 140, NETWK_TYPE_PCS_1900, "Sprocket", "Sprocket" }, /* M625000000623 */ 
/* Petrocom LLC */
{ 311, 170, NETWK_TYPE_GSM_850, "PetroCom", "PetroCom" }, 
/* Cingular Wireless  => AT&T */
{ 311, 180, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" }, /* M625000000623 */ 
/* "Cellular Properties, Inc  " */
{ 311, 190, NETWK_TYPE_GSM_850, "C1ECI", "C1ECI" }, 
/* Farmers Cellular Telephone Inc */
{ 311, 210, NETWK_TYPE_GSM_850, "FARMERS", "FARMERS" }, 
/* Farmers Cellular Telephone Inc */
{ 311, 210, NETWK_TYPE_PCS_1900, "FARMERS", "FARMERS" }, 
/* Wave Runner LLC (Guam) */
{ 311, 250, NETWK_TYPE_PCS_1900, "iCAN_GSM", "i CAN_GSM" }, 
/* "Lamar County Cellular, Inc" */
{ 311, 310, NETWK_TYPE_GSM_850, "LamarCel", "LamarCel" }, 

/****************
**** Puerto Rico****
****************/
/*America Movil*/  
#if defined (FEATURE_SAMSUNG_MXTELCEL) 
{330, 11, NETWK_TYPE_PCS_1900, "PRClaro", "PR Claro"},  // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
{330, 110, NETWK_TYPE_PCS_1900, "PRClaro", "PR Claro"},  // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#else
{330, 11, NETWK_TYPE_PCS_1900, "CLARO P.R.", "CLARO P.R."},  // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
{330, 110, NETWK_TYPE_PCS_1900, "CLARO P.R.", "CLARO P.R."},  // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif
/****************
**** Blue Sky****
****************/
/*Blue Sky US Virgin islands*/
{ 332,  11, NETWK_TYPE_PCS_1900, "Blue Sky", "Blue Sky" },

/****************
 **** Mexico ****
 ****************/
#if defined(FEATURE_SAMSUNG_ARCTI)       
{ 334,  2, NETWK_TYPE_PCS_1900, "Telcel", "Telcel" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#elif defined (FEATURE_SAMSUNG_MXTELCEL)
{ 334,  2, NETWK_TYPE_PCS_1900, "TELCEL", "TELCEL GSM" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#else
{ 334,  2, NETWK_TYPE_PCS_1900, "TELCEL GSM", "TELCEL GSM" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif

/* Pegaso Comunicaciones y Sistemas, S.A. De C.V */
{ 334,  3, NETWK_TYPE_GSM_850, "movistar", "movistar" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Telcel GSM */

#if defined(FEATURE_SAMSUNG_ARCTI)
{ 334,  20, NETWK_TYPE_PCS_1900, "Telcel", "Telcel" }, /* M625000000623 */// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#elif defined (FEATURE_SAMSUNG_MXTELCEL)
{ 334,  20, NETWK_TYPE_PCS_1900, "TELCEL", "TELCEL GSM" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#else
{ 334,  20, NETWK_TYPE_PCS_1900, "TELCEL GSM", "TELCEL GSM" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif

{ 334,  30, NETWK_TYPE_PCS_1900, "movistar", "movistar" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청

/*****************
 **** Jamaica ****
 *****************/
/* Mossel Limited T/A Digicel */
{ 338,   5, NETWK_TYPE_GSM_900, "Digicel", "Digicel" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#if defined (FEATURE_SAMSUNG_MXTELCEL)
{ 338,   7, NETWK_TYPE_GSM_900, "Claro JAM", "Claro JAM" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#else
{ 338,   7, NETWK_TYPE_GSM_900, "Claro", "Claro" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif
{ 338,   18, NETWK_TYPE_GSM_900, "LIME", "LIME" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
{ 338,   50, NETWK_TYPE_GSM_900, "Digicel", "Digicel" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#if defined (FEATURE_SAMSUNG_MXTELCEL)
{ 338,   70, NETWK_TYPE_GSM_900, "Claro JAM", "Claro JAM" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#else
{ 338,   70, NETWK_TYPE_GSM_900, "Claro", "Claro" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif
{ 338,   180, NETWK_TYPE_GSM_900, "LIME", "LIME" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청

/****************************
 **** French West Indies ****
 ****************************/
/* ORANGE CARAIBE */
{ 340,   1, NETWK_TYPE_GSM_900, "F-Orange", "F-Orange" },//M627500002609
/* Outremer Telecom */
{ 340, 2, NETWK_TYPE_GSM_900, "ONLY", "ONLY" }, 
/* Outremer Telecom */
{ 340, 2, NETWK_TYPE_DCS_1800, "ONLY", "ONLY" }, 
/* Dauphin Telecom */
{ 340,   8, NETWK_TYPE_GSM_900, "AMIGO", "AMIGO" },
/* Dauphin Telecom */
{ 340,   8, NETWK_TYPE_DCS_1800, "AMIGO", "AMIGO" },
/* Bouygues Telecom Caraibe */
{ 340,  20, NETWK_TYPE_GSM_900, "Digicel", "Digicel" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Bouygues Telecom Caraibe */
{ 340,  20, NETWK_TYPE_DCS_1800, "Digicel", "Digicel" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청

/*****************
 **** Barbados ****
 *****************/
/* Digicel (Barbados) Limited */
{ 342,  50, NETWK_TYPE_GSM_900, "JM Digicel", "JM Digicel" },
/* Digicel (Barbados) Limited */
{ 342,  50, NETWK_TYPE_DCS_1800, "JM Digicel", "JM Digicel" },
/* Cable & Wireless Barbados Ltd. */
{ 342,  60, NETWK_TYPE_PCS_1900, "Cable&Wireless", "Cable&Wireless" },//byahn 090908
/* Digicel (Barbados) Limited */
{ 342,  75, NETWK_TYPE_GSM_900,   "Digicel", "Digicel" }, //  2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cellular Communications Barbados */
{ 342,  81, NETWK_TYPE_GSM_900, "AT&T Wireless", "AT&T Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless Barbados Ltd. */
{ 342,  600, NETWK_TYPE_PCS_1900, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Digicel (Barbados) Limited */
{ 342, 750, NETWK_TYPE_GSM_900,   "Digicel", "Digicel" },  // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Digicel (Barbados) Limited */
{ 342, 750, NETWK_TYPE_DCS_1800, "Digicel", "Digicel" },  // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cellular Communications Barbados */
{ 342, 810, NETWK_TYPE_GSM_900, "AT&T Wireless", "AT&T Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cellular Communications Barbados */
{ 342, 810, NETWK_TYPE_PCS_1900, "AT&T Wireless", "AT&T Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청


/*****************
 **** Antigua ****
 *****************/
/* Antigua Public Utilities Authority-APUA */
{ 344,  3, NETWK_TYPE_PCS_1900, "APUA PCS", "APUA PCS" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Antigua Public Utilities Authority-APUA */
{ 344,  30, NETWK_TYPE_PCS_1900, "APUA PCS", "APUA PCS" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless Caribbean Cellular (Antigua) Limited */
{ 344,  92, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless Caribbean Cellular (Antigua) Limited */
{ 344,  920, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Antigua Wireless Ventures Limited */
{ 344, 930, NETWK_TYPE_GSM_900, "AT&T", "AT&T" }, 

/*********************
 **** Cayman Islands ****
 *********************/
/* Cable & Wireless (Cayman Islands) Limited */
{ 346,  14, NETWK_TYPE_PCS_1900, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless (Cayman Islands) Limited */
{ 346,  14, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless (Cayman Islands) Limited */
{ 346,  140, NETWK_TYPE_PCS_1900, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless (Cayman Islands) Limited */
{ 346,  140, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless (Cayman Islands) Limited */
{ 348,  17, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless (Cayman Islands) Limited */
{ 348,  170, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Caribbean Cellular Telephone  */
{ 348, 570, NETWK_TYPE_GSM_900, "CCTBVI", "CCTBVI" }, 
/* Caribbean Cellular Telephone  */
{ 348, 570, NETWK_TYPE_PCS_1900, "CCTBVI", "CCTBVI" }, 

/*****************
 **** Bermuda ****
 *****************/
/* Telecommunications (Bermuda & West Indies) Ltd */
{ 350,   1, NETWK_TYPE_PCS_1900, "TELECOM BDA", "TELECOM BDA" },
/* Mobility Limited */
{ 350,   2, NETWK_TYPE_PCS_1900, "BTC MOBILITY LTD.", "BTC MOBILITY LTD." },
/* Telecommunications (Bermuda & West Indies) Ltd */
{ 350, 10, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" }, 

/***************
 **** Grenada ****
 ***************/
/* Grenada Wireless Holdings Ltd */
{ 352, 3, NETWK_TYPE_GSM_900, "Digicel", "Digicel" },  // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless Caribbean Cellular (Grenada) Limited */
{ 352,  11, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Grenada Wireless Holdings Ltd */
{ 352, 30, NETWK_TYPE_PCS_1900, "Digicel", "Digicel" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless Caribbean Cellular (Grenada) Limited */
{ 352,  110, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" },   // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless Caribbean Cellular (Grenada) Limited */
{ 354, 86, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless Caribbean Cellular (Grenada) Limited */
{ 354, 860, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청

/*********************
 **** St Kitts & Nevis ****
 *********************/
/* Cable & Wireless St Kitts & Nevis Limited */
{ 356,  11, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless St Kitts & Nevis Limited */
{ 356,  110, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless St Kitts & Nevis Limited */
{ 356, 110, NETWK_TYPE_PCS_1900, "Cable&Wireless", "Cable&Wireless" },  // 2009.09.09 중남미 개발 그룹 안병우 선임 요청

/***************
 **** St Lucia ****
 ***************/
/* Digicel (St Lucia) Limited */
{ 358, 5, NETWK_TYPE_GSM_900, "Digicel", "Digicel" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless Caribbean Cellular (St Lucia) Limited */
{ 358, 11, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" },  // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Wireless Ventures (St. Lucia) Limited */
{ 358, 30, NETWK_TYPE_GSM_900, "AT&T", "AT&T" }, 
/* Wireless Ventures (St. Lucia) Limited */
{ 358, 30, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" }, 
/* Digicel (St Lucia) Limited */
{ 358, 50, NETWK_TYPE_GSM_900, "Digicel", "Digicel" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Digicel (St Lucia) Limited */
{ 358, 50, NETWK_TYPE_DCS_1800, "Digicel", "Digicel" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Digicel (St Lucia) Limited */
{ 358, 50, NETWK_TYPE_PCS_1900, "Digicel", "Digicel" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless Caribbean Cellular (St Lucia) Limited */
{ 358, 110, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청

/*********************************
 **** St. Vincent & the Grenadines ****
 ********************************/
/* Digicel (St. Vincent and the Grenadines) Limited */
{ 360, 7, NETWK_TYPE_GSM_900, "Digicel", "Digicel" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Wireless Ventures (St. Vincent) Limited */
{ 360, 10, NETWK_TYPE_GSM_900, "AT&T", "AT&T" }, 
/* Wireless Ventures (St. Vincent) Limited */
{ 360, 10, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" }, 
/* Cable & Wireless Caribbean Cellular (St. Vincent & the Grenadines) Ltd */
{ 360,  11, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Digicel (St. Vincent and the Grenadines) Limited */
{ 360, 70, NETWK_TYPE_GSM_900, "Digicel", "Digicel" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Digicel (St. Vincent and the Grenadines) Limited */
{ 360, 70, NETWK_TYPE_DCS_1800, "Digicel", "Digicel" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless Caribbean Cellular (St. Vincent & the Grenadines) Ltd */
{ 360,  110, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" },// 2009.09.09 중남미 개발 그룹 안병우 선임 요청

/******************************
 **** Netherlands Antilles ****
 ******************************/
/* Telcell N.V. */
{ 362,  51, NETWK_TYPE_GSM_900, "Telcell GSM", "Telcell GSM" },
/* Curacao Telecom N.V. */
{ 362,  69, NETWK_TYPE_GSM_900, "Digicel", "Digicel" },// 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Curacao Telecom N.V. */
{ 362,  69, NETWK_TYPE_DCS_1800, "Digicel", "Digicel" },// 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Setel NV */
{ 362,  91, NETWK_TYPE_GSM_900, "Setel", "Setel" },
/* Communications Systems Curacao N.V. */
{ 362, 630, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" }, 
/* Curacao Telecom N.V. */
{ 362,  695, NETWK_TYPE_PCS_1900, "Digicel", "Digicel" },// 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Setel NV */
{ 362, 951, NETWK_TYPE_GSM_900, "CHIPPIE", "CHIPPIE" }, 

/**************
 *** Aruba ***
 **************/
/* SETAR (Servicio di Telecomunicacion di Aruba) */
{ 363,   1, NETWK_TYPE_GSM_900, "SETAR GSM", "SETAR GSM" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* SETAR (Servicio di Telecomunicacion di Aruba) */
{ 363,   1, NETWK_TYPE_PCS_1900, "SETAR GSM", "SETAR GSM" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* New Millenium Telecom Services (NMTS) */
{ 363,  2, NETWK_TYPE_GSM_900, "Digicel", "Digicel" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* New Millenium Telecom Services (NMTS) */
{ 363, 2, NETWK_TYPE_DCS_1800, "Digicel", "Digicel" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* New Millenium Telecom Services (NMTS) */
{ 363, 20, NETWK_TYPE_GSM_900, "Digicel", "Digicel" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* New Millenium Telecom Services (NMTS) */
{ 363, 20, NETWK_TYPE_DCS_1800, "Digicel", "Digicel" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청

/**************
 **** BAHMAS ***
 **************/
/* The Bahamas Telecommunications Company Ltd */
{ 364,   39, NETWK_TYPE_PCS_1900, "BaTelCell", "BaTelCell" },


/***************
 **** Anguilla ****
 ***************/
/* Cable & Wireless (West Indies) Ltd. Anguilla */
{ 365,  84, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless (West Indies) Ltd. Anguilla */
{ 365,  840, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청


/****************
 **** Dominica ****
 ****************/
 /* Cable & Wireless Dominica Ltd. */
{ 366,  11, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Wireless Ventures (Dominica) Ltd. */
{ 366, 20, NETWK_TYPE_GSM_900, "AT&T", "AT&T" }, 
/* Wireless Ventures (Dominica) Ltd. */
{ 366, 20, NETWK_TYPE_PCS_1900, "AT&T", "AT&T" }, 
/* Cable & Wireless Dominica Ltd. */
{ 366,  110, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청


/**************
 **** Cuba ****
 **************/
/* "Empresa de Telecommunicaciones de Cuba, SA (ETECSA)" */
{ 368, 1, NETWK_TYPE_GSM_900, "Cubacel", "Cubacel" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/****************************
 **** Dominican Republic ****
 ****************************/
/* Orange Dominicana S.A. */
{ 370,   1, NETWK_TYPE_GSM_900, "orange", "orange" },// D0100306029 2011/9/16 Orange 사업자 요청
/* Orange Dominicana S.A. */
{ 370,   1, NETWK_TYPE_DCS_1800, "orange", "orange" },// D0100306029 2011/9/16 Orange 사업자 요청
/* Orange Dominicana S.A. */
{ 370,   1, NETWK_TYPE_PCS_1900, "orange", "orange" },// D0100306029 2011/9/16 Orange 사업자 요청
#if defined (FEATURE_SAMSUNG_MXTELCEL)
{ 370,   2, NETWK_TYPE_PCS_1900, "ClaroDOM", "Claro DOM" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#else
{ 370,   2, NETWK_TYPE_PCS_1900, "Claro", "Claro" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif
{ 370,   4, NETWK_TYPE_PCS_1900, "VIVA", "VIVA" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* ORANGE */
{ 370,  10, NETWK_TYPE_PCS_1900, "ORANGE", "ORANGE" },
/* Orange Dominicana S.A. */
{ 370,   11, NETWK_TYPE_PCS_1900, "Orange", "Orange" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청

/* Communication Cellulaire d'Haiti SA */
{ 372, 1, NETWK_TYPE_GSM_850, "VOILA", "VOILA" }, 
{ 372, 2, NETWK_TYPE_DCS_1800, "Digicel", "Digicel " }, 
{ 372, 20, NETWK_TYPE_DCS_1800, "Digicel", "Digicel " }, 

/*****************************
 **** Trinidad and Tobaga ****
 *****************************/
/* Telcommunications Services of Trinidad and Tobago Ltd */
{ 374,  12, NETWK_TYPE_DCS_1800, "bmobile", "bmobile" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Trinidad and Tobago Ltd */  
{ 374,  13, NETWK_TYPE_GSM_850, "Digicel", "Digicel" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Trinidad and Tobago Ltd */
{ 374,  13, NETWK_TYPE_PCS_1900, "Digicel", "Digicel" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
{ 374,  120, NETWK_TYPE_DCS_1800, "bmobile", "bmobile" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
{ 374,  125, NETWK_TYPE_DCS_1800, "bmobile", "bmobile" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
{ 374,  129, NETWK_TYPE_DCS_1800, "bmobile", "bmobile" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
{ 374,  130, NETWK_TYPE_PCS_1900, "Digicel", "Digicel" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청

/**********************
 **** Turks & Caicos ****
 **********************/
/* Cable & Wireless West Indies Ltd (Turks & Caicos) */
{ 376, 35, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless West Indies Ltd (Turks & Caicos) */
{ 376, 350, NETWK_TYPE_GSM_850, "Cable&Wireless", "Cable&Wireless" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/********************
 **** Azerbaijan ****
 ********************/
/* Azercell Telekom B.M. */
{ 400,   1, NETWK_TYPE_GSM_900, "AZE - AZERCELL GSM", "AZE - AZERCELL GSM" },
/* Bakcell */
{ 400,   2, NETWK_TYPE_GSM_900, "BAKCELL GSM 2000", "BAKCELL GSM 2000" },

/********************
 **** Kazakhstan ****
 ********************/
/* Kar-Tel Ltd */
{ 401,   1, NETWK_TYPE_GSM_900, "Beeline", "Beeline" },		// 2008.06.11 kbs@CIS
/* GSM Kazakhstan Ltd */
{ 401,   2, NETWK_TYPE_GSM_900, "Kcell", "Kcell" },	// 2008.06.11 kbs@CIS
/* NEO */
{ 401,  77, NETWK_TYPE_GSM_900, "NEO", "NEO" },	  // 2009.01.16 kbs@CIS - 신규 사업자 추가

/********************
 **** *********** ****
 ********************/
/* B-Mobile */
{ 402,   11, NETWK_TYPE_GSM_900, "BT B-Mobile", "BT B-Mobile" },

/*2008.10.14 서남아요청사항적용*/ 
/***************
 **** India ****
 ***************/
/* _SHP_OPERATOR_INDIA start */
/* Aircel Digilink India Ltd */
{ 404,   1, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" },
/* Bharti Mobile Ltd */
{ 404,   2, NETWK_TYPE_GSM_900, "AirTel", "AirTel" },
/* Bharti Mobile Ltd */
{ 404,   3, NETWK_TYPE_GSM_900, "AirTel", "AirTel" },
/* IDEA Cellular Limited */
{ 404,   4, NETWK_TYPE_DCS_1800, "IDEA", "IDEA" },
/* Fascel Limited */
{ 404,   5, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" },
/* IDEA Cellular Limited */
{ 404,   7, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* Reliance Telecom Private Ltd */
{ 404,   9, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Bharti Mobile Ltd */
{ 404,  10, NETWK_TYPE_GSM_900, "AirTel", "AirTel" },
/* Hutchison Essar Telecom Limited */
{ 404,  11, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" },
/* Hutchison Essar Telecom Limited */
{ 404,  11, NETWK_TYPE_DCS_1800, "Vodafone IN", "Vodafone IN" },
/* Escotel Mobile Communications Ltd, Haryana */
{ 404,  12, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* Hutchison Essar South Limited */
{ 404,  13, NETWK_TYPE_DCS_1800, "Vodafone IN", "Vodafone IN" },
/* Spice Communications Limited - Punjab */
{ 404,  14, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* Aircel Digilink India Ltd */
{ 404,  15, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" },
/* Bharti Mobile Ltd */
{ 404, 16, NETWK_TYPE_GSM_900, "AirTel", "AirTel" }, 
/* Dishnet Wireless Limited */
{ 404, 17, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" }, 
/* Dishnet Wireless Limited */
{ 404, 17, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" }, 
/* Reliance Telecom Private Ltd */
{ 404,  18, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Escotel Mobile Communications Ltd, Kerala */
{ 404,  19, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* Hutchison Essar Limited */
{ 404, 20, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" }, 
/* Hutchison Essar Limited */
{ 404, 20, NETWK_TYPE_DCS_1800, "Vodafone IN", "Vodafone IN" }, 
/* Loop Mobile Communications Ltd - Mumbai */
{ 404,  21, NETWK_TYPE_GSM_900, "Loop Mobile", "Loop Mobile" },
/* IDEA Cellular Limited - Maharashtra/Goa */
{ 404,  22, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* IDEA Cellular Limited - Gujarat */
{ 404,  24, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* Dishnet Wireless Limited */
{ 404, 25, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" }, 
/* Dishnet Wireless Limited */
{ 404, 25, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" }, 
/* BPL Cellular Limited - Maharshtra/Goa */
{ 404,  27, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" },
/* Dishnet Wireless Limited */
{ 404, 28, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" }, 
/* Dishnet Wireless Limited */
{ 404, 28, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" }, 
/* Dishnet Wireless Limited */
{ 404, 29, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" }, 
/* Dishnet Wireless Limited */
{ 404, 29, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" }, 
/* Hutchison Telecom East Limited */
{ 404,  30, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" },
/* BHARTI MOBITEL LIMITED */
{ 404,  31, NETWK_TYPE_GSM_900, "AirTel", "AirTel" },
/* Dishnet Wireless Limited */
{ 404, 33, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" }, 
/* Dishnet Wireless Limited */
{ 404, 33, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" }, 
/* Bharat Sanchar Nigam Limited */
{ 404,  34, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Dishnet Wireless Limited */
{ 404, 35, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" }, 
/* Dishnet Wireless Limited */
{ 404, 35, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" }, 
/* Reliance Telecom Private Ltd */
{ 404,  36, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Dishnet Wireless Limited */
{ 404, 37, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" }, 
/* Dishnet Wireless Limited */
{ 404, 37, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" }, 
/* Bharat Sanchar Nigam Limited */
{ 404,  38, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Bharti Cellular Ltd */
{ 404,  40, NETWK_TYPE_GSM_900, "AirTel", "AirTel" },
/* RPG Cellular Services */
{ 404,  41, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" },
/* Srinivas Cellcom Limited */
{ 404,  42, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" },
/* BPL Cellular Limited - Tamil Nadu/Pondicherry */
{ 404,  43, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" },
/* Spice Communications Limited - Karnataka */
{ 404,  44, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* Bharti Mobile Ltd - Karnataka */
{ 404,  45, NETWK_TYPE_GSM_900, "AirTel", "AirTel" },
/* BPL Cellular Limited - Kerala */
{ 404,  46, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" },
/* Bharti Mobile Ltd - Andhra Pradesh */
{ 404,  49, NETWK_TYPE_GSM_900, "AirTel", "AirTel" },
/* Reliance Telecom Private Ltd */
{ 404,  50, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Bharat Sanchar Nigam Limited */
{ 404,  51, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Reliance Telecom Private Ltd */
{ 404,  52, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Bharat Sanchar Nigam Limited */
{ 404,  53, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Bharat Sanchar Nigam Limited */
{ 404,  54, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Bharat Sanchar Nigam Limited */
{ 404,  55, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Escotel Mobile Communications Ltd, UP(W) */
{ 404,  56, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* Bharat Sanchar Nigam Limited */
{ 404,  57, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Bharat Sanchar Nigam Limited */
{ 404,  58, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Bharat Sanchar Nigam Limited */
{ 404,  59, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Aircel Digilink India Ltd */
{ 404,  60, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" },
/* Bharat Sanchar Nigam Limited */
{ 404,  62, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Bharat Sanchar Nigam Limited */
{ 404,  64, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Bharat Sanchar Nigam Limited */
{ 404,  66, NETWK_TYPE_GSM_900, "CellOne","CellOne" },
/* Reliance Telecom Private Ltd */
{ 404,  67, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Mahanagar Telephone Nigam Ltd */
{ 404,  68, NETWK_TYPE_GSM_900, "Dolphin", "Dolphin" },
/* Mahanagar Telephone Nigam Ltd */
{ 404,  69, NETWK_TYPE_GSM_900, "Dolphin", "Dolphin" },
/* Hexacom India Limited */
{ 404,  70, NETWK_TYPE_GSM_900, "AirTel", "AirTel" },
/* Bharat Sanchar Nigam Limited */
{ 404,  71, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Bharat Sanchar Nigam Limited */
{ 404,  72, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Bharat Sanchar Nigam Limited */
{ 404,  73, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Bharat Sanchar Nigam Limited */
{ 404,  74, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Bharat Sanchar Nigam Limited */
{ 404,  75, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Bharat Sanchar Nigam Limited */
{ 404,  76, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Bharat Sanchar Nigam Limited */
{ 404,  77, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* IDEA Cellular Limited */
{ 404,  78, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* Bharat Sanchar Nigam Limited */
{ 404,  79, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Bharat Sanchar Nigam Limited */
{ 404,  80, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* Bharat Sanchar Nigam Limited */
{ 404,  81, NETWK_TYPE_GSM_900, "CellOne", "CellOne" },
/* IDEA Cellular Limited */
{ 404,  82, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* Reliance Telecom Private Ltd */
{ 404,  83, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Hutchison Essar South Limited */
{ 404,  84, NETWK_TYPE_DCS_1800, "Vodafone IN", "Vodafone IN" },
/* Reliance Telecom Private Ltd */
{ 404,  85, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Hutchison Essar South Limited */
{ 404,  86, NETWK_TYPE_DCS_1800, "Vodafone IN", "Vodafone IN"},
/* IDEA Cellular Limited */
{ 404,  87, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* Hutchison Essar South Limited */
{ 404,  88, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" },
/* IDEA Cellular Limited */
{ 404,  89, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* Bharti Cellular Ltd */
{ 404,  90, NETWK_TYPE_DCS_1800, "AirTel", "AirTel" },
/* Dishnet Wireless Limited */
{ 404, 91, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" }, 
/* Dishnet Wireless Limited */
{ 404, 91, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" }, 
/* Bharti Cellular Ltd */
{ 404,  92, NETWK_TYPE_DCS_1800, "AirTel", "AirTel" },
/* Bharti Cellular Ltd */
{ 404,  93, NETWK_TYPE_DCS_1800, "AirTel", "AirTel" },
/* Bharti Cellular Ltd */
{ 404,  94, NETWK_TYPE_DCS_1800, "AirTel", "AirTel" },
/* Bharti Cellular Ltd */
{ 404,  95, NETWK_TYPE_DCS_1800, "AirTel", "AirTel" },
/* Bharti Cellular Ltd */
{ 404,  96, NETWK_TYPE_DCS_1800, "AirTel", "AirTel" },
/* Bharti Cellular Ltd */
{ 404,  97, NETWK_TYPE_DCS_1800, "AirTel", "AirTel" },
/* Bharti Cellular Ltd */
{ 404,  98, NETWK_TYPE_DCS_1800, "AirTel", "AirTel" },

/********************
 ***               **
  *******************/
/* Reliance COMM */
{ 405, 1, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 3, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 4, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 5, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 6, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 7, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 8, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 9, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 10, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 11, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 12, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 13, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 14, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 15, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 17, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 18, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 19, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 20, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 21, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },
/* Reliance COMM */
{ 405, 22, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },    
/* Reliance COMM */
{ 405, 23, NETWK_TYPE_GSM_900, "Reliance", "Reliance" },    
/* Tata Teleservices */
{ 405, 25, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 26, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 27, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 28, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 29, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 30, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 31, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 32, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 33, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 34, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 35, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 36, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 37, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 38, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 39, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 40, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 41, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 42, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 43, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 44, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 45, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 46, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Tata Teleservices */
{ 405, 47, NETWK_TYPE_GSM_900, "TATA DOCOMO", "TATA DOCOMO" },
/* Bharti Televentures Limited */
{ 405, 51, NETWK_TYPE_GSM_900, "AirTel", "AirTel" }, 
/* Bharti Televentures Limited */
{ 405, 52, NETWK_TYPE_GSM_900, "AirTel", "AirTel" }, 
/* Bharti Televentures Limited */
{ 405, 53, NETWK_TYPE_GSM_900, "AirTel", "AirTel" }, 
/* Bharti Televentures Limited */
{ 405, 54, NETWK_TYPE_GSM_900, "AirTel", "AirTel" }, 
/* Bharti Televentures Limited */
{ 405, 55, NETWK_TYPE_GSM_900, "AirTel", "AirTel" }, 
/* Bharti Televentures Limited */
{ 405, 56, NETWK_TYPE_GSM_900, "AirTel", "Airtel" }, 
/* Hutchison Essar South Limited */
{ 405, 66, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" }, 
/* Hutchison Essar South Limited */
{ 405, 67, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" }, 
/* _SHP_OPERATOR_INDIA end */
/* IDEA Cellular Limited */
{ 405, 70, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 70, NETWK_TYPE_DCS_1800, "IDEA", "IDEA" },
/* Hutchison Essar South Limited */
{ 405, 750, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" }, 
/* Hutchison Essar South Limited */
{ 405, 751, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" }, 
/* Hutchison Essar South Limited */
{ 405, 752, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" }, 
/* Hutchison Essar South Limited */
{ 405, 753, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" }, 
/* Hutchison Essar South Limited */
{ 405, 754, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" }, 
/* Hutchison Essar South Limited */
{ 405, 755, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" }, 
/* Hutchison Essar South Limited */
{ 405, 756, NETWK_TYPE_GSM_900, "Vodafone IN", "Vodafone IN" }, 
/* IDEA Cellular Limited */
{ 405, 799, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 799, NETWK_TYPE_DCS_1800, "IDEA", "IDEA" },
/* Dishnet Wireless Limited *//*AIRCEL DELHI*/
{ 405, 800, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" }, 
/* AIRCEL */
{ 405, 801, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 801, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 802, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 802, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 803, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 803, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 804, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 804, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" },
/* Dishnet Wireless Limited *//*AIRCEL MUMBAI*/
{ 405, 805, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" }, 
/* AIRCEL */
{ 405, 806, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 806, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 807, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 807, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 808, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 808, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 809, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 809, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 810, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 810, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 811, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 811, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 812, NETWK_TYPE_GSM_900, "AIRCEL", "AIRCEL" },
/* AIRCEL */
{ 405, 812, NETWK_TYPE_DCS_1800, "AIRCEL", "AIRCEL" },
/* Uninor */
{ 405, 813, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 813, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 814, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 814, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 815, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 815, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 816, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 816, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 817, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 817, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 818, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 818, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 819, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 819, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 820, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 820, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 821, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 821, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 822, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 822, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Videocon */
{ 405, 823, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 823, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 824, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 824, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 825, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 825, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 826, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 826, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 827, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 827, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 828, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 828, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 829, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 829, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 830, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 830, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 831, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 831, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 832, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 832, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 833, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 833, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 834, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 834, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 835, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 835, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 836, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 836, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 837, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 837, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 838, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 838, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 839, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 839, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 840, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 840, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 841, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 841, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 842, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 842, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Videocon */
{ 405, 843, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 843, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* Uninor */
{ 405, 844, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 844, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* IDEA Cellular Limited */
{ 405, 845, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 845, NETWK_TYPE_DCS_1800, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 846, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 846, NETWK_TYPE_DCS_1800, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 847, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 847, NETWK_TYPE_DCS_1800, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 848, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 848, NETWK_TYPE_DCS_1800, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 849, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 849, NETWK_TYPE_DCS_1800, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 850, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 850, NETWK_TYPE_DCS_1800, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 851, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 851, NETWK_TYPE_DCS_1800, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 852, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 852, NETWK_TYPE_DCS_1800, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 853, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 853, NETWK_TYPE_DCS_1800, "IDEA", "IDEA" },
/* Uninor */
{ 405, 875, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 875, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 876, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 876, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 877, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 877, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 878, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 878, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 879, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 879, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 880, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 880, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* STEL */
{ 405, 881, NETWK_TYPE_GSM_900, "STEL", "STEL" },
/* STEL */
{ 405, 881, NETWK_TYPE_DCS_1800, "STEL", "STEL" },
/* STEL */
{ 405, 882, NETWK_TYPE_GSM_900, "STEL", "STEL" },
/* STEL */
{ 405, 882, NETWK_TYPE_DCS_1800, "STEL", "STEL" },
/* STEL */
{ 405, 883, NETWK_TYPE_GSM_900, "STEL", "STEL" },
/* STEL */
{ 405, 883, NETWK_TYPE_DCS_1800, "STEL", "STEL" },
/* STEL */
{ 405, 884, NETWK_TYPE_GSM_900, "STEL", "STEL" },
/* STEL */
{ 405, 884, NETWK_TYPE_DCS_1800, "STEL", "STEL" },
/* STEL */
{ 405, 885, NETWK_TYPE_GSM_900, "STEL", "STEL" },
/* STEL */
{ 405, 885, NETWK_TYPE_DCS_1800, "STEL", "STEL" },
/* STEL */
{ 405, 886, NETWK_TYPE_GSM_900, "STEL", "STEL" },
/* STEL */
{ 405, 886, NETWK_TYPE_DCS_1800, "STEL", "STEL" },
/* IDEA Cellular Limited */
{ 405, 908, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 908, NETWK_TYPE_DCS_1800, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 909, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 909, NETWK_TYPE_DCS_1800, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 910, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 910, NETWK_TYPE_DCS_1800, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 911, NETWK_TYPE_GSM_900, "IDEA", "IDEA" },
/* IDEA Cellular Limited */
{ 405, 911, NETWK_TYPE_DCS_1800, "IDEA", "IDEA" },
/* Cheers */
{ 405, 912, NETWK_TYPE_DCS_1800, "Cheers", "Cheers" },
/* Cheers */
{ 405, 912, NETWK_TYPE_GSM_900, "Cheers", "Cheers" },
/* Cheers */
{ 405, 913, NETWK_TYPE_DCS_1800, "Cheers", "Cheers" },
/* Cheers */
{ 405, 913, NETWK_TYPE_GSM_900, "Cheers", "Cheers" },
/* Cheers */
{ 405, 914, NETWK_TYPE_DCS_1800, "Cheers", "Cheers" },
/* Cheers */
{ 405, 914, NETWK_TYPE_GSM_900, "Cheers", "Cheers" },
/* Cheers */
{ 405, 915, NETWK_TYPE_DCS_1800, "Cheers", "Cheers" },
/* Cheers */
{ 405, 915, NETWK_TYPE_GSM_900, "Cheers", "Cheers" },
/* Cheers */
{ 405, 916, NETWK_TYPE_DCS_1800, "Cheers", "Cheers" },
/* Cheers */
{ 405, 916, NETWK_TYPE_GSM_900, "Cheers", "Cheers" },
/* Cheers */
{ 405, 917, NETWK_TYPE_DCS_1800, "Cheers", "Cheers" },
/* Cheers */
{ 405, 918, NETWK_TYPE_GSM_900, "Cheers", "Cheers" },
/* Cheers */
{ 405, 919, NETWK_TYPE_DCS_1800, "Cheers", "Cheers" },
/* Cheers */
{ 405, 919, NETWK_TYPE_GSM_900, "Cheers", "Cheers" },
/* Cheers */
{ 405, 920, NETWK_TYPE_DCS_1800, "Cheers", "Cheers" },
/* Cheers */
{ 405, 920, NETWK_TYPE_GSM_900, "Cheers", "Cheers" },
/* Cheers */
{ 405, 921, NETWK_TYPE_DCS_1800, "Cheers", "Cheers" },
/* Cheers */
{ 405, 921, NETWK_TYPE_GSM_900, "Cheers", "Cheers" },
/* Cheers */
{ 405, 922, NETWK_TYPE_DCS_1800, "Cheers", "Cheers" },
/* Cheers */
{ 405, 922, NETWK_TYPE_GSM_900, "Cheers", "Cheers" },
/* Cheers */
{ 405, 923, NETWK_TYPE_DCS_1800, "Cheers", "Cheers" },
/* Cheers */
{ 405, 923, NETWK_TYPE_GSM_900, "Cheers", "Cheers" },
/* Cheers */
{ 405, 924, NETWK_TYPE_DCS_1800, "Cheers", "Cheers" },
/* Cheers */
{ 405, 924, NETWK_TYPE_GSM_900, "Cheers", "Cheers" },
/* Uninor */
{ 405, 925, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 925, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 926, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 926, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 927, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 927, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 928, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 928, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
/* Uninor */
{ 405, 929, NETWK_TYPE_GSM_900, "Uninor", "Uninor" },
/* Uninor */
{ 405, 929, NETWK_TYPE_DCS_1800, "Uninor", "Uninor" },
{ 405, 930, NETWK_TYPE_DCS_1800, "Cheers", "Cheers" },
{ 405, 930, NETWK_TYPE_GSM_900, "Cheers", "Cheers" },
{ 405, 931, NETWK_TYPE_DCS_1800, "Cheers", "Cheers" },
{ 405, 931, NETWK_TYPE_GSM_900, "Cheers", "Cheers" },
/* Videocon */
{ 405, 932, NETWK_TYPE_GSM_900, "Videocon", "Videocon" },
/* Videocon */
{ 405, 932, NETWK_TYPE_DCS_1800, "Videocon", "Videocon" },
/* <<< SWA_seol _SHP_SWA_PLMN for India PLMN end <<< */
/*2008.10.14 서남아요청사항적용*/ 
 
/******************
 **** Pakistan ****
 ******************/
/* Mobilink */
{ 410,   1, NETWK_TYPE_GSM_900, "Mobilink", "Mobilink" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 410,   1, NETWK_TYPE_DCS_1800, "Mobilink", "Mobilink" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Ufone */
{ 410,   3, NETWK_TYPE_GSM_900, "Ufone", "Ufone" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Paktel  */
{ 410, 4, NETWK_TYPE_GSM_900, "ZONG", "ZONG" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 410, 4, NETWK_TYPE_DCS_1800, "ZONG", "ZONG" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Telenor Pakistan */
{ 410, 6, NETWK_TYPE_GSM_900, "Telenor", "Telenor" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 410, 6, NETWK_TYPE_DCS_1800, "Telenor", "Telenor" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Warid Telecom */
{ 410, 7, NETWK_TYPE_GSM_900, "Warid", "Warid" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 410, 7, NETWK_TYPE_DCS_1800, "Warid", "Warid" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/******************
 **** Afghanistan ****
 ******************/
/* Telephone Systems International Inc */
{ 412,   1, NETWK_TYPE_GSM_900, "AF AWCC", "AF AWCC" },
/* Telephone Systems International Inc */
{ 412,   1, NETWK_TYPE_DCS_1800, "AF AWCC", "AF AWCC" },
/* Telecom Development Company Afghanistan Ltd. */
{ 412,   20, NETWK_TYPE_GSM_900, "AF AWCC", "AF AWCC" },

/*******************
 **** Sri Lanka ****
 *******************/
/* Mobitel (Pvt) Limited */
{ 413,	 1, NETWK_TYPE_DCS_1800, "Mobitel", "Mobitel" },
/* MTN Networks (Pvt) Ltd */
{ 413,	 2, NETWK_TYPE_GSM_900, "Dialog", "Dialog" },
/* Celltel Lanka Limited */
{ 413,	 3, NETWK_TYPE_GSM_900, "Etisalat", "Etisalat" },
/* Airtel */
{ 413,	 5, NETWK_TYPE_GSM_900, "Airtel", "Airtel" },
/* Hutchison Telecommunications Lanka (Pte) Limited */
{ 413,	 8, NETWK_TYPE_GSM_900, "Hutch", "Hutch" }, 

/*****************
 **** Myanmar ****
 *****************/
/* Myanmar Posts and Telecommunications */
{ 414,   1, NETWK_TYPE_GSM_900, "MM 900", "MM 900" },

/*****************
 **** Lebanon ****
 *****************/
/* Alfa */
{ 415,   1, NETWK_TYPE_GSM_900, "Alfa", "Alfa" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* MTC Touch */
{ 415,   3, NETWK_TYPE_GSM_900, "MTC Touch", "MTC Touch" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/****************
 **** Jordan ****
 ****************/
/* Fastlink */
{ 416,   1, NETWK_TYPE_GSM_900, "zain JO", "zain JO" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Umniah */
{ 416,   3, NETWK_TYPE_DCS_1800, "Umniah", "Umniah" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* MOBILECOM */
{ 416,  77, NETWK_TYPE_GSM_900, "Orange JO", "Orange JO" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/***************
 **** Syria ****
 ***************/
 /* SYRIATEL */
{ 417,   1, NETWK_TYPE_GSM_900, "SYRIATEL", "SYRIATEL" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 417,   1, NETWK_TYPE_DCS_1800, "SYRIATEL", "SYRIATEL" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
 /* MTN Syria */
{ 417,   2, NETWK_TYPE_GSM_900, "MTN Syria", "MTN Syria" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 417,   2, NETWK_TYPE_DCS_1800, "MTN Syria", "MTN Syria" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* MOBILE SYRIA */
{ 417,   9, NETWK_TYPE_GSM_900, "MOBILE SYRIA", "MOBILE SYRIA" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Syriatel Holdings S.A */
{ 417,  93, NETWK_TYPE_GSM_900, "Syriatel", "Syriatel" },
/* Syriatel Holdings S.A */
{ 417,  93, NETWK_TYPE_DCS_1800, "Syriatel", "Syriatel" },

/****************
 **** Iraq ****
 ****************/
/* Asia Cell Telecommunications Company Ltd */
{ 418, 0, NETWK_TYPE_GSM_900, "ASIACELL", "ASIACELL" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* SanaTel */
{ 418, 2, NETWK_TYPE_GSM_900, "SanaTel", "SanaTel" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* ASIACELL */
{ 418,   5, NETWK_TYPE_GSM_900, "ASIACELL", "ASIACELL" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* SanaTel */
{ 418, 8, NETWK_TYPE_GSM_900, "SanaTel", "SanaTel" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Atheer */
{ 418,   20, NETWK_TYPE_GSM_900, "Zain Iraq", "Zain Iraq" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Zain Iraq */
{ 418,   30, NETWK_TYPE_GSM_900, "Zain Iraq", "Zain Iraq" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Korek Telecom */
{ 418,   40, NETWK_TYPE_GSM_900, "Korek Telecom", "Korek Telecom" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* mobitel */
{ 418,   45, NETWK_TYPE_UMTS, "mobitel", "mobitel" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/****************
 **** Kuwait ****
 ****************/
/* MTCNet */
{ 419,   2, NETWK_TYPE_GSM_900, "zain KW", "zain KW" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 419,   2, NETWK_TYPE_DCS_1800, "zain KW", "zain KW" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 419,   2, NETWK_TYPE_UMTS, "zain KW", "zain KW" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Wataniya Telecom */
{ 419,   3, NETWK_TYPE_GSM_900, "WATANIYA", "WATANIYA" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 419,   3, NETWK_TYPE_DCS_1800, "WATANIYA", "WATANIYA" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* VIVA */
{ 419,   4, NETWK_TYPE_GSM_900, "VIVA", "VIVA" }, 
{ 419,   4, NETWK_TYPE_DCS_1800, "VIVA", "VIVA" }, 
{ 419,   4, NETWK_TYPE_UMTS, "VIVA", "VIVA" }, 

/**********************
 **** Saudi Arabia ****
 **********************/
/* Al JAWAL */
{ 420,   1, NETWK_TYPE_GSM_900, "SA Al Jawal", "SA Al Jawal" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 420,   1, NETWK_TYPE_UMTS, "SA Al Jawal", "SA Al Jawal" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Mobily */
{ 420, 3, NETWK_TYPE_GSM_900, "mobily", "mobily" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 420, 3, NETWK_TYPE_UMTS, "mobily", "mobily" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* zain SA */
{ 420, 4, NETWK_TYPE_GSM_900, "zain SA", "zain SA" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 420, 4, NETWK_TYPE_DCS_1800, "zain SA", "zain SA" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 420, 4, NETWK_TYPE_UMTS, "zain SA", "zain SA" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 420,   7, NETWK_TYPE_GSM_900, "EAE", "EAE" },

/***************
 **** Yemen ****
 ***************/
/* SABAFON */
{ 421,   1, NETWK_TYPE_GSM_900, "SABAFON", "SABAFON" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Spacetel */
{ 421,   2, NETWK_TYPE_GSM_900, "MTN", "MTN" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/**************
 **** Oman ****
 **************/
/* OMANTEL */
{ 422,   2, NETWK_TYPE_GSM_900, "OMAN MOBILE", "OMAN MOBILE" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* nawras  */
{ 422, 3, NETWK_TYPE_GSM_900, "nawras", "nawras" },  // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/******************************
 **** United Arab Emirates ****
 ******************************/
/* UAE Etisalat */
{ 424,   2, NETWK_TYPE_GSM_900, "ETISALAT", "ETISALAT" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 424,   2, NETWK_TYPE_UMTS, "ETISALAT", "ETISALAT" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* du */
{ 424,   3, NETWK_TYPE_GSM_900, "du", "du" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 424,   3, NETWK_TYPE_DCS_1800, "du", "du" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 424,   3, NETWK_TYPE_UMTS, "du", "du" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/****************
 **** Israel ****
 ****************/
/* Partner Communications Company Ltd */
{ 425,   1, NETWK_TYPE_GSM_900, "Orange", "Orange" },   /* refer to selist20080115.xls for Orange [jm0122.yoon 20080122] */
/* Partner Communications Company Ltd */
{ 425,   1, NETWK_TYPE_DCS_1800, "Orange", "Orange" },   /* refer to selist20080115.xls for Orange [jm0122.yoon 20080122] */
/* Cellcom Israel Ltd */
{ 425,   2, NETWK_TYPE_DCS_1800, "Cellcom", "Cellcom" },
/* Cellcom Israel Ltd */
{ 425,   2, NETWK_TYPE_UMTS, "Cellcom", "Cellcom" },
/* PCL */
{ 425,	  3, NETWK_TYPE_GSM_900, "PCL", "PCL" },
{ 425,	  3, NETWK_TYPE_DCS_1800, "PCL", "PCL" },
{ 425,	  3, NETWK_TYPE_UMTS, "PCL", "PCL" },
 /* Wataniya */
{ 425,	  6, NETWK_TYPE_GSM_900, "Wataniya", "Wataniya" },
{ 425,	  6, NETWK_TYPE_DCS_1800, "Wataniya", "Wataniya" },
{ 425,	  6, NETWK_TYPE_UMTS, "Wataniya", "Wataniya" },


/*******************************
 **** Palestinian Authority ****
 *******************************/
/* Palestine Telecoms Co Plc */
{ 425,   5, NETWK_TYPE_GSM_900, "JAWWAL-PALESTINE", "JAWWAL-PALESTINE" },

/*****************
 **** Bahrain ****
 *****************/
/* Bahrain Telecommunications Company */
{ 426,   1, NETWK_TYPE_GSM_900, "BATELCO", "BATELCO" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* MTC-Vodafone */
{ 426,   2, NETWK_TYPE_GSM_900, "zain BH", "zain BH" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 426,   2, NETWK_TYPE_DCS_1800, "zain BH", "zain BH" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 426,   2, NETWK_TYPE_UMTS, "zain BH", "zain BH" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 426,   4, NETWK_TYPE_GSM_900, "VIVA", "VIVA" },
{ 426,	 4, NETWK_TYPE_DCS_1800, "VIVA", "VIVA" },
{ 426,	 4, NETWK_TYPE_UMTS, "VIVA", "VIVA" },

/***************
 **** Qatar ****
 ***************/
/* Q-Tel */
{ 427,   1, NETWK_TYPE_GSM_900, "Qatarnet", "Qatarnet" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 427,   1, NETWK_TYPE_DCS_1800, "Qatarnet", "Qatarnet" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 427,   2, NETWK_TYPE_GSM_900, "Vodafone Qatar", "Vodafone Qatar" },
{ 427,   2, NETWK_TYPE_DCS_1800, "Vodafone Qatar", "Vodafone Qatar" },
{ 427,   2, NETWK_TYPE_UMTS, "Vodafone Qatar", "Vodafone Qatar" },

/******************
 **** Mongolia ****
 ******************/
/* MobiCom */
{ 428,  99, NETWK_TYPE_GSM_900, "MN MobiCom", "MN MobiCom" },

/***************
 **** Nepal ****
 ***************/
/* Nepal Telecommunications Corporation */
{ 429,	 1, NETWK_TYPE_GSM_900, "Nepal Telecom", "Nepal Telecom" },
/* NCELL */
{ 429,	 2, NETWK_TYPE_GSM_900, "NCELL", "NCELL" },

/**************
 **** Iran ****
 **************/
/* TCI */
{ 432,  11, NETWK_TYPE_GSM_900, "IR MCI", "IR MCI" },
{ 432,  11, NETWK_TYPE_DCS_1800, "IR MCI", "IR MCI" },
/* TKC */
{ 432,  14, NETWK_TYPE_GSM_900, "TKC", "TKC" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* MTCE */
{ 432,  19, NETWK_TYPE_GSM_900, "MTCE", "MTCE" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Taliya */
{ 432, 32, NETWK_TYPE_GSM_900, "Taliya", "Taliya" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Irancell */
{ 432, 35, NETWK_TYPE_GSM_900, "Irancell", "Irancell" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 432, 35, NETWK_TYPE_DCS_1800, "Irancell", "Irancell" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/********************
 **** Uzbekistan ****
 ********************/
/* Buztel */
{ 434,   1, NETWK_TYPE_GSM_900, "Buztel", "Buztel" },
/* JV Uzmacom */
{ 434,   2, NETWK_TYPE_GSM_900, "Uzmacom", "Uzmacom" },
/* DAEWOO Unitel Company */
{ 434,   4, NETWK_TYPE_GSM_900, "Daewoo Unitel", "Daewoo Unitel" },
/* DAEWOO Unitel Company */
{ 434,   4, NETWK_TYPE_DCS_1800, "Daewoo Unitel", "Daewoo Unitel" },
/* Coscom */
{ 434,   5, NETWK_TYPE_GSM_900, "Coscom", "Coscom" },
/* Uzdunrobita JV */
{ 434,   7, NETWK_TYPE_GSM_900, "UZB-UZD", "UZB-UZD" },
/* Uzdunrobita JV */
{ 434,   7, NETWK_TYPE_DCS_1800, "UZB-UZD", "UZB-UZD" },

/********************
 *** Tajikistan ****
 ********************/
/* JSC Somoncom */
{ 436,   1, NETWK_TYPE_GSM_900, "Somoncom", "Somoncom" },
/* Indigo Tajikistan */
{ 436,   2, NETWK_TYPE_GSM_900, "Indigo-T", "Indigo-T" },
/* TT Mobile, Closed joint-stock company */
{ 436,   3, NETWK_TYPE_GSM_900, "TJK MLT", "TJK MLT" },
/* TT Mobile, Closed joint-stock company  */
{ 436,   3, NETWK_TYPE_DCS_1800, "TJK MLT", "TJK MLT" },
/* JOSA Babilon-Mobile */
{ 436,   4, NETWK_TYPE_GSM_900, "Babilon-M", "Babilon-M" },
/* Tajik Tel */
{ 436,   5, NETWK_TYPE_GSM_900, "TJT - Tajik Tel", "TJT - Tajik Tel" },

/*************************
   **** Kyrgyz Republic ****
 *************************/
/* Bitel Limited */
{ 437,   1, NETWK_TYPE_GSM_900, "Beeline KG", "Beeline KG" },  //2011.02.09 The PLMN Name is changed as the operator's requirement. (Hong Seongmin (alex46.hong@samsung.com))
/* BiMoCom Ltd */
{ 437, 5, NETWK_TYPE_GSM_900, "MEGACOM", "MEGACOM" }, 
/* BiMoCom Ltd */
{ 437, 5, NETWK_TYPE_DCS_1800, "MEGACOM", "MEGACOM" }, 

/**********************
 **** Turkmenistan ****
 **********************/
/* Barash Communication Technologies Inc. */
{ 438,   1, NETWK_TYPE_GSM_900, "BCTI", "BCTI" },
#endif /* FEATURE_SAMSUNG_SOUTHEAST_ASIA */

/**********************
 **** JAPAN ****
 **********************/
/*  NTT DoCoMo, Inc */
{ 440,  10, NETWK_TYPE_UMTS, "JP DoCoMo", "JP DoCoMo" },
/*  J-phone */
{ 440,  20, NETWK_TYPE_GSM_900, "Vodafone JP", "Vodafone JP" },
/*  J-phone */
{ 440,  20, NETWK_TYPE_UMTS, "Vodafone JP", "Vodafone JP" },

/**********************
 **** KOREA ****
 **********************/
/* Samsung Testbed */
{ 450, 1, NETWK_TYPE_UMTS, "SEC TEST BED", "SEC TEST BED" }, 
/* "KT Freetel Co., Ltd" */
{ 450, 2, NETWK_TYPE_UMTS, "KT", "KT" }, 
/* SK Telecom */
{ 450, 5, NETWK_TYPE_UMTS, "SKT", "SKT" }, 
/* "KT Freetel Co., Ltd" */
{ 450, 8, NETWK_TYPE_UMTS, "KT", "KT" }, 

/*****************
 **** Vietnam ****
 *****************/
/* Vietnam Mobile Telecom Service */
{ 452,   1, NETWK_TYPE_GSM_900, "VN MOBIFONE", "VN MOBIFONE" },
/* Vietnam Telecoms Services Co (GPC) */
{ 452,   2, NETWK_TYPE_GSM_900, "VINAPHONE", "VINAPHONE" },
/* Vietel Corporation */
{ 452, 4, NETWK_TYPE_GSM_900, "VIETTEL", "VIETTEL" }, 
{ 452,   5, NETWK_TYPE_GSM_900, "VIETNAMOBILE", "VIETNAMOBILE" }, 
{ 452,   7, NETWK_TYPE_GSM_900, "BEELINE VN", "BEELINE VN" },
{ 452,   8, NETWK_TYPE_UMTS, "EVNTelecom", "EVNTelecom" },

/*******************
 **** Hong Kong ****
 *******************/
/* Hong Kong CSL Limited */
{ 454,   0, NETWK_TYPE_GSM_900, "CSL", "CSL" },
/* Hong Kong CSL Limited */
{ 454,   0, NETWK_TYPE_DCS_1800, "CSL", "CSL" },
/* Hong Kong CSL Limited */
{ 454, 0, NETWK_TYPE_UMTS, "CSL", "CSL" }, 
/* Hong Kong CSL Limited */
{ 454, 2, NETWK_TYPE_GSM_900, "CSL", "CSL" }, 
/* Hong Kong CSL Limited */
{ 454, 2, NETWK_TYPE_DCS_1800, "CSL", "CSL" }, 
/* Hong Kong CSL Limited */
{ 454,   2, NETWK_TYPE_UMTS, "CSL", "CSL" },
/* Hutchison Telecom (HK) Ltd */
{ 454,   3, NETWK_TYPE_UMTS, "3", "3" },
/* Hutchison Telecom (HK) Ltd */
{ 454,   4, NETWK_TYPE_GSM_900, "3 (2G)", "3 (2G)" },
/* Hutchison Telecom (HK) Ltd */
{ 454,   4, NETWK_TYPE_DCS_1800, "3 (2G)", "3 (2G)" },
/* SmarTone Mobile Comms Ltd */
{ 454,   6, NETWK_TYPE_GSM_900, "SmarToneVodafone", "SmarToneVodafone" },
/* SmarTone Mobile Comms Ltd */
{ 454,   6, NETWK_TYPE_DCS_1800, "SmarToneVodafone", "SmarToneVodafone" },
/* New World PCS Ltd */
{ 454,  10, NETWK_TYPE_DCS_1800, "NEW WORLD", "NEW WORLD" },
/* Peoples Telephone Co Ltd */
{ 454,  12, NETWK_TYPE_DCS_1800, "CMCC PEOPLES", "CMCC PEOPLES" },
/* SmarTone Mobile Comms Ltd */
{ 454,  15, NETWK_TYPE_UMTS, "454-15", "454-15" },
/* PCCW Mobile */
{ 454,  16, NETWK_TYPE_DCS_1800, "PCCW 2G", "PCCW 2G" },
/* Hong Kong CSL Limited */
{ 454,  18, NETWK_TYPE_GSM_900, "CSL", "CSL" },
/* Hong Kong CSL Limited */
{ 454,  18, NETWK_TYPE_DCS_1800, "CSL", "CSL" },
/* Hong Kong CSL Limited */
{ 454, 18, NETWK_TYPE_UMTS, "CSL", "CSL" }, 
/* PCCW Mobile */
{ 454,  19, NETWK_TYPE_UMTS, "PCCW 3G", "PCCW 3G" },
/* P Plus Communicatoins Ltd */
{ 454,  22, NETWK_TYPE_DCS_1800, "HK P PLUS", "HK P PLUS" },

/***************
 **** Macau ****
 ***************/
/* SmarTone Mobile Communications (Macau) Ltd */
{ 455,   0, NETWK_TYPE_GSM_900, "SmarTone", "SmarTone" },
/* SmarTone Mobile Communications (Macau) Ltd */
{ 455,   0, NETWK_TYPE_DCS_1800, "SmarTone", "SmarTone" },
/* C.T.M. */
{ 455,   1, NETWK_TYPE_GSM_900, "CTM", "CTM" },
/* C.T.M. */
{ 455,   1, NETWK_TYPE_DCS_1800, "CTM", "CTM" },
/* Hutchison Telephone (Macau) Company Limited */
{ 455,   3, NETWK_TYPE_GSM_900, "HT Macau", "HT Macau" },
/* Hutchison Telephone (Macau) Company Limited */
{ 455,   3, NETWK_TYPE_DCS_1800, "HT Macau", "HT Macau" },
/* C.T.M. */
{ 455,   4, NETWK_TYPE_UMTS, "CTM", "CTM" },

/******************
 **** Cambodia ****
 ******************/
/* CamGSM */
{ 456,   1, NETWK_TYPE_GSM_900, "MOBITEL - KHM", "MOBITEL - KHM" },
/* Cambodia Samart Communication Co Ltd */
{ 456,   2, NETWK_TYPE_GSM_900, "KHM-Hello GSM", "KHM-Hello GSM" },
/* Cambodia Shinawatra */
{ 456,  18, NETWK_TYPE_DCS_1800, "CAMBODIA SHINAWATRA ", "CAMBODIA SHINAWATRA " },

/*************
 **** Lao ****
 *************/
/* Lao Telecommunications */
{ 457,   1, NETWK_TYPE_GSM_900, "LAO GSM", "LAO GSM" },
/* Enterprise of Telecommunications Lao (ETL) */
{ 457,   2, NETWK_TYPE_GSM_900, "ETL MOBILE NETWORK", "ETL MOBILE NETWORK" },
/* Lao Asia Telecommunication State Enterprise (LAT) */
{ 457,   3, NETWK_TYPE_GSM_900, "45703", "45703" },
/* Lao Asia Telecommunication State Enterprise (LAT) */
{ 457,   3, NETWK_TYPE_DCS_1800, "45703", "45703" },
/* Millicom Lao Co Ltd */
{ 457,   8, NETWK_TYPE_GSM_900, "TANGO LAO", "TANGO LAO" },
/* Millicom Lao Co Ltd */
{ 457,   8, NETWK_TYPE_DCS_1800, "TANGO LAO", "TANGO LAO" },

/***************
 **** China ****
 ***************/
/* China Mobile */
{ 460, 0, NETWK_TYPE_GSM_900, "CMCC", "CMCC" }, 
/* China Unicom */
{ 460, 1, NETWK_TYPE_GSM_900, "CU-GSM", "CU-GSM" }, 

/****************
 **** Taiwan ****
 ****************/
/* Far EasTone Telecommunications GSM 900/1800 */
{ 466,   1, NETWK_TYPE_GSM_900, "FarEasTone", "FarEasTone" },
/* Far EasTone Telecommunications GSM 900/1800 */
{ 466,   1, NETWK_TYPE_DCS_1800, "FarEasTone", "FarEasTone" },
/* Far EasTone Telecommunications GSM 900/1800 */
{ 466,   1, NETWK_TYPE_UMTS, "FarEasTone", "FarEasTone" },
/* KG Telecom */
{ 466,   6, NETWK_TYPE_DCS_1800, "TWN Tuntex GSM 1800", "TWN Tuntex GSM 1800" },
/*ACeS International Limited (AIL) */
{ 466,   68, NETWK_TYPE_GSM_SAT, "ACeS", "ACeS" },
/* KG Telecom */
{ 466,   88, NETWK_TYPE_DCS_1800, "KGT-ONLINE", "KGT-ONLINE" },
/* Taiwan 3G Mobile Network */
{ 466,   89, NETWK_TYPE_UMTS, "VIBO", "VIBO" },
/* Chunghwa Telecom */
{ 466,   92, NETWK_TYPE_GSM_900, "Chunghwa", "Chunghwa" },
/* Chunghwa Telecom */
{ 466,   92, NETWK_TYPE_DCS_1800, "Chunghwa", "Chunghwa" },
/* Chunghwa Telecom */
{ 466,   92, NETWK_TYPE_UMTS, "Chunghwa", "Chunghwa" },
/* MobiTai */
{ 466,   93, NETWK_TYPE_GSM_900, "MoBiTai", "MobiTai" },
/* Taiwan Cellular Corporation */
{ 466,   97, NETWK_TYPE_DCS_1800, "Taiwan Mobile", "Taiwan Mobile" },
/* Taiwan Cellular Corporation */
{ 466,   97, NETWK_TYPE_UMTS, "Taiwan Mobile", "Taiwan Mobile" },
/* TransAsia Telecommunications */
{ 466,   99, NETWK_TYPE_GSM_900, "Trans Asia", "Trans Asia" },

/********************
 ****            ****
 ********************/
/* Korea Posts and Telecommunications Corporation (KPTC) */
{ 467, 3, NETWK_TYPE_GSM_900, "SUNNET", "SUNNET" }, 
/* NEAT&T Ltd. */
{ 467,   193, NETWK_TYPE_GSM_900, "KP SUN", "KP SUN" },

/********************
 **** Bangladesh ****
 ********************/
/* Grameen Phone Ltd */
{ 470,	 1, NETWK_TYPE_GSM_900, "Grameenphone", "Grameenphone" },
/* TM International (Bangladesh) Ltd */
{ 470,	 2, NETWK_TYPE_GSM_900, "Robi", "Robi" }, // yg7948.park@samsung.com - PLMN Name 변경 사항 적용건( AKTEL -> Robi, WARID -> Airtel )
/* Sheba Telecom (Pvt.) Ltd. */
{ 470,	 3, NETWK_TYPE_GSM_900, "Banglalink", "Banglalink" }, 
/* Teletalk Bangladesh Ltd */
{ 470,	 4, NETWK_TYPE_GSM_900, "Teletalk", "Teletalk" }, 
/* Warid */
{ 470,	 7, NETWK_TYPE_GSM_900, "Airtel", "Airtel" }, // yg7948.park@samsung.com - PLMN Name 변경 사항 적용건( AKTEL -> Robi, WARID -> Airtel )
/* Sheba Telecom (PVT) Ltd */
{ 470,	 19, NETWK_TYPE_GSM_900, "BD ShebaWorld", "BD ShebaWorld" },

/******************
 **** Maldives ****
 ******************/
/* Dhivehi Raajjeyge Gulhun Private Ltd GSM 900 */
{ 472,   1, NETWK_TYPE_GSM_900, "MV DHIMOBILE", "MV DHIMOBILE" },
/* Wataniya Telecom Maldives Pvt. Ltd */
{ 472, 2, NETWK_TYPE_GSM_900, "WMOBILE", "WMOBILE" }, 

/******************
 **** Malaysia ****
 ******************/
/* Maxis Communications Berhad */ 
{ 502,  12, NETWK_TYPE_GSM_900, "MY MAXIS", "MY MAXIS" },
/* Maxis Communications Berhad */
{ 502,  12, NETWK_TYPE_DCS_1800, "MY MAXIS", "MY MAXIS" },
/* Celcom (Malaysia) Sdn Bhd */
{ 502,  13, NETWK_TYPE_DCS_1800, "MY CELCOM", "MY CELCOM" },
/* Celcom (Malaysia) Sdn Bhd */
{ 502, 13, NETWK_TYPE_UMTS, "MY CELCOM", "MY CELCOM" }, 
/* DiGi Telecommunications Sdn Bhd */
{ 502,  16, NETWK_TYPE_DCS_1800, "DiGi", "DiGi" },
/* Maxis Mobile Sdn Bhd */
{ 502,  17, NETWK_TYPE_DCS_1800, "MY MAXIS", "MY MAXIS" },
/* UMobile Sdn Bhd */
{ 502, 18, NETWK_TYPE_UMTS, "U Mobile", "U Mobile" }, 
/* Celcom (Malaysia) Sdn Bhd */
{ 502,  19, NETWK_TYPE_GSM_900, "MY CELCOM", "MY CELCOM" },

/*******************
 **** Australia ****
 *******************/
/* Telstra Mobile Comms */
{ 505,   1, NETWK_TYPE_GSM_900, "Telstra", "Telstra" },
/* Telstra Mobile Comms */
{ 505,   1, NETWK_TYPE_DCS_1800, "Telstra", "Telstra" },
/* Singtel Optus Limited */
{ 505,   2, NETWK_TYPE_GSM_900, "YES OPTUS", "YES OPTUS" },
/* Vodafone Pacific Limited */
{ 505,   3, NETWK_TYPE_GSM_900, "vodafone AU", "vodafone AU" },
/* Vodafone Pacific Limited */
{ 505,   3, NETWK_TYPE_DCS_1800, "vodafone AU", "vodafone AU" },
/* Hutchison 3G Australia Pty Limited */
{ 505,   6, NETWK_TYPE_UMTS, "3Telstra", "3Telstra" },
/* One.Tel Networks */
{ 505,   8, NETWK_TYPE_DCS_1800, "One.Tel", "One.Tel" },

/*******************
 **** Satellite ****
 *******************/
/* ACeS International Limited (AIL) - INDONESIA */
{ 510,   0, NETWK_TYPE_GSM_SAT, "ACeS", "ACeS" },

/*******************
 **** Indonesia ****
 *******************/
/* PT Indonesian Satellite Corporation Tbk (INDOSAT) */
{ 510,  1, NETWK_TYPE_GSM_900, "INDOSAT", "INDOSAT" },
/* PT Indonesian Satellite Corporation Tbk (INDOSAT) */
{ 510,  1, NETWK_TYPE_DCS_1800, "INDOSAT", "INDOSAT" },
/* AXIS */
{ 510,   8, NETWK_TYPE_DCS_1800, "AXIS", "AXIS" },
/* PT Telekomunikasi Selular */
{ 510,  10, NETWK_TYPE_GSM_900, "TELKOMSEL", "TELKOMSEL" },
/* PT Telekomunikasi Selular */
{ 510,  10, NETWK_TYPE_DCS_1800, "TELKOMSEL", "TELKOMSEL" },
/* Excelcom */
{ 510,  11, NETWK_TYPE_GSM_900, "XL", "XL" },
/* Excelcom */
{ 510,  11, NETWK_TYPE_DCS_1800, "XL", "XL" },
/* INDOSAT-M3 */
{ 510,  21, NETWK_TYPE_DCS_1800, "INDOSAT", "INDOSAT" },
/* INDOSAT-M3 */
{ 510,  21, NETWK_TYPE_GSM_900, "INDOSAT", "INDOSAT" },
/* INDOSAT-M3 */
{ 510,  21, NETWK_TYPE_DCS_1800, "INDOSAT", "INDOSAT" },
/* PT Hutchison CP Telecommunications */
{ 510, 89, NETWK_TYPE_DCS_1800, "3", "3" }, 
/* PT Hutchison CP Telecommunications */
{ 510, 89, NETWK_TYPE_UMTS, "3", "3" }, 

/********************
 **** TLS ****
 ********************/
/* Timor Telecom. */
{ 514,   2, NETWK_TYPE_GSM_900, "TLS-TT", "TLS-TT" },

/********************
 **** Philipines ****
 ********************/
/* Isla Comms Co. Inc. */
{ 515,   1, NETWK_TYPE_GSM_900, "ISLACOM", "ISLACOM" },
/* Globe Telecom, GMCR Inc. */
{ 515,   2, NETWK_TYPE_GSM_900, "Globe", "Globe" },
/* Globe Telecom, GMCR Inc. */
{ 515,   2, NETWK_TYPE_DCS_1800, "Globe", "Globe" },
/* Smart Communications Inc. */
{ 515,   3, NETWK_TYPE_GSM_900, "Smart", "Smart" },
/* Smart Communications Inc. */
{ 515,   3, NETWK_TYPE_DCS_1800, "Smart", "Smart" },
/* Digital Telecommunications Phils, Inc */
{ 515,   5, NETWK_TYPE_DCS_1800, "SUN", "SUN" },

/*******************
 **** Satellite ****
 *******************/
/* ACeS International Limited (AIL) - PHILIPPINES */
{ 515,  11, NETWK_TYPE_GSM_SAT, "ACeS", "ACeS" },
/* Connectivity Unlimited Resource Enterprise Inc */
{ 515,  18, NETWK_TYPE_UMTS, "Red Mobile", "Red Mobile" },

/******************
 **** Thailand ****
 ******************/
/* Advanced Info Service Plc */
{ 520,   1, NETWK_TYPE_GSM_900, "TH GSM", "TH GSM" },
/* Wireless Comm. Services Co. */
{ 520,  10, NETWK_TYPE_DCS_1800, "TH WCS", "TH WCS" },
/* ACT Mobile Company, Limited */
{ 520,  15, NETWK_TYPE_PCS_1900, "TH ACT 1900", "TH ACT 1900" },
/* Total Access Comms Co. */
{ 520,  18, NETWK_TYPE_DCS_1800, "DTAC", "DTAC" },
/* ACeS International Limited (AIL) - THAILAND */
{ 520,  20, NETWK_TYPE_GSM_SAT, "ACeS", "ACeS" },
/* Digital Phone Co Ltd */
{ 520,  23, NETWK_TYPE_DCS_1800, "TH GSM 1800", "TH GSM 1800" },
/* TA Orange Company Ltd */
{ 520,  99, NETWK_TYPE_DCS_1800, "TRUE", "TRUE" },

/*******************
 **** Singapore ****
 *******************/
/* Singapore Telecom Mobile Pte Ltd */
{ 525,   1, NETWK_TYPE_GSM_900, "SingTel", "SingTel" },
/* Singapore Telecom Mobile Pte Ltd */
{ 525,   1, NETWK_TYPE_DCS_1800, "SingTel", "SingTel" },
/* Singapore Telecom GSM 1800 */
{ 525,   2, NETWK_TYPE_DCS_1800, "SingTel", "SingTel" },
/* MobileOne (Asia) Pte Ltd */
{ 525,   3, NETWK_TYPE_GSM_900, "SGP-M1-3GSM", "SGP-M1-3GSM" },
/* MobileOne (Asia) Pte Ltd */
{ 525,   3, NETWK_TYPE_DCS_1800, "SGP-M1-3GSM", "SGP-M1-3GSM" },
/* MobileOne (Asia) Pte Ltd */
{ 525,   3, NETWK_TYPE_UMTS, "SGP-M1-3GSM", "SGP-M1-3GSM" },
/* StarHub Mobile Pte Ltd */
{ 525,   5, NETWK_TYPE_DCS_1800, "StarHub", "StarHub" },
/* StarHub Mobile Pte Ltd */
{ 525, 5, NETWK_TYPE_UMTS, "StarHub", "StarHub" }, 

/****************
 **** Brunei ****
 ****************/
/* B-Mobile Communications Sdn Bhd */
{ 528, 2, NETWK_TYPE_UMTS, "b-mobile", "b-mobile" }, 
/* DataStream Technology */
{ 528,  11, NETWK_TYPE_GSM_900, "BRU-DSTCom", "BRU-DSTCom" },

/*********************
 **** New Zealand ****
 *********************/
/* Vodafone Mobile NZ Limited */
{ 530,   1, NETWK_TYPE_GSM_900, "vodafone NZ", "vodafone NZ" },
{ 530,   5, NETWK_TYPE_UMTS, "Telecom NZ", "Telecom NZ" },
{ 530,  24, NETWK_TYPE_UMTS, "2degrees", "2degrees" },

#if !defined (FEATURE_SAMSUNG_SOUTHEAST_ASIA)
/*********************
 **** PNG ****
 *********************/
/* Pacific Mobile Communications */
{ 537,   1, NETWK_TYPE_GSM_900, "PNGBMobile", "PNGBMobile" },

/***************
 **** Tonga ****
 ***************/
/* Tonga Communications Corporation */
{ 539,   1, NETWK_TYPE_GSM_900, "U-CALL", "U-CALL" },

/***************
 ****       ****
 ***************/
/* Solomon Telekom Co Ltd */
{ 540, 1, NETWK_TYPE_GSM_900, "BREEZE", "BREEZE" }, 

/*******************
 **** Satellite ****
 *******************/
/* ACeS International Limited (AIL) - REGIONAL */
{ 541,   0, NETWK_TYPE_GSM_SAT, "ACeS", "ACeS" },

/*****************
 **** Vanuatu ****
 *****************/
/* Telecom Vanuatu Ltd */
{ 541,   1, NETWK_TYPE_GSM_900, "VUT SMILE", "VUT SMILE" },

/**************
 **** Fiji ****
 **************/
/* Vodafone Fiji Limited */
{ 542,   1, NETWK_TYPE_GSM_900, "FJ VODAFONE", "FJ VODAFONE" },

/************************
 **** American Samoa ****
 ************************/
/* Blue Sky */
{ 544,  11, NETWK_TYPE_PCS_1900, "Blue Sky", "Blue Sky" },

/**************
 **** KI ****
 **************/
/* Telecom Services Kiribati Limited (TSKL) */
{ 545,   9, NETWK_TYPE_GSM_900, "KL-Frigate", "KL-Frigate" },

/***********************
 **** New Caledonia ****
 ***********************/
/* OPT New Caledonia */
{ 546,   1, NETWK_TYPE_GSM_900, "NCL MOBILIS", "NCL MOBILIS" },

/**************************
 **** French Polynesia ****
 **************************/
/* Tikiphone S.A */
{ 547,  20, NETWK_TYPE_GSM_900, "F-VINI", "F-VINI" },

/********************
 **** Cook Islands ****
 ********************/
/* Telecom Cook Islands */
{ 548, 1, NETWK_TYPE_GSM_900, "KOKANET", "KOKANET" }, 

/********************
 **** Micronesia ****
 ********************/
/* FSM Telecommunications Corporation */
{ 550,   1, NETWK_TYPE_GSM_900, "FSM Telecom", "FSM Telecom" },

/***************
 **** Palau ****
 ***************/
/* Palau Mobile Corporation */
{ 552, 80, NETWK_TYPE_GSM_900, "PLWPMC", "PLWPMC" }, 

/***************
 **** Egypt ****
 ***************/
/* Mobinil */
{ 602,   1, NETWK_TYPE_GSM_900, "MobiNiL", " EGY MobiNiL" },// D0100306029 2011/9/16 Orange 사업자 요청
/* Vodafone */
{ 602,   2, NETWK_TYPE_GSM_900, "vodafone", "vodafone" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 602,   2, NETWK_TYPE_UMTS, "vodafone", "vodafone" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Etisalat */
{ 602,   3, NETWK_TYPE_GSM_900, "Etisalat", "Etisalat" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 602,   3, NETWK_TYPE_DCS_1800, "Etisalat", "Etisalat" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 602,   3, NETWK_TYPE_UMTS, "Etisalat", "Etisalat" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/*****************
 **** Algeria ****
 *****************/
/* Algerie Telecom */
{ 603,   1, NETWK_TYPE_GSM_900, "Mobilis", "Mobilis" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Orascom Telecom Algerie Spa */
{ 603,   2, NETWK_TYPE_GSM_900, "Djezzy", "Djezzy" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 603,   2, NETWK_TYPE_DCS_1800, "Djezzy", "Djezzy" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Wataniya Telecom Algerie */
{ 603, 3, NETWK_TYPE_GSM_900, "Nedjma", "Nedjma" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 603, 3, NETWK_TYPE_DCS_1800, "Nedjma", "Nedjma" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/*****************
 **** Morocco ****
 *****************/
/* Medi Telecom */
{ 604,   0, NETWK_TYPE_GSM_900, "Meditel", "Meditel" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Itissalat Al-Maghrib S.A */
{ 604,   1, NETWK_TYPE_GSM_900, "IAM", "IAM" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* WANA GSM */
{ 604,   2, NETWK_TYPE_GSM_900, "WANA GSM", "WANA GSM" },

/*****************
 **** Tunisia ****
 *****************/
 /* Orange */
{ 605,   1, NETWK_TYPE_GSM_900, "Orange", "Orange" },
/* Tunisie Telecom */
{ 605,   2, NETWK_TYPE_GSM_900, "TUNTEL", "TUNTEL" },
/* Orascom Telecom Tunisie */
{ 605,   3, NETWK_TYPE_GSM_900, "TUNISIANA", "TUNISIANA" },


/* Libyana Mobile Phone  */
{ 606, 0, NETWK_TYPE_GSM_900, "Libyana", "Libyana" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 606, 0, NETWK_TYPE_DCS_1800, "Libyana", "Libyana" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* AL MADAR Telecomm Company */
{ 606, 1, NETWK_TYPE_GSM_900, "Almadar", "Almadar" }, // 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341


/****************
 **** Gambia ****
 ****************/
/* Gambia Telecommunications Cellular Company Ltd */
{ 607,   1, NETWK_TYPE_GSM_900, "GAMCEL", "GAMCEL" },
/* Africell (Gambia) Ltd */
{ 607,   2, NETWK_TYPE_GSM_900, "AFRICELL", "AFRICELL" },

/*****************
 **** Senegal ****
 *****************/
/* Sonatel */
{ 608,   1, NETWK_TYPE_GSM_900, "SN ALIZE", "SN ALIZE" },
/* Sentel GSM */
{ 608,   2, NETWK_TYPE_GSM_900, "SN-SENTEL SG", "SN-SENTEL SG" },

/*****************
 **** Maurital ****
 *****************/
/* MATTEL */
{ 609,   1, NETWK_TYPE_GSM_900, "MR MATTEL", "MR MATTEL" },
/* MAURITEL MOBILES */
{ 609, 10, NETWK_TYPE_GSM_900, "MAURITEL", "MAURITEL" }, 

/**************
 **** Mali ****
 **************/
/* Malitel-SA */
{ 610,   1, NETWK_TYPE_GSM_900, "MALITEL ML", "MALITEL ML" },
/* Orange Mali SA */
{ 610,   2, NETWK_TYPE_GSM_900, "ORANGE ML", "ORANGE ML" },

/****************
 **** Guinea ****
 ****************/
/* Spacetel Guinee SA */
{ 611,   1, NETWK_TYPE_GSM_900, "Orange GN", "Orange GN" },   /* refer to selist20080115.xls for Orange [jm0122.yoon 20080122] */
/* Sotelgui SA */
{ 611,   2, NETWK_TYPE_GSM_900, "GN LAGUI", "GN LAGUI" },
/* Areeba Guinea */
{ 611, 4, NETWK_TYPE_GSM_900, "Areeba", "Areeba" }, 
/* Areeba Guinea */
{ 611, 4, NETWK_TYPE_DCS_1800, "Areeba", "Areeba" }, 

/***********************
 **** Cote d'Ivoire ****
 ***********************/
/* CORA de COMSTAR */
{ 612,   1, NETWK_TYPE_GSM_900, "CI CORA", "CI CORA" },
/* Atlantique Cellulaire */
{ 612, 2, NETWK_TYPE_GSM_900, "ACELL-CI", "ACELL-CI" }, 
/* Atlantique Cellulaire */
{ 612, 2, NETWK_TYPE_DCS_1800, "ACELL-CI", "ACELL-CI" }, 
/* S.I.M. */
{ 612,   3, NETWK_TYPE_GSM_900, "Orange CI", "Orange CI" },
/* Loteny Telecom (SA) */
{ 612,   5, NETWK_TYPE_GSM_900, "TELECEL-CI", "TELECEL-CI" },

/**********************
 **** Burkina Faso ****
 **********************/
/* Onatel */
{ 613,   1, NETWK_TYPE_GSM_900, "Onatel", "Onatel" },
/* Celtel Burkina Faso */
{ 613,   2, NETWK_TYPE_GSM_900, "BF CELTEL", "BF CELTEL" },

/***************
 **** Niger ****
 ***************/
/* Sahel-Com */
{ 614, 1, NETWK_TYPE_GSM_900, "SAHELCOM", "SAHELCOM" }, 
/* Celtel Niger */
{ 614,   2, NETWK_TYPE_GSM_900, "NE CELTEL", "NE CELTEL" },
/* Telecel Niger SA */
{ 614,   3, NETWK_TYPE_GSM_900, "NE TELECEL", "NE TELECEL" },
/* Orange Niger SA */
{ 614,	 4, NETWK_TYPE_GSM_900, "Orange NE", "Orange NE" }, /* refer to selist26-FEB-09.xls for Orange */
/* Orange Niger SA */
{ 614,	 4, NETWK_TYPE_DCS_1800, "Orange NE", "Orange NE" }, /* refer to selist26-FEB-09.xls for Orange */

/**************
 **** Togo ****
 **************/
/* Togo Cellulaire */
{ 615,   1, NETWK_TYPE_GSM_900, "TG-TOGO CELL", "TG-TOGO CELL" },

/***************
 **** Benin ****
 ***************/
/* LIBERCOM */
{ 616,   1, NETWK_TYPE_GSM_900, "LIBERCOM", "LIBERCOM" },
/* Telecel Benin Ltd */
{ 616,   2, NETWK_TYPE_GSM_900, "TELECEL BENIN", "TELECEL BENIN" },
/* Spacetel-Benin */
{ 616,   3, NETWK_TYPE_GSM_900, "BJ BENINCELL", "BJ BENINCELL" },
/* Bell Benin Communications (BBCOM) */
{ 616,  4, NETWK_TYPE_GSM_900, "BELL BENIN COMMUNICATION", "BELL BENIN COMMUNICATION" },
/* Bell Benin Communications (BBCOM) */
{ 616,  4, NETWK_TYPE_DCS_1800, "BELL BENIN COMMUNICATION", "BELL BENIN COMMUNICATION" },

/*******************
 **** Mauritius ****
 *******************/
/* Cellplus Mobile Comms */
{ 617,   1, NETWK_TYPE_GSM_900, "CELLPLUS-MRU", "CELLPLUS-MRU" },
/* Emtel Ltd */
{ 617,  10, NETWK_TYPE_GSM_900, "EMTEL-MRU", "EMTEL-MRU" },

/*****************
 **** Liberia ****
 *****************/
/* Lonestar Communications Corporation */
{ 618,   1, NETWK_TYPE_GSM_900, "LBR Lonestar Cell", "LBR Lonestar Cell" },
/* Atlantic Wireless (Liberia) Inc.  */
{ 618, 2, NETWK_TYPE_GSM_900, "LIBERCEL", "LIBERCEL" }, 
/* "Celcom Telecommunications, Inc" */
{ 618, 3, NETWK_TYPE_GSM_900, "Celcom", "Celcom" }, 
/* "Celcom Telecommunications, Inc" */
{ 618, 3, NETWK_TYPE_DCS_1800, "Celcom", "Celcom" }, 

/****************
 **** SL ****
*****************/
/* Celtel (SL) Limited */
{ 619,   1, NETWK_TYPE_GSM_900, "CELTEL SL", "CELTEL SL" },
/* Celtel (SL) Limited */
{ 619,   2, NETWK_TYPE_GSM_900, "MILLICOM SL", "MILLICOM SL" },

/***************
 **** Ghana ****
 ***************/
/* ScanCom Ltd */
{ 620,   1, NETWK_TYPE_GSM_900, "MTN", "MTN" },//2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Ghana Telecommunications Company Ltd */
{ 620,   2, NETWK_TYPE_GSM_900, "GH Onetouch", "GH Onetouch" },//2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Millicom Ghana Limited */
{ 620,   3, NETWK_TYPE_GSM_900, "Tigo", "Tigo" },//2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Zain Gh */
{ 620,   6, NETWK_TYPE_GSM_900, "Zain Gh", "Zain Gh" },//2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 620,   6, NETWK_TYPE_DCS_1800, "Zain Gh", "Zain Gh" },//2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/*****************
 **** Nigeria ****
 *****************/
/* Globacom Limited */
{ 621,  20, NETWK_TYPE_GSM_900, "Celtel Nigeria", "Celtel Nigeria" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 621,  20, NETWK_TYPE_DCS_1800, "Celtel Nigeria", "Celtel Nigeria" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 621,  20, NETWK_TYPE_UMTS, "Celtel Nigeria", "Celtel Nigeria" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/* MTN Nigeria Communications Limited */
{ 621,  30, NETWK_TYPE_GSM_900, "MTN Nigeria", "MTN Nigeria" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 621,  30, NETWK_TYPE_DCS_1800, "MTN Nigeria", "MTN Nigeria" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/* Nigerian Mobile Telecommunications Limited (MTEL) */
{ 621,  40, NETWK_TYPE_GSM_900, "Mtel", "Mtel" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 621,  40, NETWK_TYPE_DCS_1800, "Mtel", "Mtel" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/* Celtel Nigeria Limited */
{ 621,  50, NETWK_TYPE_GSM_900, "Glo Mobile", "Glo Mobile" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 621,  50, NETWK_TYPE_DCS_1800, "Glo Mobile", "Glo Mobile" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/* Emerging Markets Telecommunication */
{ 621,  60, NETWK_TYPE_GSM_900, "Etisalat", "Etisalat" },
{ 621,  60, NETWK_TYPE_DCS_1800, "Etisalat", "Etisalat" },
/**************
 **** Chad ****
 **************/
/* CelTel Tchad SA */
{ 622,   1, NETWK_TYPE_GSM_900, "CELTEL TCD", "CELTEL TCD" },
/* Tchad Mobile SA */
{ 622,   2, NETWK_TYPE_GSM_900, "TD LIBERTIS", "TD LIBERTIS" },

/******************
 **** Cameroon ****
 ******************/
/* MTN Cameroon Ltd */
{ 624,   1, NETWK_TYPE_GSM_900, "MTN CAM", "MTN CAM" },
/* Orange Cameroun S.A. */
{ 624,   2, NETWK_TYPE_GSM_900, "Orange CAM", "Orange CAM" },

/********************
 **** Cabo Verde ****
 ********************/
/* Cabo Verde Telecom */
{ 625,   1, NETWK_TYPE_GSM_900, "CPV MOVEL", "CPV MOVEL" },

/********************
 **** STP  ****
 ********************/
/* Companhia Santomense de Telecomunicacoes SARL */
{ 626,   1, NETWK_TYPE_GSM_900, "STP CSTmovel", "STP CSTmovel" },

/********************
 **** GNQ  ****
 ********************/
/* GETESA */
{ 627,   1, NETWK_TYPE_GSM_900, "GNQ01", "GNQ01" },

/***************
 **** Gabon ****
 ***************/
/* Libertis S.A. */
{ 628,   1, NETWK_TYPE_GSM_900, "LIBERTIS", "LIBERTIS" },
/* Telecel Gabon SA */
{ 628,   2, NETWK_TYPE_GSM_900, "GAB TELECEL", "GAB TELECEL" },
/* Celtel Gabon SA */
{ 628,   3, NETWK_TYPE_GSM_900, "CELTEL GA", "CELTEL GA" },

/***************
 **** Congo ****
 ***************/
/* CelTel Congo SA */
{ 629,   1, NETWK_TYPE_GSM_900, "CELTEL CD", "CELTEL CD" },
/* Libertis Telecom */
{ 629,   10, NETWK_TYPE_GSM_900, "COG LIBERTIS", "COG LIBERTIS" },


/* Vodacom Congo (RDC) sprl */
{ 630,   1, NETWK_TYPE_GSM_900, "Vodacom", "Vodacom" },
/* Vodacom Congo (RDC) sprl */
{ 630,   1, NETWK_TYPE_DCS_1800, "Vodacom", "Vodacom" },
/* CelTel Congo SA */
{ 630,   2, NETWK_TYPE_GSM_900, "CELTEL RC", "CELTEL RC" },
/* CELLCO Sarl */
{ 630,   4, NETWK_TYPE_GSM_900, "CELLCO GSM", "CELLCO GSM" },
/* Supercell Sprl */
{ 630, 5, NETWK_TYPE_GSM_900, "SCELL", "SCELL" }, 
/* Supercell Sprl */
{ 630, 5, NETWK_TYPE_DCS_1800, "SCELL", "SCELL" }, 
/* SAIT Telecom SPRL */
{ 630,  89, NETWK_TYPE_DCS_1800, "CD OASIS", "CD OASIS" },

/*****************
 **** Somalia ****
 *****************/
/* UNITEL S.a.r.l. */
{ 631,   1, NETWK_TYPE_GSM_900, "UNITEL", "UNITEL" },
/* UNITEL */
{ 631,   2, NETWK_TYPE_GSM_900, "UNITEL", "UNITEL" }, //angola 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 631,   2, NETWK_TYPE_DCS_1800, "UNITEL", "UNITEL" }, //angola 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Telsom Mobile Somalia */
{ 631,  82, NETWK_TYPE_GSM_900, "telsom", "telsom" },

/* Orange Bissau GNB*/
{ 632, 3, NETWK_TYPE_GSM_900, "Orange BS", "Orange BS" },   /* refer to selist20080115.xls for Orange [jm0122.yoon 20080122] */
/* Orange Bissau GNB */
{ 632, 3, NETWK_TYPE_DCS_1800, "Orange BS", "Orange BS" },   /* refer to selist20080115.xls for Orange [jm0122.yoon 20080122] */
/* Guinetel */
{ 632, 7, NETWK_TYPE_GSM_900, "GTM", "GTM" }, 

/********************
 **** Seychelles ****
 ********************/
/* Cable & Wireless (Seychelles) Ltd */
{ 633,   1, NETWK_TYPE_GSM_900, "SEYCEL", "SEYCEL" },
/* Cable & Wireless (Seychelles) */
{ 633, 1, NETWK_TYPE_GSM_900, "C&W SEY", "C&W SEY" }, 
/* Telecom (Seychelles Limited) */
{ 633,  10, NETWK_TYPE_GSM_900, "SEZ AIRTEL", "SEZ AIRTEL" },


/***************
 **** Sudan ****
 ***************/
/* Zain SUDAN */
{ 634,   1, NETWK_TYPE_GSM_900, "Zain SUDAN", "Zain SUDAN" },
/* Bashair Telecom Co.Ltd */
{ 634, 2, NETWK_TYPE_GSM_900, "areeba", "areeba" },
/* Bashair Telecom Co.Ltd */
{ 634, 2, NETWK_TYPE_DCS_1800, "areeba", "areeba" },

/****************
 **** Rwanda ****
 ****************/
/* Rwandacell SARL */
{ 635,  10, NETWK_TYPE_GSM_900, "R-CELL", "R-CELL" },

/******************
 **** Ethiopia ****
 ******************/
/* Ethiopian Telecoms Auth. */
{ 636,   1, NETWK_TYPE_GSM_900, "ETH-MTN", "ETH-MTN" },

/******************
 **** Somalia ****
 ******************/
/* Telesom Company */
{ 637,   1, NETWK_TYPE_GSM_900, "SOMTELESOM", "SOMTELESOM" },
/* Somafone FZLLC  */
{ 637, 4, NETWK_TYPE_GSM_900, "SOMAFONE", "SOMAFONE" }, 
/* Somafone FZLLC  */
{ 637, 4, NETWK_TYPE_DCS_1800, "SOMAFONE", "SOMAFONE" }, 
/* Golis Telecommunications Company Ltd */
{ 637, 30, NETWK_TYPE_GSM_900, "Golis", "Golis" }, 
/* Telsom Mobile Somalia*/
{ 637,   82, NETWK_TYPE_GSM_900, "Telsom Mobile", "Telsom Mobile" },

/******************
 **** DJ ****
 ******************/
/* Djibouti Telecom SA */
{ 638,   1, NETWK_TYPE_GSM_900, "DJ EVATIS", "DJ EVATIS" },

/***************
 **** Kenya ****
 ***************/
/* Safaricom Limited */
{ 639,   2, NETWK_TYPE_GSM_900, "Safaricom", "Safaricom" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 639,   2, NETWK_TYPE_DCS_1800, "Safaricom", "Safaricom" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Celtel Kenya Limited */
{ 639,   3, NETWK_TYPE_GSM_900, "ZAIN KE", "ZAIN KE" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Econet */
{ 639,   5, NETWK_TYPE_GSM_900, "Econet", "Econet" },//신규사업자 09년 1월 런칭예정 , 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Telkom Kenya Limited */
{ 639,   7, NETWK_TYPE_GSM_900, "Orange", "Orange" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 639,   7, NETWK_TYPE_DCS_1800, "Orange", "Orange" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/******************
 **** Tanzania ****
 ******************/
/* Tritel (T) Ltd */
{ 640,   1, NETWK_TYPE_GSM_900, "TRITEL-TZ", "TRITEL-TZ" },
/* MIC Tanzania Ltd */
{ 640,   2, NETWK_TYPE_GSM_900, "Tigo", "Tigo" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 640,   2, NETWK_TYPE_DCS_1800, "Tigo", "Tigo" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/* Zanzibar Telecom */
{ 640,   3, NETWK_TYPE_GSM_900, "ZANTEL", "ZANTEL" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 640,   3, NETWK_TYPE_DCS_1800, "ZANTEL", "ZANTEL" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/* Vodacom Tanzania Ltd */
{ 640,   4, NETWK_TYPE_GSM_900, "Vodacom", "Vodacom" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 640,   4, NETWK_TYPE_DCS_1800, "Vodacom", "Vodacom" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/* Celtel Tanzania Limited */
{ 640,   5, NETWK_TYPE_GSM_900, "ZAIN TZ", "ZAIN TZ" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 640,   5, NETWK_TYPE_DCS_1800, "ZAIN TZ", "ZAIN TZ" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Celtel Tanzania Limited ******
{ 640, 5, NETWK_TYPE_GSM_400, "celtel", "celtel" }, *****/

/****************
 **** Uganda ****
 ****************/
/* Celtel Uganda Limited (Zain Uganda) */
{ 641,   1, NETWK_TYPE_GSM_900, "Celtel", "Celtel" },
/* MTN Uganda Ltd */
{ 641,  10, NETWK_TYPE_GSM_900, "MTN UG", "MTN UG" },
/* Uganda Telecom Ltd */
{ 641,  11, NETWK_TYPE_GSM_900, "mango", "mango" },
/* Orange Uganda Limited */
{ 641,  14, NETWK_TYPE_GSM_900, "OUL", "ORANGE UGANDA" },// D0100306029 2011/9/16 Orange 사업자 요청
/* Orange Uganda Limited */
{ 641,  14, NETWK_TYPE_DCS_1800, "OUL", "ORANGE UGANDA" },// D0100306029 2011/9/16 Orange 사업자 요청

/*****************
 **** Burundi ****
 *****************/
/* Spacetel - Burundi */
{ 642,   1, NETWK_TYPE_GSM_900, "Spacetel BI", "Spacetel BI" },
/* Africell PLC Company */
{ 642,   2, NETWK_TYPE_GSM_900, "BUSAFA", "BUSAFA" },
/* ONATEL  */
{ 642, 3, NETWK_TYPE_GSM_900, "ONATEL", "ONATEL " }, 
/* Telecel-Burundi Company */
{ 642, 82, NETWK_TYPE_GSM_900, "BDITL", "BDITL" }, 


/********************
 **** Mozambique ****
 ********************/
/* SS_Protocol park071 20090811 - Mozambique PLMN fix (L760 JC version) */
/* Mozambique Celluar S.A.R.L. */
{ 643,   1, NETWK_TYPE_GSM_900, "mCel", "mCel" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* Mozambique Celluar S.A.R.L. */
{ 643,   1, NETWK_TYPE_DCS_1800, "mCel", "mCel" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* VM, S.A.R.L. */
{ 643,   4, NETWK_TYPE_GSM_900, "Vodacom", "Vodacom" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
/* VM, S.A.R.L. */
{ 643,   4, NETWK_TYPE_DCS_1800, "Vodacom", "Vodacom" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/****************
 **** Zambia ****
 ****************/
/* Celtel Zambia */
{ 645,   1, NETWK_TYPE_GSM_900, "ZM CELTEL", "ZM CELTEL" },
/* Telecel Zambia Limited */
{ 645,   2, NETWK_TYPE_GSM_900, "Telecel", "Telecel" },

/********************
 **** Madagascar ****
 ********************/
/* Madacom SA */
{ 646,   1, NETWK_TYPE_GSM_900, "MG Madacom", "MG Madacom" },
/* Orange Madagascar S.A. */
{ 646,   2, NETWK_TYPE_GSM_900, "Orange MG", "Orange MG" }, /* refer to selist26-FEB-09.xls for Orange */

/**********************
 **** Reunion (La) ****
 **********************/
/* Orange Reunion */
{ 647,   0, NETWK_TYPE_GSM_900, "Orange re", "Orange re" },
/* Orange Reunion */
{ 647,   0, NETWK_TYPE_DCS_1800, "Orange re", "Orange re" },
/* Outremer Telecom */
{ 647,   2, NETWK_TYPE_DCS_1800, "F-OMT", "F-OMT" },
/* Societe Reunionnaise */
{ 647,  10, NETWK_TYPE_GSM_900, "SFR RU", "SFR RU" },  //2008.12.01 현채인 Zineb 요청 사항

/******************
 **** Zimbabwe ****
 ******************/
/* Net*One Cellular (Pvt) Ltd */
{ 648,   1, NETWK_TYPE_GSM_900, "ZW NET*ONE", "ZW NET*ONE" },
/* Telecel Zimbabwe (PVT) Ltd */
{ 648,   3, NETWK_TYPE_GSM_900, "TELECEL ZW", "TELECEL ZW" },
/* Econet Wireless (Private) Limited */
{ 648,   4, NETWK_TYPE_GSM_900, "ZW ECONET", "ZW ECONET" },

/*****************
 **** Namibia ****
 *****************/
/* MTC */
{ 649,   1, NETWK_TYPE_GSM_900, "MTC NAMIBIA", "MTC NAMIBIA" },

/****************
 **** Malawi ****
 ****************/
/* Telekom Network Ltd */
{ 650,   1, NETWK_TYPE_GSM_900, "MW CP 900", "MW CP 900" },
/* CelTel Limited */
{ 650,  10, NETWK_TYPE_GSM_900, "CELTEL MW", "CELTEL MW" },

/*****************
 **** Lesotho ****
 *****************/
/* Vodacom Lesotho (Pty) Ltd */
{ 651,   1, NETWK_TYPE_GSM_900, "VCL COMMS", "VCL COMMS" },
/* Econet Ezi Cel Lesotho (Pty) */
{ 651,   2, NETWK_TYPE_GSM_900, "LS-ECONET-EZI-CEL", "LS-ECONET-EZI-CEL" },

/******************
 **** Botswana ****
 ******************/
/* Mascom Wireless (Pty) Limited */
{ 652,   1, NETWK_TYPE_GSM_900, "BW MASCOM", "BW MASCOM" },
/* Orange (Botswana) Pty Limited */
{ 652,   2, NETWK_TYPE_GSM_900, "Orange", "Orange" },

/*******************
 **** Swaziland ****
 *******************/
/* Swazi MTN Limited */
{ 653,  10, NETWK_TYPE_GSM_900, "Swazi-MTN", "Swazi-MTN" },


/* Societe Nationale des Telecommunications (Comores Telecom) */
{ 654, 1, NETWK_TYPE_GSM_900, "HURI", "HURI" }, 


/**********************
   **** South Africa ****
 **********************/
/* request from vodacom (south africa) 7 Jan 2005 */
/* Vodacom (Pty) Ltd */
{ 655,   1, NETWK_TYPE_GSM_900, "RSA Vodacom", "RSA Vodacom" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 655,   1, NETWK_TYPE_UMTS, "RSA Vodacom", "RSA Vodacom" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/*Telkom ,Ltd */
{ 655,   2, NETWK_TYPE_GSM_900, "RSA 8ta", "RSA 8ta" },// 2009.9.1 중아개발그룹 이용구 책임 요청
{ 655,   2, NETWK_TYPE_DCS_1800,  "RSA 8ta", "RSA 8ta" },// 2009.9.1 중아개발그룹 이용구 책임 요청
{ 655,   2, NETWK_TYPE_UMTS,  "RSA 8ta", "RSA 8ta" },// 2009.9.1 중아개발그룹 이용구 책임 요청

/* Cell C (Pty) Ltd */
{ 655,   7, NETWK_TYPE_GSM_900, "RSA CELL C", "RSA CELL C" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 655,   7, NETWK_TYPE_DCS_1800, "RSA CELL C", "RSA CELL C" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/* MTN - Mobile Telephone Networks (Pty) Ltd. */
{ 655,  10, NETWK_TYPE_GSM_900, "RSA MTN", "RSA MTN" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 655,  10, NETWK_TYPE_DCS_1800, "RSA MTN", "RSA MTN" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341
{ 655,  10, NETWK_TYPE_UMTS, "RSA MTN", "RSA MTN" },// 2009.9.1 중아개발그룹 박영기 선임 요청. CR B0100230341

/*********************
 ****            ****
 *********************/
/* Belize Telecommunications Ltd */
{ 702,   67, NETWK_TYPE_PCS_1900, "BTL", "BTL" },
/* International Telecommunication Limited (INTELCO) */
{ 702,   68, NETWK_TYPE_PCS_1900, "INTELCO", "INTELCO" },

/*********************
 **** Guatemala ****
 *********************/
#if defined(FEATURE_SAMSUNG_MXTELCEL) 
/* Claro GTM */
{ 704, 1, NETWK_TYPE_PCS_1900, "ClaroGTM", "Claro GTM" },
#elif defined(FEATURE_SAMSUNG_ARCTI)  
/* SERCOM S.A. (Guatemala) */
{ 704, 1, NETWK_TYPE_PCS_1900, "SERCOM", "SERCOM" },
#else
/* SERCOM S.A. (Guatemala) */
{ 704, 1, NETWK_TYPE_PCS_1900, "Claro", "Claro" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif
/* COMCEL-Communicaciones Celulares Sociedad Anonima */
{ 704, 2, NETWK_TYPE_GSM_850, "TIGO", "TIGO" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
{ 704, 3, NETWK_TYPE_PCS_1900, "movistar", "movistar" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#if defined (FEATURE_SAMSUNG_ARCTI)
{ 704, 12, NETWK_TYPE_PCS_1900, "SERCOM", "SERCOM" }, 
#else
{ 704, 12, NETWK_TYPE_PCS_1900, "Claro", "Claro" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif

/*********************
 **** El Salvador ****
 *********************/
#if defined(FEATURE_SAMSUNG_MXTELCEL)  
/* Claro SLV */
{ 706,   1, NETWK_TYPE_PCS_1900, "ClaroSLV", "Claro SLV" },
#elif defined(FEATURE_SAMSUNG_ARCTI)
{ 706,   1, NETWK_TYPE_PCS_1900, "CTE Personal", "CTE Personal" },
#elif defined(FEATURE_SAMSUNG_COMCEL)	
{ 706,   1, NETWK_TYPE_PCS_1900, "Claro SLV", "Claro SLV" },
#else
/* CTE Telecom Personal SA de CV */
{ 706,   1, NETWK_TYPE_PCS_1900, "Claro", "Claro" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif
/* DIGICEL, S.A. de C.V. */
{ 706,   2, NETWK_TYPE_GSM_900, "Digicel", "Digicel" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Telemovil EL Salvador S.A */
{ 706,  3, NETWK_TYPE_GSM_850, "TIGO", "TIGO" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* "Telefonica Moviles El Salvador, S.A de c.v" */
{ 706, 4, NETWK_TYPE_GSM_850, "movistar", "movistar" }, 
/* PERSONAL */
{ 706,  10, NETWK_TYPE_PCS_1900, "PERSONAL", "PERSONAL" }, /* M625000000623 */

/*********************
 **** Honduras ****
 *********************/
#if defined(FEATURE_SAMSUNG_MXTELCEL) 
/* Claro HND */
{ 708,   0, NETWK_TYPE_PCS_1900, "Claro HND", "Claro HND" },
{ 708,   1, NETWK_TYPE_PCS_1900, "Claro HND", "Claro HND" },
#elif defined(FEATURE_SAMSUNG_ARCTI)
{ 708,   0, NETWK_TYPE_PCS_1900, "Megatel", "Megatel" },
{ 708,   1, NETWK_TYPE_PCS_1900, "Megatel", "Megatel" },
#elif defined(FEATURE_SAMSUNG_COMCEL)	
{ 708,   0, NETWK_TYPE_PCS_1900, "Claro HND", "Claro HND" },
{ 708,   1, NETWK_TYPE_PCS_1900, "Claro HND", "Claro HND" },
#else
/* Megatel S.A. de C.V. */
{ 708,   0, NETWK_TYPE_PCS_1900, "Claro", "Claro" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
{ 708,   1, NETWK_TYPE_PCS_1900, "Claro", "Claro" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif
/* Telefonica Celular S.A (CELTEL) */
{ 708,   2, NETWK_TYPE_GSM_900, "TIGO", "TIGO" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Empresa Hondurena de Telecomunicaciones HONDUTEL */
{ 708, 30, NETWK_TYPE_PCS_1900, "HT - 200", "HT - 200" }, 

/*********************
 **** Nicaragua ****
 *********************/
#if defined(FEATURE_SAMSUNG_MXTELCEL) 
/* Claro NIC */
{ 710, 21, NETWK_TYPE_PCS_1900, "ClaroNIC", "Claro NIC" }, 
#elif defined(FEATURE_SAMSUNG_ARCTI) 
{ 710, 21, NETWK_TYPE_PCS_1900, "ENITEL", "ENITEL" }, 
#else
{ 710, 21, NETWK_TYPE_PCS_1900, "Claro", "Claro" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif
/* Telefonica Moviles S.A. */
{ 710, 30, NETWK_TYPE_GSM_850, "movistar", "Movistar" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#if defined(FEATURE_SAMSUNG_ARCTI)
/* SERCOM S.A. (Nicaragua) */
{ 710, 73, NETWK_TYPE_PCS_1900, "SERCOM", "SERCOM" }, 
{ 710, 300, NETWK_TYPE_GSM_850, "movistar", "Movistar" },
{ 710, 730, NETWK_TYPE_PCS_1900, "SERCOM", "SERCOM" }, 
#else
{ 710, 73, NETWK_TYPE_PCS_1900, "Claro", "Claro" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
{ 710, 300, NETWK_TYPE_GSM_850, "movistar", "movistar" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
{ 710, 730, NETWK_TYPE_PCS_1900, "Claro", "Claro" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif

/***********************
 ****   Costarica   ****
 **********************/
/* Belize Telecommunications Ltd */
{ 712,   1, NETWK_TYPE_DCS_1800, "ICE", "ICE" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* I.C.E. (Instituto Costarricense de Electricidad) */
{ 712, 2, NETWK_TYPE_DCS_1800, "ICE", "ICE" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Belize Telecommunications Ltd */
{ 712,   10, NETWK_TYPE_DCS_1800, "ICE", "ICE" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* I.C.E. (Instituto Costarricense de Electricidad) */
{ 712, 21, NETWK_TYPE_DCS_1800, "ICE", "ICE" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청

/*********************
 **** Panama ****
 *********************/
/* Cable & Wireless Panama */
{ 714,  1, NETWK_TYPE_GSM_850, "+Movil", "+Movil" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* "TELEFONICA MOVILES PANAMA, S.A." */
{ 714,  2, NETWK_TYPE_GSM_850, "movistar", "movistar" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#if defined(FEATURE_SAMSUNG_MXTELCEL) 
{ 714,  3, NETWK_TYPE_GSM_850, "CLARO PA", "CLARO PA" }, 
#elif defined (FEATURE_SAMSUNG_COMCEL) 
{ 714,  3, NETWK_TYPE_GSM_850, "Claro Pa", "Claro Pa" }, 
#else
{ 714,  3, NETWK_TYPE_GSM_850, "Claro", "Claro" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif
{ 714,  4, NETWK_TYPE_GSM_850, "Digicel", "Digicel" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Cable & Wireless Panama */
{ 714,  11, NETWK_TYPE_GSM_850, "+Movil", "+Movil" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* "TELEFONICA MOVILES PANAMA, S.A." */
{ 714, 20, NETWK_TYPE_GSM_850, "movistar", "movistar" }, 

/**************
 **** Peru ****
 **************/
/* Telefonica Moviles S.A. */
{ 716, 6, NETWK_TYPE_GSM_850, "movistar", "movistar" }, 
#if defined(FEATURE_SAMSUNG_MXTELCEL ) 
/* Claro Peru */
{ 716,  10, NETWK_TYPE_PCS_1900, "Claro PER", "Claro PER" },
#else
{ 716,  10, NETWK_TYPE_PCS_1900, "Claro", "Claro" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif

/*******************
 **** Argentina ****
 *******************/
/* UNIFON */
{ 722,   7, NETWK_TYPE_PCS_1900, "movistar", "movistar" },  // 2009.09.01 중남미 개발 그룹 안병우 선임 요청 
/* CTI Movil ARG */
#if defined( FEATURE_SAMSUNG_MXTELCEL)
/* CTI Movil ARG */
{ 722,  31, NETWK_TYPE_PCS_1900, "CLARO AR", "CLARO ARGENTINA" },
#elif defined (FEATURE_SAMSUNG_COMCEL)		
/* CTI Movil ARG */
{ 722,  31, NETWK_TYPE_PCS_1900, "CTI Movil", "CTI Movil" },
#else
{ 722,  31, NETWK_TYPE_GSM_850, "Claro AR", "Claro AR" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청 
{ 722,  31, NETWK_TYPE_PCS_1900, "Claro AR", "Claro AR" },  // 2009.09.01 중남미 개발 그룹 안병우 선임 요청 
#endif
/* Telecom Personal S.A. */
{ 722,  34, NETWK_TYPE_PCS_1900, "Personal", "Personal" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청 
/* PORT-HABLE */
{ 722,  35, NETWK_TYPE_GSM_900, "PORT-HABLE", "PORT-HABLE" },
{ 722,   71, NETWK_TYPE_PCS_1900, "movistar", "movistar" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청 

#if defined( FEATURE_SAMSUNG_MXTELCEL)
/* CTI Movil ARG */
{ 722,  310, NETWK_TYPE_PCS_1900, "CLARO AR", "CLARO ARGENTINA" },
#elif defined (FEATURE_SAMSUNG_COMCEL)		
/* CTI Movil ARG */
{ 722,  310, NETWK_TYPE_PCS_1900, "CTI Movil", "CTI Movil" },
#else
/* CTI Compania de Telefonos del Interior S.A. */
{ 722,  310, NETWK_TYPE_GSM_850, "Claro AR", "Claro AR" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청 
/* CTI PCS S.A. */
{ 722,  310, NETWK_TYPE_PCS_1900, "Claro AR", "Claro AR" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청   
#endif
/* Telecom Personal S.A. */
{ 722,  341, NETWK_TYPE_PCS_1900, "Personal", "Personal" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청   

/***************
 **** Brasil ****
 ***************/
/* TIM Celular S.A. */
{ 724, 2, NETWK_TYPE_DCS_1800, "TIM", "TIM" }, 
/* TIM Celular S.A. */
{ 724, 3, NETWK_TYPE_DCS_1800, "TIM", "TIM" }, 
/* TIM Celular S.A. */
{ 724, 4, NETWK_TYPE_DCS_1800, "TIM", "TIM" }, 

/* Albra Telecommunicacoes Ltda */
{ 724, 5, NETWK_TYPE_DCS_1800, "Claro", "Claro" }, 

/*********************************************************************
* Date                   : 30th Oct, 2007
* Location               : Brasil
* Description of problem : UE does not display PLMN. (724-10)
* Code Change by         : Kang na-young
* Patch Description      :if NITZ include only space on N/W name, skip to copy N/W name field
* Bug Id                 : A827 E-CIM 1723535
* Dependencies           : QmochaNw_fnc.c QmochaNw_table.h
**********************************************************************/
/* VIVO */
{ 724, 6, NETWK_TYPE_GSM_850, "VIVO", "VIVO" },   //항목 추가 
/* VIVO */
{ 724, 6, NETWK_TYPE_PCS_1900, "VIVO", "VIVO" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* VIVO */
{ 724, 10, NETWK_TYPE_GSM_850, "VIVO", "VIVO" },   //항목 추가 
/* VIVO */
{ 724, 10, NETWK_TYPE_PCS_1900, "VIVO", "VIVO" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* VIVO */
{ 724, 11, NETWK_TYPE_GSM_850, "VIVO", "VIVO" },   //항목 추가 
/* VIVO */
{ 724, 11, NETWK_TYPE_PCS_1900, "VIVO", "VIVO" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청

/* Sercomtel Celular S/A */
{ 724, 15, NETWK_TYPE_GSM_900, "Sercomtel", "Sercomtel" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Sercomtel Celular S/A */
{ 724, 15, NETWK_TYPE_DCS_1800, "Sercomtel", "Sercomtel" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청

#if defined(FEATURE_SAMSUNG_ARCTI)
/* 14 Brasil Telecom Celular S.A */
{ 724, 16, NETWK_TYPE_DCS_1800, "Telecom", "Telecom" },
#else
{ 724, 16, NETWK_TYPE_DCS_1800, "Brasil Telecom", "Brasil Telecom" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif

/* TIM Celular S.A. */           
{ 724, 21, NETWK_TYPE_DCS_1800, "TIM", "TIM" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청

/* Telemig Celular S/A */
{ 724, 23, NETWK_TYPE_GSM_900, "VIVO", "VIVO" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Telemig Celular S/A */
{ 724, 23, NETWK_TYPE_DCS_1800, "VIVO", "VIVO" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청

/* Amazonia Celular S/A */
{ 724, 24, NETWK_TYPE_GSM_900, "Amaz" "\xF4\x6E" "ia Cel", "Amaz" "\xF4\x6E" "ia Cel" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Amazonia Celular S/A */
{ 724, 24, NETWK_TYPE_DCS_1800, "Amaz" "\xF4\x6E" "ia Cel", "Amaz" "\xF4\x6E" "ia Cel" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청 

/* TNL PCS S.A. */
{ 724, 31, NETWK_TYPE_DCS_1800, "Oi", "Oi" }, 

/* Triangulo Celular S.A */
{ 724, 32, NETWK_TYPE_GSM_900, "CTBC", "CTBC" }, 
/* Triangulo Celular S.A */
{ 724, 32, NETWK_TYPE_DCS_1800, "CTBC", "CTBC" }, 
/* Triangulo Celular S.A */
{ 724, 33, NETWK_TYPE_GSM_900, "CTBC", "CTBC" }, 
/* Triangulo Celular S.A */
{ 724, 33, NETWK_TYPE_DCS_1800, "CTBC", "CTBC" }, 
/* Triangulo Celular S.A */
{ 724, 34, NETWK_TYPE_GSM_900, "CTBC", "CTBC" }, 
/* Triangulo Celular S.A */
{ 724, 34, NETWK_TYPE_DCS_1800, "CTBC", "CTBC" }, 

{ 724, 37, NETWK_TYPE_DCS_1800, "AEIOU", "AEIOU" }, 

/* TIM Celular S.A. */
{ 724, 40, NETWK_TYPE_DCS_1800, "TIM", "TIM" },        
/* Telet S.A. */
{ 724, 51, NETWK_TYPE_DCS_1800, "Claro", "Claro" }, 
#if defined(FEATURE_SAMSUNG_ARCTI)
/* 14 Brasil Telecom Celular S.A */
{ 724, 160, NETWK_TYPE_DCS_1800, "Telecom", "Brasil Telecom" },
#else
{ 724, 160, NETWK_TYPE_DCS_1800, "Brasil Telecom", "Brasil Telecom" },
#endif
/* TNL PCS S.A. */
{ 724, 310, NETWK_TYPE_DCS_1800, "Oi", "Oi" }, 

/***************
 **** Chile ****
 ***************/
/* Entel Telefonia Movil S.A */
{ 730,   1, NETWK_TYPE_PCS_1900, "ENTEL PCS", "ENTEL PCS" },
#if defined(FEATURE_SAMSUNG_ARCTI)
/* Telefonica  Movil de Chile */
{ 730,   2, NETWK_TYPE_PCS_1900, "Telefonica", "Telefonica" },
#else
{ 730,   2, NETWK_TYPE_PCS_1900, "movistar", "movistar" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif
#if defined(FEATURE_SAMSUNG_MXTELCEL)
/* Claro CHL */
{ 730, 3, NETWK_TYPE_PCS_1900, "ClaroCHL", "Claro CHL" }, 
#else
{ 730, 3, NETWK_TYPE_PCS_1900, "Claro", "Claro" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif

/* Entel PCS Telecomunicaciones S.A */
{ 730,  10, NETWK_TYPE_PCS_1900, "ENTEL PCS", "ENTEL PCS" },
{ 730,   11, NETWK_TYPE_PCS_1900, "ENTEL PCS", "ENTEL PCS" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#if  defined (FEATURE_SAMSUNG_ARCTI)   
/* Telefonica  Movil de Chile */
{ 730,   23, NETWK_TYPE_PCS_1900, "Telefonica", "Telefonica" },
#else
{ 730,   23, NETWK_TYPE_PCS_1900, "movistar", "movistar" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif
{ 730,   101, NETWK_TYPE_PCS_1900, "ENTEL PCS", "ENTEL PCS" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청

/*****************
 **** Colombia ****
 *****************/
#if defined(FEATURE_SAMSUNG_ARCTI)
{ 732,  101, NETWK_TYPE_GSM_850, "Comcel", "Comcel" },
{ 732,  101, NETWK_TYPE_PCS_1900, "Comcel", "Comcel" },
#elif defined(FEATURE_SAMSUNG_MXTELCEL)
/* Occidente y Caribe Celular SA Occel SA */
{ 732,  101, NETWK_TYPE_GSM_850, "COMCEL", "Comcel 3GSM" }, 
/* Comunicacion Celular SA Comcel SA */
{ 732,  101, NETWK_TYPE_PCS_1900, "COMCEL", "Comcel 3GSM" },
#else
/* Occidente y Caribe Celular SA Occel SA */
{ 732,  101, NETWK_TYPE_GSM_850, "COMCEL 3GSM", "COMCEL 3GSM" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Comunicacion Celular SA Comcel SA */
{ 732,  101, NETWK_TYPE_PCS_1900, "COMCEL 3GSM", "COMCEL 3GSM" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif

/* Colombia Movil SA */
{ 732,   103, NETWK_TYPE_PCS_1900, "TIGO", "TIGO" },// 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Colombia Movil SA */
{ 732,  111, NETWK_TYPE_PCS_1900, "TIGO", "TIGO" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Colombia Telecomunicaciones S.A. ESP */
{ 732,  111, NETWK_TYPE_GSM_900, "TIGO", "TIGO" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Telefonica Moviles Colombia S.A. */
{ 732,  123, NETWK_TYPE_GSM_850, "COL Movistar", "COL Movistar" }, // 2010.09.13, O2 lifecard TC 4.1.1.3
/* Telefonica Moviles Colombia S.A. */
{ 732,  123, NETWK_TYPE_PCS_1900, "COL Movistar", "COL Movistar" }, // 2010.09.13, O2 lifecard TC 4.1.1.3

/*******************
   **** Venezuela ****
 *******************/
#if defined(FEATURE_SAMSUNG_ARCTI)   
/* Infonet, Redes De Informacion C.A */
{ 734,   1, NETWK_TYPE_GSM_900, "Infonet", "Infonet" },
#else
{ 734,   1, NETWK_TYPE_GSM_900, "Digitel", "Digitel" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif
/* Corporacion Digitel C.A */
{ 734,   2, NETWK_TYPE_GSM_900, "Digitel", "Digitel" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Corporacion Digitel C.A */
{ 734,   3, NETWK_TYPE_GSM_900, "Digitel", "Digitel" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* movistar */
{ 734,   4, NETWK_TYPE_GSM_850, "movistar", "movistar" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
{ 734,   6, NETWK_TYPE_GSM_850, "Movilnet", "Movilnet" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
{ 734,   21, NETWK_TYPE_GSM_900, "Digitel", "Digitel" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청

/*****************
 **** Bolivia ****
 *****************/
/* Nuevatel PCS De Bolivia SA */
{ 736,   1, NETWK_TYPE_PCS_1900, "NUEVATEL", "NUEVATEL" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
#if defined(FEATURE_SAMSUNG_ARCTI)
/* Entel SA */
{ 736,   2, NETWK_TYPE_PCS_1900, "Entel", "Entel" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
#else
/* Entel SA */
{ 736,   2, NETWK_TYPE_PCS_1900, "MOVIL-E", "MOVIL-E" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
#endif
/* Telefonica Celular De Bolivia S.A. */
{ 736, 3, NETWK_TYPE_GSM_850, "TIGO", "TIGO" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청

/****************
 **** Guyana ****
 ****************/
/* Cel*Star Guyana Inc */
{ 738, 1, NETWK_TYPE_GSM_900, "Cel*Star", "Cel*Star" }, // 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Guyana Telephone & Telegraph Co. */
{ 738, 2, NETWK_TYPE_GSM_900, "CLNK PLS", "CLNK PLS" }, 

/**************
 **** ECU  ****
 **************/
/* OTECEL S.A. */
{ 740, 0, NETWK_TYPE_GSM_850, "movistar", "movistar" }, 
#if defined (FEATURE_SAMSUNG_ARCTI)  
{ 740, 1, NETWK_TYPE_GSM_850, "Conecel", "Conecel" }, 
#elif defined (FEATURE_SAMSUNG_COMCEL)
/* Porta GSM */
{ 740, 1, NETWK_TYPE_PCS_1900, "PORTA GSM", "PORTA GSM" }, 
#elif defined (FEATURE_SAMSUNG_MXTELCEL) 
/* Porta GSM */
{ 740, 1, NETWK_TYPE_PCS_1900, "PORTAGSM", "PORTA GSM" }, 
#else
/* Porta GSM */
{ 740, 1, NETWK_TYPE_GSM_850, "Porta GSM", "Porta GSM" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif
/* Latin Open */
{ 740, 2, NETWK_TYPE_GSM_850, "ALEGRO", "ALEGRO" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/******************
 **** Paraguay ****
 ******************/
/* Hola Paraguay S.A. */
{ 744, 1, NETWK_TYPE_PCS_1900, "HPGYSA", "HPGYSA" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#if defined(FEATURE_SAMSUNG_MXTELCEL)
/* CTI  Movil PRY */
{ 744, 2, NETWK_TYPE_PCS_1900, "CLARO PY", "CLARO PY" },
#elif defined(FEATURE_SAMSUNG_COMCEL)	
{ 744, 2, NETWK_TYPE_PCS_1900, "CTI Movil", "CTI Movil" }, 
#else
/* AMX Paraguay Sociedad Anonima */
{ 744, 2, NETWK_TYPE_PCS_1900, "Claro PY", "Claro PY" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif
/* Telefonica Celular Del Paraguay S.A. (Telecel S.A.) */
{ 744, 4, NETWK_TYPE_GSM_850, "TIGO", "TIGO" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Nucleo S.A */
{ 744, 5, NETWK_TYPE_GSM_850, "Personal", "Personal" }, 
/* Nucleo S.A */
{ 744, 5, NETWK_TYPE_PCS_1900, "Personal", "Personal" }, 

/*****************
 **** Surinam ****
 *****************/
/* NV ICMS */
{ 746,   1, NETWK_TYPE_GSM_900, "ICMS", "ICMS" },
/* Telesur */
{ 746,   2, NETWK_TYPE_GSM_900, "Telesur", "Telesur" },// 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* NV ICMS */
{ 746,   2, NETWK_TYPE_DCS_1800, "Telesur", "Telesur" },// 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/* Digitel */
{ 746,   3, NETWK_TYPE_DCS_1800, "Digicel", "Digicel" },// 2009.09.09 중남미 개발 그룹 안병우 선임 요청
/*****************
 **** Uruguay ****
 *****************/
/* ANCEL */
{ 748, 1, NETWK_TYPE_GSM_900, "ANTEL", "ANTEL" },  // 2011.07.22 중남미 개발 그룹 안병우 선임 요청
/* ANCEL */
{ 748, 1, NETWK_TYPE_DCS_1800, "ANTEL", "ANTEL" },  // 2011.07.22 중남미 개발 그룹 안병우 선임 요청
/* ANCEL */
{ 748, 1, NETWK_TYPE_UMTS, "ANTEL", "ANTEL" },   // 2011.07.22 중남미 개발 그룹 안병우 선임 요청
/* Abiatar S.A. */
{ 748, 7, NETWK_TYPE_GSM_850, "movistar", "movistar" },   // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
/* Abiatar S.A. */
{ 748, 7, NETWK_TYPE_PCS_1900, "movistar", "movistar" }, // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#if defined(FEATURE_SAMSUNG_MXTELCEL) 
/* CTI Movil URY */
{ 748, 10, NETWK_TYPE_PCS_1900, "CLARO UY", "CLARO URUGUAY" }, 
#elif defined (FEATURE_SAMSUNG_COMCEL)  
{ 748, 10, NETWK_TYPE_PCS_1900, "CTI Movil", "CTI Movil" }, 
#else
/* AM Wireless Uruguay S.A. */
{ 748, 10, NETWK_TYPE_PCS_1900, "Claro UY", "Claro UY" },  // 2009.09.01 중남미 개발 그룹 안병우 선임 요청
#endif

/*******************
 **** Satellite ****
 *******************/
/* Thuraya Satellite Telecommunications Co */
{ 901, 5, NETWK_TYPE_GSM_SAT, "Thuraya", "Thuraya" }, 
/* Maritime Communications Partner AS */
{ 901, 12, NETWK_TYPE_DCS_1800, "MCP", "MCP" }, 
/*Cingular Wireless and MTN*/
{ 901, 18, NETWK_TYPE_DCS_1800, "cellular at SEA", "cellular at SEA" },
#endif /* FEATURE_SAMSUNG_SOUTHEAST_ASIA */
};  /* qmnetwk_network_table */



/* PLMN Country Table */
static const PLMN_COUNTRY_t Plmn_Country_List[] = 
{
	/*000*/ {PLMN_MCC_UNKNOWN, PLMN_CC_UNKNOWN, PLMN_UNKNOWN_COUNTRY},
	/*001*/ {0x232, 0x43, "A"},
	/*002*/ {0x276, 0x355, "AL"},
	/*003*/ {0x603, 0x213, "ALG"},
	/*004*/ {0x344, 0x268, "AN"},
	/*005*/ {0x213, 0x376, "AND"},
	/*006*/ {0x631, 0x244, "AGO"},
	/*007*/ {0x362, 0x599, "ANT"},
	/*008*/ {0x283, 0x374, "RA"},
	/*009*/ {0x544, 0x684, "AS"},
	/*010*/ {0x505, 0x61, "AUS"},
	/*011*/ {0x400, 0x994, "AZE"},
	/*012*/ {0x206, 0x32, "B"},
	/*013*/ {0x350, 0x809, "BMU"},
	/*014*/ {0x613, 0x226, "BF"},
	/*015*/ {0x284, 0x359, "BG"},
	/*016*/ {0x470, 0x880, "BGD"},
	/*017*/ {0x426, 0x973, "BHR"},
	/*018*/ {0x218, 0x387, "BA"},
	/*019*/ {0x616, 0x229, "BEN"},
	/*020*/ {0x736, 0x591, "BOL"},
	/*021*/ {0x724, 0x55, "BRA"},
	/*022*/ {0x528, 0x673, "BRU"},
	/*023*/ {0x642, 0x257, "BUR"},
	/*024*/ {0x652, 0x267, "BW"},
	/*025*/ {0x257, 0x375, "BY"},
	/*026*/ {0x368, 0x53, "CU"},
	/*027*/ {0x624, 0x237, "CAM"},
	/*028*/ {0x302, 0x1, "CAN"},
	/*029*/ {0x228, 0x41, "CH"},
	/*030*/ {0x622, 0x235, "TD"},
	/*031*/ {0x730, 0x56, "CHI"},
	/*032*/ {0x460, 0x86, "CHN"},
	/*033*/ {0x612, 0x225, "CI"},
	/*034*/ {0x629, 0x242, "RC"},
	/*035*/ {0x625, 0x238, "CPV"},
	/*036*/ {0x280, 0x357, "CY"},
	/*037*/ {0x230, 0x420, "CZ"},
	/*038*/ {0x262, 0x49, "D"},
	/*039*/ {0x238, 0x45, "DK"},
	/*040*/ {0x370, 0x809, "DO"},
	/*041*/ {0x214, 0x34, "E"},
	/*042*/ {0x248, 0x372, "EE"},
	/*043*/ {0x602, 0x20, "EGY"},
	/*044*/ {0x706, 0x503, "ESV"},
	/*045*/ {0x636, 0x251, "ETH"},
	/*046*/ {0x208, 0x33, "F"},
	/*047*/ {0x244, 0x358, "FI"},
	/*048*/ {0x542, 0x679, "FIJ"},
	/*049*/ {0x288, 0x298, "FO"},
	/*050*/ {0x550, 0x691, "FSM"},
	/*051*/ {0x282, 0x995, "GEO"},
	/*052*/ {0x627, 0x240, "GQN"},
	/*053*/ {0x620, 0x233, "GH"},
	/*054*/ {0x266, 0x350, "GIB"},
	/*055*/ {0x340, 0x590, "F"},
	/*056*/ {0x611, 0x224, "GN"},
	/*057*/ {0x628, 0x241, "GA"},
	/*058*/ {0x202, 0x30, "GR"},
	/*059*/ {0x290, 0x299, "GL"},
	/*060*/ {0x535, 0x671, "GUM"},
	/*061*/ {0x216, 0x36, "H"},
	/*062*/ {0x454, 0x852, "HK"},
	/*063*/ {0x219, 0x385, "HR"},
	/*064*/ {0x222, 0x39, "I"},
	/*065*/ {0x425, 0x972, "IL"},
	/*066*/ {0x404, 0x91, "INA"},
	/*067*/ {0x510, 0x62, "IND"},
	/*068*/ {0x432, 0x98, "IR"},
	/*069*/ {0x272, 0x353, "IRL"},
	/*070*/ {0x274, 0x354, "IS"},
	/*071*/ {0x338, 0x876, "JM"},
	/*072*/ {0x416, 0x962, "JOR"},
	/*073*/ {0x639, 0x254, "KE"},
	/*074*/ {0x437, 0x996, "KGZ"},
	/*075*/ {0x456, 0x855, "KHM"},
	/*076*/ {0x420, 0x966, "KSA"},
	/*077*/ {0x419, 0x965, "KT"},
	/*078*/ {0x401, 0x7, "KZ"},
	/*079*/ {0x270, 0x352, "L"},
	/*080*/ {0x457, 0x856, "LAO"},
	/*081*/ {0x606, 0x218, "LAR"},
	/*082*/ {0x618, 0x231, "LBR"},
	/*083*/ {0x295, 0x41, "LI"},
	/*084*/ {0x651, 0x266, "LSO"},
	/*085*/ {0x246, 0x370, "LT"},
	/*086*/ {0x247, 0x371, "LV"},
	/*087*/ {0x278, 0x356, "MT"},
	/*088*/ {0x455, 0x853, "MAC"},
	/*089*/ {0x212, 0x377, "MC"},
	/*090*/ {0x259, 0x373, "MD"},
	/*091*/ {0x646, 0x261, "MG"},
	/*092*/ {0x334, 0x52, "MX"},
	/*093*/ {0x428, 0x976, "MN"},
	/*094*/ {0x294, 0x389, "MKD"},
	/*095*/ {0x604, 0x212, "MOR"},
	/*096*/ {0x643, 0x258, "MOZ"},
	/*097*/ {0x617, 0x230, "MRU"},
	/*098*/ {0x472, 0x90, "MV"},
	/*099*/ {0x650, 0x265, "MW"},
	/*100*/ {0x502, 0x60, "MY"},
	/*101*/ {0x414, 0x95, "MM"},
	/*102*/ {0x242, 0x47, "N"},
	/*103*/ {0x649, 0x264, "NA"},
	/*104*/ {0x546, 0x687, "NCL"},
	/*105*/ {0x429, 0x977, "NEP"},
	/*106*/ {0x204, 0x31, "NL"},
	/*107*/ {0x530, 0x64, "NZ"},
	/*108*/ {0x422, 0x968, "OMN"},
	/*109*/ {0x268, 0x351, "P"},
	/*110*/ {0x716, 0x51, "PE"},
	/*111*/ {0x744, 0x595, "PGY"},
	/*112*/ {0x515, 0x63, "PH"},
	/*113*/ {0x410, 0x92, "PK"},
	/*114*/ {0x260, 0x48, "PL"},
	/*115*/ {0x537, 0x675, "PNG"},	/* Papua New Guinea */
	/*116*/ {0x547, 0x689, "F"},
	/*117*/ {0x427, 0x974, "QAT"},
	/*118*/ {0x635, 0x250, "R"},
	/*119*/ {0x722, 0x54, "AR"},
	/*120*/ {0x623, 0x236, "RCA"},
	/*121*/ {0x630, 0x242, "CD"},
	/*122*/ {0x647, 0x262, "F"},
	/*123*/ {0x609, 0x222, "MR"},
	/*124*/ {0x415, 0x961, "RL"},
	/*125*/ {0x610, 0x223, "ML"},
	/*126*/ {0x614, 0x227, "NE"},
	/*127*/ {0x226, 0x40, "RO"},
	/*128*/ {0x250, 0x7, "RUS"},
	/*129*/ {0x240, 0x46, "S"},
	/*130*/ {0x655, 0x27, "SA"},
	/*131*/ {0x634, 0x249, "SDN"},
	/*132*/ {0x633, 0x248, "SEZ"},
	/*133*/ {0x525, 0x65, "SGP"},
	/*134*/ {0x293, 0x386, "SI"},
	/*135*/ {0x231, 0x421, "SK"},
	/*136*/ {0x608, 0x221, "SN"},
	/*137*/ {0x637, 0x252, "SOM"},
	/*138*/ {0x746, 0x597, "SR"},
	/*139*/ {0x413, 0x94, "SRI"},
	/*140*/ {0x626, 0x239, "STP"},
	/*141*/ {0x417, 0x963, "SYR"},
	/*142*/ {0x653, 0x268, "SZ"},
	/*143*/ {0x615, 0x228, "TG"},
	/*144*/ {0x539, 0x676, "TON"},
	/*145*/ {0x520, 0x66, "TH"},
	/*146*/ {0x438, 0x993, "TMN"},
	/*147*/ {0x605, 0x216, "TN"},
	/*148*/ {0x286, 0x90, "TR"},
	/*149*/ {0x466, 0x886, "TW"},
	/*150*/ {0x640, 0x255, "TZ"},
	/*151*/ {0x424, 0x971, "UAE"},
	/*152*/ {0x641, 0x256, "UG"},
	/*153*/ {0x234, 0x44, "UK"},
	/*154*/ {0x255, 0x380, "UKR"},
	/*155*/ {0x310, 0x1, "US"},
	/*156*/ {0x434, 0x998, "UZB"},
	/*157*/ {0x541, 0x678, "VUT"},
	/*158*/ {0x452, 0x84, "VN"},
	/*159*/ {0x734, 0x58, "VZ"},
	/*160*/ {0x607, 0x0, "GMB"},
	/*161*/ {0x619, 0x232, "WAL"},
	/*162*/ {0x621, 0x234, "NG"},
	/*163*/ {0x421, 0x967, "YE"},
	/*164*/ {0x220, 0x381, "YU"},
	/*165*/ {0x645, 0x260, "ZM"},
	/*166*/ {0x648, 0x263, "ZW"},
	/*167*/ {0x363, 0x297, "ABW"},		/* Aruba */
	/*168*/ {0x412, 0x93, "AF"},		/* Afghanistine */
	/*169*/ {0x712, 0x506, "CRI"},		/* Costa Rica */
	/*170*/ {0x423, 0x970, "PAL"},		/* Palestine */
	/*171*/ {0x436, 0x7, "TJK"},		/* Tajikistan */
	/*172*/ {0x374, 0x868, "TTO"},		/* Trinidad and Tobago */
	/*173*/ {0x311, 0x1, "US"},			/* United States */
	/*174*/ {0x332, 0x1, "VI"},			/* United States Virgin Islands */
	/*175*/ {0x702, 0x591,"BL"},		/* Bolivia */ 
	/*176*/ {0x402, 0x975, "BTN"},		/* Bhutan */
	/*177*/ {0x710, 0x17, "NIC"},
	/*178*/ {0x364, 0x364, "BAH"},
	/*179*/ {0x342, 0x342, "BRB"},
	/*180*/ {0x346, 0x346, "CYM"},
	/*181*/ {0x740, 0x740, "ECU"},
	/*182*/ {0x714, 0x714, "PA"},
	/*183*/ {0x352, 0x352, "GRN"},
	/*184*/ {0x545, 0x545, "KI"},
	/*185*/ {0x638, 0x638, "DJ"},
	/*186*/ {0x654, 0x654, "KM"},
	/*187*/ {0x514, 0x514, "TLS"},
	/*188*/ {0x732, 0x732, "COL"},
	/*189*/ {0x418, 0x418, "IRQ"},
	/*190*/ {0x365, 0x365, "AI"},
	/*191*/ {0x366, 0x366, "DM"},
	/*192*/ {0x708, 0x708, "HND"},
	/*193*/ {0x356, 0x356, "KN"},
	/*194*/ {0x358, 0x358, "SLU"},
	/*195*/ {0x376, 0x376, "TCA"},
	/*196*/ {0x360, 0x360, "VC"},
	/*197*/ {0x704, 0x704, "GTM"},
	/*198*/ {0x548, 0x548, "COK"},
	/*199*/ {0x738, 0x738, "GUY"},
	/*200*/ {0x348, 0x348, "VGB"},
	/*201*/ {0x405, 0x405, "INA"},
	/*202*/ {0x467, 0x467, "PRK"},
	/*203*/ {0x354, 0x354, "MS"},
	/*204*/ {0x748, 0x748, "URY"},
	/*205*/ {0x901, 0x901, "NOR"},
	/*206*/ {0x795, 0x795, "TKM"},
	/*207*/ {0x308, 0x308, "SPM"},
	/*208*/ {0x440, 0x440, "JP"},
	/*209*/ {0x450, 0x450, "KR"},
	/*210*/ {0x632, 0x632, "GNB"},
	/*211*/ {0x540, 0x540, "SLB"},
	/*212*/ {0x372, 0x372, "HTI"},
    /*213*/ {0x552, 0x552, "PLW"},
	/*214*/ {0x549, 0x549, "WSM"},
	/*215*/ {0x750, 0x750, "FLK"},
	/*216*/ {0x330, 0x330, "PR"},
	/*217*/ {0x292, 0x292, "SMT"},
	/*218*/ {0x1,	0x1,   "-"} // Test network
};

/* Unicode-encoded PLMN name table (data is encoded in UCS2 
 * format, which is defined in Annex B of GSM 11.11. The first byte indicates the UCS2
 * encoding format. 
 */

// "China Mobile" network: Long Name and Short Name are the same.
static const UInt8 China_Mobile_Ucs2[] =	{0x80, 0x4E, 0x2D, 0x56, 0xFD, 0x79, 0xFB, 0x52, 0xA8};

// "China Unicom" network: Long Name and Short Name are the same.
static const UInt8 China_Unicom_Ucs2[] =	{0x80, 0x4E, 0x2D, 0x56, 0xFD, 0x80, 0x54, 0x90, 0x1A};

// "HK NEW WORLD" network: Long Name and Short Name are different. 
static const UInt8 HK_NewWorld_Ucs2_Long[] =	{0x80, 0x65, 0xB0, 0x4E, 0x16, 0x75, 0x4C, 0x50, 0xB3, 0x52, 0xD5, 0x7D, 0xB2};
static const UInt8 HK_NewWorld_Ucs2_Short[] =	{0x80, 0x65, 0xB0, 0x4E, 0x16, 0x75, 0x4C};

// "HK Peoples" network: Long Name and Short Name are the same.
static const UInt8 HK_PEOPLES_Ucs2[] =	{0x80, 0x83, 0xEF, 0x6F, 0x64, 0x84, 0x2C, 0x77, 0x3E};

// "MAC-CTMGSM" network: Long Name and Short Name are the same.
static const UInt8 Macau_Ctm_Ucs2[] =		{0x80, 0x6F, 0xB3, 0x95, 0x80, 0x96, 0xFB, 0x8A, 0x0A};

// "MAC-Hutchison" network: Long Name and Short Name are the same. 
static const UInt8 Macau_Hutchison_Ucs2[] =	{0x80, 0x6F, 0xB3, 0x95, 0x80, 0x54, 0x8C, 0x8A, 0x18, 0x96, 0xFB, 0x8A, 0x0A};

// "Far EasTone" network: Long Name and Short Name are the same. 
static const UInt8 TW_Fet_Ucs2[] =			{0x80, 0x90, 0x60, 0x50, 0xB3, 0x96, 0xFB, 0x4F, 0xE1};

// "TWN Tuntex GSM 1800" network: Long Name and Short Name are the same.
static const UInt8 TW_Tuntex_Ucs2[] =		{0x80, 0x67, 0x71, 0x69, 0xAE, 0x96, 0xFB, 0x4F, 0xE1};

// "KGT-Online" network: Long Name and Short Name are the same.
static const UInt8 TW_Kgt_Ucs2[] =			{0x80, 0x54, 0x8C, 0x4F, 0xE1, 0x96, 0xFB, 0x8A, 0x0A};

// "Chunghwa" network: Long Name and Short Name are the same.
static const UInt8 TW_ChungHwa_Ucs2[] =		{0x80, 0x4E, 0x2D, 0x83, 0xEF, 0x96, 0xFB, 0x4F, 0xE1};

// "MOBITAI" network: Long Name and Short Name are the same.
static const UInt8 TW_Mobitai_Ucs2[] =		{0x80, 0x67, 0x71, 0x4F, 0xE1, 0x96, 0xFB, 0x8A, 0x0A};

// "Pacific Cellular Co." network: Long Name and Short Name are the same.
static const UInt8 TW_Pacific_Ucs2[] =		{0x80, 0x53, 0xF0, 0x70, 0x63, 0x59, 0x27, 0x54, 0xE5, 0x59, 0x27};

// "TransAsia" network: Long Name and Short Name are the same.
static const UInt8 TW_TransAsia_Ucs2[] =	{0x80, 0x6C, 0xDB, 0x4E, 0x9E, 0x96, 0xFB, 0x4F, 0xE1};

//"Vibo" netowrk: Long Name and Short Name are the same.
static const UInt8 TW_Vibo_Ucs2[] =	{0x80, 0x5A, 0x01, 0x5B, 0xF6, 0x96, 0xFB, 0x4F, 0xE1};

static const UCS2_PLMN_ITEM_t Plmn_Ucs2_List[] = 
{
{0x0,	China_Mobile_Ucs2,		China_Mobile_Ucs2,		sizeof(China_Mobile_Ucs2),	sizeof(China_Mobile_Ucs2),	32},
{0x2,	China_Mobile_Ucs2,		China_Mobile_Ucs2,		sizeof(China_Mobile_Ucs2),	sizeof(China_Mobile_Ucs2),	32},
{0x7,	China_Mobile_Ucs2,		China_Mobile_Ucs2,		sizeof(China_Mobile_Ucs2),	sizeof(China_Mobile_Ucs2),	32},
{0x1,	China_Unicom_Ucs2,		China_Unicom_Ucs2,		sizeof(China_Unicom_Ucs2),	sizeof(China_Unicom_Ucs2),	32},   
{0x10,	HK_NewWorld_Ucs2_Long,	HK_NewWorld_Ucs2_Short,	sizeof(HK_NewWorld_Ucs2_Long), sizeof(HK_NewWorld_Ucs2_Short), 62},
{0x12,	HK_PEOPLES_Ucs2,		HK_PEOPLES_Ucs2,		sizeof(HK_PEOPLES_Ucs2),	sizeof(HK_PEOPLES_Ucs2), 62},
{0x1,	Macau_Ctm_Ucs2,			Macau_Ctm_Ucs2,			sizeof(Macau_Ctm_Ucs2),		sizeof(Macau_Ctm_Ucs2),		88},
{0x3,	Macau_Hutchison_Ucs2,	Macau_Hutchison_Ucs2,	sizeof(Macau_Hutchison_Ucs2), sizeof(Macau_Hutchison_Ucs2), 88},
{0x1,	TW_Fet_Ucs2,			TW_Fet_Ucs2,			sizeof(TW_Fet_Ucs2),		sizeof(TW_Fet_Ucs2),		149},
{0x6,	TW_Tuntex_Ucs2,			TW_Tuntex_Ucs2,			sizeof(TW_Tuntex_Ucs2),		sizeof(TW_Tuntex_Ucs2),		149},
{0x88,	TW_Kgt_Ucs2,			TW_Kgt_Ucs2,			sizeof(TW_Kgt_Ucs2),		sizeof(TW_Kgt_Ucs2),		149},
{0x92,	TW_ChungHwa_Ucs2,		TW_ChungHwa_Ucs2,		sizeof(TW_ChungHwa_Ucs2),	sizeof(TW_ChungHwa_Ucs2),	149},
{0x93,	TW_Mobitai_Ucs2,		TW_Mobitai_Ucs2,		sizeof(TW_Mobitai_Ucs2),	sizeof(TW_Mobitai_Ucs2),	149},
{0x97,	TW_Pacific_Ucs2,		TW_Pacific_Ucs2,		sizeof(TW_Pacific_Ucs2),	sizeof(TW_Pacific_Ucs2),	149},
{0x99,	TW_TransAsia_Ucs2,		TW_TransAsia_Ucs2,		sizeof(TW_TransAsia_Ucs2),	sizeof(TW_TransAsia_Ucs2),	149},
{0x89,	TW_Vibo_Ucs2,			TW_Vibo_Ucs2,			sizeof(TW_Vibo_Ucs2),		sizeof(TW_Vibo_Ucs2),	149}
};

#define NUMBER_OF_UCS2_PLMN (sizeof(Plmn_Ucs2_List) / sizeof(UCS2_PLMN_ITEM_t))


typedef struct
{
	UInt16 mcc;
	UInt8 mnc;
} PLMN_VALUE_t;

typedef struct
{
	const PLMN_VALUE_t *plmn_list;
	UInt8 plmn_size;
} HPLMN_TABLE_t;

/* Table for HPLMN match. 
 * 
 * For example, China Mobile has 460/00, 460/02 and 460/07 PLMN values. As long 
 * as the registered PLMN and IMSI HPLMN belong to one of these three values and 
 * not necessarily the same (e.g. network PLMN is 460/00 and IMSI HPLMN is 460/02), the 
 * phone shall be considered registred in Home network and Roaming indicator must not be 
 * displayed to the user per China Mobile requirements.
 */
static const PLMN_VALUE_t German_O2_Plmn[] = { {0x62F2, 0x70}, {0x62F2, 0x80} };

static const PLMN_VALUE_t China_Mobile_Plmn[] = { {0x64F0, 0x00}, {0x64F0, 0x20}, {0x64F0, 0x70} };

static const HPLMN_TABLE_t Hplmn_Table[] = 
{
	{ German_O2_Plmn, sizeof(German_O2_Plmn) / sizeof(PLMN_VALUE_t) }, 
	{ China_Mobile_Plmn, sizeof(China_Mobile_Plmn) / sizeof(PLMN_VALUE_t) }
};

#define NUM_OF_HPLMN_LIST (sizeof(Hplmn_Table) / sizeof(HPLMN_TABLE_t))
#define NUMBER_OF_PLMN_ENTRY_SAMSUNG  (sizeof(Plmn_Table_Samsung) / sizeof(network_info_s_type))
#define MSAP_COUNTRY_LIST_LENGTH (sizeof(Plmn_Country_List) / sizeof(PLMN_COUNTRY_t))

/* END : Irine_SunnyVale */

