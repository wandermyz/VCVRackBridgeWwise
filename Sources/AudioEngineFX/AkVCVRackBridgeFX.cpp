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
// AkDelayFX.cpp
//
// Sample delay FX implementation.
//
//////////////////////////////////////////////////////////////////////

#include "common/client.cpp"

#include "AkVCVRackBridgeFX.h"
#include <AK/Tools/Common/AkAssert.h>
#include <AK/AkWwiseSDKVersion.h>
#include <cassert>

/// Plugin mechanism. Instantiation method that must be registered to the plug-in manager.
AK::IAkPlugin* CreateAkVCVRackBridgeFX( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkVCVRackBridgeFX( ) );
}

/// Plugin mechanism. Instantiation method that must be registered to the plug-in manager.
AK::IAkPluginParam * CreateAkVCVRackBridgeFXParams(AK::IAkPluginMemAlloc * in_pAllocator)
{
	return AK_PLUGIN_NEW(in_pAllocator, CAkDelayFXParams());
}

AK_IMPLEMENT_PLUGIN_FACTORY(AkVCVRackBridgeFX, AkPluginTypeEffect, 65, 1)

/// Constructor.
CAkVCVRackBridgeFX::CAkVCVRackBridgeFX()
	: m_pParams( NULL )
	, m_pAllocator( NULL )
    , client (nullptr)
{
}

/// Destructor.
CAkVCVRackBridgeFX::~CAkVCVRackBridgeFX()
{
    if (client != nullptr)
    {
        delete client;
        client = nullptr;
    }
}

/// Initializes and allocate memory for the effect.
AKRESULT CAkVCVRackBridgeFX::Init(	AK::IAkPluginMemAlloc *			in_pAllocator,		/// Memory allocator interface.
							AK::IAkEffectPluginContext *	in_pFXCtx,			/// Sound engine plug-in execution context.
							AK::IAkPluginParam *			in_pParams,			/// Associated effect parameters node.
							AkAudioFormat &					in_rFormat			/// Input/output audio format.
							)
{
    if (client != nullptr)
    {
        delete client;
    }
    client = new BridgeClient();
    client->setSampleRate(in_rFormat.uSampleRate);
    client->setPort(0);

	m_pParams = (CAkDelayFXParams*)in_pParams;
	m_pAllocator = in_pAllocator;
 
	m_FXState.Setup( m_pParams, in_rFormat.uSampleRate );
	AKRESULT eResult = m_FXState.InitDelay( in_pAllocator, m_pParams, in_rFormat.channelConfig );
	m_FXState.ComputeTailLength( m_pParams->RTPC.bFeedbackEnabled, m_pParams->RTPC.fFeedback );
	m_pParams->NonRTPC.bHasChanged = false; 
	m_pParams->RTPC.bHasChanged = false;

	AK_PERF_RECORDING_RESET();

	return eResult;
}

/// Effect termination.
AKRESULT CAkVCVRackBridgeFX::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
    if (client != nullptr)
    {
        delete client;
        client = nullptr;
    }

	m_FXState.TermDelay( in_pAllocator );
	AK_PLUGIN_DELETE( in_pAllocator, this ); /// Effect must delete itself
	return AK_Success;
}

/// Actions to perform on FX reset (example on bypass)
AKRESULT CAkVCVRackBridgeFX::Reset( )
{
	m_FXState.ResetDelay();
	return AK_Success;
}

/// Effect info query.
AKRESULT CAkVCVRackBridgeFX::GetPluginInfo( AkPluginInfo & out_rPluginInfo )
{
	out_rPluginInfo.eType = AkPluginTypeEffect;
	out_rPluginInfo.bIsInPlace = false;
	out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
	return AK_Success;
}


void CAkVCVRackBridgeFX::Execute(
    AkAudioBuffer *							io_pInBuffer,		///< Input audio buffer data structure
    AkUInt32								in_uInOffset,	///< Offset position into input buffer data
    AkAudioBuffer *							io_pOutBuffer		///< Output audio buffer data structure
)
{
    assert( io_pInBuffer->NumChannels() == io_pOutBuffer->NumChannels() );
    const AkUInt32 uNumChannels = io_pInBuffer->NumChannels();
    AkUInt32 uFramesConsumed = 0; // Track how much data is consumed from input buffer
    AkUInt32 uFramesProduced = 0; // Track how much data is produced to output buffer

    // TODO
    float input[BRIDGE_INPUTS * 512];
    float output[BRIDGE_OUTPUTS * 512];

    // Interleave samples
    AkUInt16 validFrames = io_pInBuffer->uValidFrames;
    for (AkUInt32 i = 0; i < uNumChannels; i++)
    {
        AkReal32 * AK_RESTRICT pInBuf = (AkReal32 * AK_RESTRICT) io_pInBuffer->GetChannel(i) + in_uInOffset;

        for (int j = 0; j < validFrames; j++)
        {
            input[j * BRIDGE_INPUTS + i] = pInBuf[j];
        }
    }
    // for (int i = 0; i < validFrames; i++) {
    //     for (int c = 0; c < BRIDGE_INPUTS; c++) {
    //         input[BRIDGE_INPUTS*i + c] = inputs[c][i];
    //     }
    // }
    // Process audio
    client->processStream(input, output, validFrames);

    // Uninterleave samples
    for (AkUInt32 i = 0; i < uNumChannels; i++)
    {
        AkReal32 * AK_RESTRICT pfOutBuf = (AkReal32 * AK_RESTRICT) io_pOutBuffer->GetChannel(i) + io_pOutBuffer->uValidFrames;

        for (int j = 0; j < validFrames; j++)
        {
            pfOutBuf[j] = output[j * BRIDGE_OUTPUTS + i];
        }
    }
    // for (int i = 0; i < validFrames; i++) {
    //     // To prevent the DAW from pausing the processReplacing() calls, add a noise floor so the DAW thinks audio is still being processed.
    //     float r = (float) rand() / RAND_MAX;
    //     r = 1.f - 2.f * r;
    //     // Ableton Live's threshold is 1e-5 or -100dB
    //     r *= 1.5e-5f; // -96dB
    //     for (int c = 0; c < BRIDGE_OUTPUTS; c++) {
    //         outputs[c][i] = output[BRIDGE_OUTPUTS*i + c] + r;
    //     }
    // }


    // for ( AkUInt32 i = 0; i < uNumChannels; i++ )
    // {
    //     AkReal32 * AK_RESTRICT pInBuf = (AkReal32 * AK_RESTRICT) io_pInBuffer->GetChannel( i ) + in_uInOffset; 
    //     AkReal32 * AK_RESTRICT pfOutBuf = (AkReal32 * AK_RESTRICT) io_pOutBuffer->GetChannel( i ) + io_pOutBuffer->uValidFrames;

    //     uFramesConsumed = 0; // Reset for every channel
    //     uFramesProduced = 0; 

    //     while ( (uFramesConsumed < io_pInBuffer->uValidFrames) && (uFramesProduced < io_pOutBuffer->MaxFrames()) )
    //     {
    //         *pfOutBuf = *pInBuf;

    //         pInBuf++;
    //         uFramesConsumed++;

    //         pfOutBuf++;
    //         uFramesProduced++;
    //     }
    // }

    uFramesConsumed += validFrames;
    uFramesProduced += validFrames;

    // Update AkAudioBuffer structure to continue processing
    io_pInBuffer->uValidFrames -= uFramesConsumed;
    io_pOutBuffer->uValidFrames += uFramesProduced;
    if ( io_pInBuffer->eState == AK_NoMoreData && io_pInBuffer->uValidFrames == 0 )
        io_pOutBuffer->eState = AK_NoMoreData; // Input entirely consumed and nothing more to output, the effect is done
    else if ( io_pOutBuffer->uValidFrames == io_pOutBuffer->MaxFrames() )
        io_pOutBuffer->eState = AK_DataReady; // A complete audio buffer is ready
    else
        io_pOutBuffer->eState = AK_DataNeeded; // We need more data to continue processing
}    

AKRESULT CAkVCVRackBridgeFX::TimeSkip(
    AkUInt32 &io_uFrames	///< Number of frames the audio processing should advance.  The output value should be the number of frames that would be consumed to output the number of frames this parameter has at the input of the function.
)
{
    return AK_DataReady;
}

/// Effect plug-in DSP processing
void CAkVCVRackBridgeFX::Execute( AkAudioBuffer * io_pBuffer )
{
	for ( AkUInt32 i = 0; i < 2; ++i ) 
    {
        AkReal32 * AK_RESTRICT pfChan = (AkReal32 * AK_RESTRICT) io_pBuffer->GetChannel(i);
        for (int i = 0; i < io_pBuffer->uValidFrames; i++)
        {
            pfChan[i] = 0;
        }
    }
    return;

	if ( AK_EXPECT_FALSE( m_pParams->NonRTPC.bHasChanged ) ) 
	{
		AKRESULT eResult = m_FXState.InitDelay( m_pAllocator, m_pParams, io_pBuffer->GetChannelConfig() );
		if ( eResult != AK_Success )
			return; // passthrough
		m_FXState.ResetDelay();
		m_pParams->NonRTPC.bHasChanged = false; 
	}

	if ( AK_EXPECT_FALSE( m_pParams->RTPC.bHasChanged ) )
	{
		m_FXState.ComputeTailLength( m_pParams->RTPC.bFeedbackEnabled, m_pParams->RTPC.fFeedback );
		m_pParams->RTPC.bHasChanged = false;
	}
	
	AK_PERF_RECORDING_START( "Delay", 25, 30 );
	// Execute DSP processing synchronously here
	m_FXState.Process( io_pBuffer, m_pParams );
	AK_PERF_RECORDING_STOP( "Delay", 25, 30 );
}
