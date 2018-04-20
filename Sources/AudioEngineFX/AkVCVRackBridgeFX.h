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
// AkDelayFX.h
//
// Sample delay FX implementation.
//
//////////////////////////////////////////////////////////////////////

#ifndef _AK_DELAYFX_H_
#define _AK_DELAYFX_H_

#include "AkDelayFXParams.h"
#include "AkDelayFXDSP.h"

struct BridgeClient;

//-----------------------------------------------------------------------------
// Name: class CAkDelayFX
//-----------------------------------------------------------------------------
class CAkVCVRackBridgeFX : public AK::IAkOutOfPlaceEffectPlugin
{
public:

    /// Constructor
    CAkVCVRackBridgeFX();

	 /// Destructor
    ~CAkVCVRackBridgeFX();

    BridgeClient* client;

	/// Effect plug-in initialization
    AKRESULT Init(	AK::IAkPluginMemAlloc *			in_pAllocator,		/// Memory allocator interface.
					AK::IAkEffectPluginContext *	in_pFXCtx,			/// Sound engine plug-in execution context.
                    AK::IAkPluginParam *			in_pParams,			/// Associated effect parameters node.
                    AkAudioFormat &					in_rFormat			/// Input/output audio format.
                    );
    
	/// Effect plug-in termination
	AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator );

	/// Reset effect
	AKRESULT Reset( );

    /// Effect info query
    AKRESULT GetPluginInfo( AkPluginInfo & out_rPluginInfo );

    /// Effect plug-in DSP processing
	void Execute(	AkAudioBuffer * io_pBuffer		/// Input/Output audio buffer structure.		
		);

    void Execute(
        AkAudioBuffer *							in_pBuffer,		///< Input audio buffer data structure
        AkUInt32								in_uInOffset,	///< Offset position into input buffer data
        AkAudioBuffer *							out_pBuffer		///< Output audio buffer data structure
    );

    /// Skips execution of some frames, when the voice is virtual playing from elapsed time.
    /// This can be used to simulate processing that would have taken place (e.g. update internal state).
    /// Return AK_DataReady or AK_NoMoreData, depending if there would be audio output or not at that point.
    AKRESULT TimeSkip(
        AkUInt32 &io_uFrames	///< Number of frames the audio processing should advance.  The output value should be the number of frames that would be consumed to output the number of frames this parameter has at the input of the function.
    );

	/// Execution processing when the voice is virtual. Nothing special to do for this effect.
	AKRESULT TimeSkip(AkUInt32 in_uFrames){ return AK_DataReady; }	

private:

	/// Cached information
	CAkDelayFXDSP			m_FXState;		/// Internal effect state
    CAkDelayFXParams *		m_pParams;		/// Effect parameter node
	AK::IAkPluginMemAlloc * m_pAllocator;	/// Memory allocator interface
};

#endif // _AK_DELAYFX_H_
