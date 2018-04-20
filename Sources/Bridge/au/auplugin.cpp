#include "AUEffectBase.h"


#include "../common/client.cpp"


class BridgeEffect : public AUEffectBase {
private:
	BridgeClient *client;

public:
	BridgeEffect(AudioUnit component) : AUEffectBase(component) {
		CreateElements();
		Globals()->UseIndexedParameters(1 + BRIDGE_NUM_PARAMS);
		client = new BridgeClient();
	}

	~BridgeEffect() {
		delete client;
	}

	Float64 GetTailTime() override {
		// 1 year, essentially infinite
		return (60.0*60.0*24.0*365.0);
	}

	bool SupportsTail() override {
		return true;
	}

	UInt32 SupportedNumChannels(const AUChannelInfo** outInfo) override {
		// -1, -1 means any number of matching input/output channels.
		static const AUChannelInfo sChannels[1] = {{-1, -1}};
		if (outInfo)
			*outInfo = sChannels;
		return sizeof(sChannels) / sizeof(AUChannelInfo);
	}

	ComponentResult GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID, CFArrayRef *outStrings) override {
		if (!outStrings)
			return noErr;

		if (inScope == kAudioUnitScope_Global) {
			if (inParameterID == 0) {
				CFStringRef	strings[BRIDGE_NUM_PORTS];
				for (int i = 0; i < BRIDGE_NUM_PORTS; i++) {
					strings[i] = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), i + 1);
				}

				*outStrings = CFArrayCreate(NULL, (const void**) strings, 16, NULL);
				return noErr;
			}
		}
		return kAudioUnitErr_InvalidParameter;
	}

	ComponentResult GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo &outParameterInfo) override {
		outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;

		if (inScope == kAudioUnitScope_Global) {
			if (inParameterID == 0) {
				FillInParameterName(outParameterInfo, CFSTR("Port"), false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
				outParameterInfo.minValue = 0.0;
				outParameterInfo.maxValue = (float) (BRIDGE_NUM_PARAMS - 1);
				outParameterInfo.defaultValue = 0.0;
				return noErr;
			}
			else if (1 <= inParameterID && inParameterID < BRIDGE_NUM_PARAMS + 1) {
				CFStringRef name = CFStringCreateWithFormat(NULL, NULL, CFSTR("CC %d"), (int) (inParameterID - 1));
				FillInParameterName(outParameterInfo, name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
				outParameterInfo.minValue = 0.0;
				outParameterInfo.maxValue = 10.0;
				outParameterInfo.defaultValue = 0.0;
				return noErr;
			}
		}
		return kAudioUnitErr_InvalidParameter;
	}

	OSStatus ProcessBufferLists(AudioUnitRenderActionFlags &ioActionFlags, const AudioBufferList &inBuffer, AudioBufferList &outBuffer, UInt32	inFramesToProcess) override {
		// Push parameters
		int port = (int) GetParameter(0);
		client->setPort(port);
		for (int i = 0; i < BRIDGE_NUM_PARAMS; i++) {
			float param = GetParameter(i + 1) / 10.f;
			client->setParam(i, param);
		}
		client->setSampleRate((int) GetSampleRate());

		// TODO Check that the stream is Float32, add better error handling.
		float input[BRIDGE_INPUTS * inFramesToProcess];
		float output[BRIDGE_OUTPUTS * inFramesToProcess];
		memset(input, 0, sizeof(input));
		memset(output, 0, sizeof(output));
		// Interleave input
		for (int c = 0; c < (int) inBuffer.mNumberBuffers; c++) {
			const float *buffer = (const float*) inBuffer.mBuffers[c].mData;
			for (int i = 0; i < (int) inFramesToProcess; i++) {
				input[BRIDGE_INPUTS * i + c] = buffer[i];
			}
		}
		// Process audio
		client->processStream(input, output, inFramesToProcess);
		// Deinterleave output
		for (int c = 0; c < (int) outBuffer.mNumberBuffers; c++) {
			float *buffer = (float*) outBuffer.mBuffers[c].mData;
			for (int i = 0; i < (int) inFramesToProcess; i++) {
				buffer[i] = output[BRIDGE_OUTPUTS * i + c];
			}
		}
		return noErr;
	}
};



AUDIOCOMPONENT_ENTRY(AUBaseFactory, BridgeEffect)
