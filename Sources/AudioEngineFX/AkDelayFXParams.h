/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Version: v2017.2.3  Build: 6575
  Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/
//////////////////////////////////////////////////////////////////////
//
// AkDelayFXParams.h
//
// Shared parameter implementation for Delay FX.
//
//////////////////////////////////////////////////////////////////////

#ifndef _AK_DELAYFXPARAMS_H_
#define _AK_DELAYFXPARAMS_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>

// Parameters IDs for the Wwise or RTPC.
// Those IDs map to the AudioEnginePropertyID attributes in the XML property definition
static const AkPluginParamID AK_DELAYFXPARAM_PORT_ID                = 0;
static const AkPluginParamID AK_DELAYFXPARAM_CC0_ID			        = 1;	// RTPC
static const int RACK_CC_COUNT = 16;
static const AkPluginParamID AK_DELAYFXPARAM_NUM					= 17;

// Default parameter values
// #define DELAYFXPARAM_DELAYTIME_DEF			(0.5f)		// secs
// #define DELAYFXPARAM_FEEDBACK_DEF			(0.f)		// Percent
// #define DELAYFXPARAM_WETDRYMIX_DEF			(50.f)		// Percent
// #define DELAYFXPARAM_OUTPUTLEVEL_DEF		(0.f)		// dBFS
// #define DELAYFXPARAM_FEEDBACKENABLED_DEF	(false)		// Disabled
// #define DELAYFXPARAM_PROCESSLFE_DEF			(true)

static const AkReal32 ONEOVER_DELAYFXPARAM_PERCENT_MAX = 0.01f;

struct AkDelayRTPCParams
{
    AkInt16     iCC[16];

    bool        bHasChanged;
};

struct AkDelayNonRTPCParams
{
    AkInt16     iPort;

    bool        bHasChanged;
};

struct AkDelayFXParams
{
	AkDelayRTPCParams RTPC;
	AkDelayNonRTPCParams NonRTPC;
} AK_ALIGN_DMA;


class CAkDelayFXParams 
	: public AK::IAkPluginParam
	, public AkDelayFXParams
{
public:

    /// Constructor.
    CAkDelayFXParams( );

	/// Copy constructor.
	CAkDelayFXParams( const CAkDelayFXParams & in_rCopy );

	/// Destructor.
    ~CAkDelayFXParams();

    /// Create parameter node duplicate.
    IAkPluginParam * Clone( AK::IAkPluginMemAlloc * in_pAllocator );

    /// Parameter node initialization.
	AKRESULT Init(	
		AK::IAkPluginMemAlloc *	in_pAllocator,						    
		const void *			in_pParamsBlock, 
		AkUInt32				in_ulBlockSize 
		);

	/// Parameter node termination.
    AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );

    /// Parameter block set.
    AKRESULT SetParamsBlock(	
		const void *in_pParamsBlock, 
		AkUInt32	in_ulBlockSize
		);

    /// Update a single parameter.
	AKRESULT SetParam(	
		AkPluginParamID in_ParamID,
		const void *	in_pValue, 
		AkUInt32		in_ulParamSize
		);
};

#endif // _AK_DELAYFXPARAMS_H_
