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
// AkDelayFXParams.cpp
//
// Delay FX parameters sample implementation
//
//////////////////////////////////////////////////////////////////////

#include <AK/Tools/Common/AkAssert.h>
#include <AK/Tools/Common/AkBankReadHelpers.h>
#include <math.h>
#include "AkDelayFXParams.h"


// Constructor.
CAkDelayFXParams::CAkDelayFXParams( )
{
}

// Destructor.
CAkDelayFXParams::~CAkDelayFXParams( )
{
}

// Copy constructor.
CAkDelayFXParams::CAkDelayFXParams( const CAkDelayFXParams & in_rCopy )
{
	RTPC = in_rCopy.RTPC;
	RTPC.bHasChanged = true;
	NonRTPC = in_rCopy.NonRTPC;
	NonRTPC.bHasChanged = true;
}

// Create parameter node duplicate.
AK::IAkPluginParam * CAkDelayFXParams::Clone( AK::IAkPluginMemAlloc * in_pAllocator )
{
    return AK_PLUGIN_NEW( in_pAllocator, CAkDelayFXParams( *this ) );
}

// Parameter node initialization.
AKRESULT CAkDelayFXParams::Init(	
								AK::IAkPluginMemAlloc *	in_pAllocator,									   
								const void *			in_pParamsBlock, 
								AkUInt32				in_ulBlockSize  )
{
    if ( in_ulBlockSize == 0)
    {
		// Init default parameters.
        memset(RTPC.iCC, 0, RACK_CC_COUNT * sizeof(AkInt16));
		RTPC.bHasChanged = true;
        NonRTPC.iPort = 0;
		NonRTPC.bHasChanged = true;
		return AK_Success;
    }
    return SetParamsBlock( in_pParamsBlock, in_ulBlockSize );
}

// Parameter interface termination.
AKRESULT CAkDelayFXParams::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
    AK_PLUGIN_DELETE( in_pAllocator, this );
    return AK_Success;
}

// Parameter block set.
AKRESULT CAkDelayFXParams::SetParamsBlock( 
	const void * in_pParamsBlock, 
	AkUInt32 in_ulBlockSize )
{
	AKRESULT eResult = AK_Success;
	AkUInt8 * pParamsBlock = (AkUInt8 *)in_pParamsBlock;
	NonRTPC.iPort = READBANKDATA( AkInt16, pParamsBlock, in_ulBlockSize );
    for (int i = 0; i < RACK_CC_COUNT; i++)
    {
        RTPC.iCC[i] = READBANKDATA( AkInt16, pParamsBlock, in_ulBlockSize );
    }
	CHECKBANKDATASIZE( in_ulBlockSize, eResult );

	RTPC.bHasChanged = true;
	NonRTPC.bHasChanged = true;

    return eResult;
}

// Update a single parameter.
AKRESULT CAkDelayFXParams::SetParam(	
									AkPluginParamID in_ParamID,
									const void * in_pValue, 
									AkUInt32 in_ulParamSize )
{
	AKRESULT eResult = AK_Success;

    if (in_ParamID == AK_DELAYFXPARAM_PORT_ID)
    {
		NonRTPC.iPort = *(AkInt16*)(in_pValue);
		NonRTPC.bHasChanged = true;
    }
    else if (in_ParamID >= AK_DELAYFXPARAM_CC0_ID && in_ParamID < AK_DELAYFXPARAM_CC0_ID + RACK_CC_COUNT)
    {
		AkInt16 iValue = *(AkInt16*)(in_pValue);
		iValue = AkClamp( iValue, 0, 127 );
        RTPC.iCC[in_ParamID - AK_DELAYFXPARAM_CC0_ID] = iValue;
		RTPC.bHasChanged = true;
    }
    else
    {
		AKASSERT(!"Invalid parameter.");
		eResult = AK_InvalidParameter;
    }

	return eResult;
}
