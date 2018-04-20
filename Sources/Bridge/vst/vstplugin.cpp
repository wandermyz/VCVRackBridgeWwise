#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-override"
#include "public.sdk/source/vst2.x/audioeffectx.h"
#pragma GCC diagnostic pop


#include "../common/client.cpp"


class BridgeEffect : public AudioEffectX {
private:
	BridgeClient *client;

public:
	BridgeEffect(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, 0, 1 + BRIDGE_NUM_PARAMS) {
		setNumInputs(BRIDGE_INPUTS);
		setNumOutputs(BRIDGE_OUTPUTS);
		setUniqueID('VCVB');
		canProcessReplacing();
		// No DAWs I've tested honor this.
		noTail(true);
		client = new BridgeClient();
	}

	~BridgeEffect() {
		delete client;
	}

	void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) override {
		// Interleave samples
		float input[BRIDGE_INPUTS * sampleFrames];
		float output[BRIDGE_OUTPUTS * sampleFrames];
		for (int i = 0; i < sampleFrames; i++) {
			for (int c = 0; c < BRIDGE_INPUTS; c++) {
				input[BRIDGE_INPUTS*i + c] = inputs[c][i];
			}
		}
		// Process audio
		client->processStream(input, output, sampleFrames);
		// Uninterleave samples
		for (int i = 0; i < sampleFrames; i++) {
			// To prevent the DAW from pausing the processReplacing() calls, add a noise floor so the DAW thinks audio is still being processed.
			float r = (float) rand() / RAND_MAX;
			r = 1.f - 2.f * r;
			// Ableton Live's threshold is 1e-5 or -100dB
			r *= 1.5e-5f; // -96dB
			for (int c = 0; c < BRIDGE_OUTPUTS; c++) {
				outputs[c][i] = output[BRIDGE_OUTPUTS*i + c] + r;
			}
		}

		// TEMP
		// VstTimeInfo *timeInfo = getTimeInfo(0);
		// debug("%f %f %f %f", timeInfo->ppqPos, timeInfo->samplePos, timeInfo->barStartPos, timeInfo->tempo);
	}

	void setParameter(VstInt32 index, float value) override {
		if (index == 0) {
			client->setPort((int) roundf(value * (BRIDGE_NUM_PARAMS - 1.f)));
		}
		else if (index > 0) {
			client->setParam(index - 1, value);
		}
	}

	float getParameter(VstInt32 index) override {
		if (index == 0) {
			return client->getPort() / (BRIDGE_NUM_PARAMS - 1.f);
		}
		else if (index > 0) {
			return client->getParam(index - 1);
		}
		return 0.f;
	}

	void getParameterLabel(VstInt32 index, char *label) override {
		if (index == 0) {
			snprintf(label, kVstMaxParamStrLen, "");
		}
		else if (index > 0) {
			snprintf(label, kVstMaxParamStrLen, "");
		}
	}

	void getParameterDisplay(VstInt32 index, char *text) override {
		if (index == 0) {
			snprintf(text, kVstMaxParamStrLen, "%d", client->getPort() + 1);
		}
		else if (index > 0) {
			snprintf(text, kVstMaxParamStrLen, "%0.2f V", 10.f * client->getParam(index - 1));
		}
	}

	void getParameterName(VstInt32 index, char *text) override {
		if (index == 0) {
			// Port selector
			snprintf(text, kVstMaxParamStrLen, "Port");
		}
		else if (index > 0) {
			// Automation parameters
			snprintf(text, kVstMaxParamStrLen, "CC %d", index - 1);
		}
	}

	bool getEffectName(char *name) override {
		snprintf(name, kVstMaxEffectNameLen, "VCV Bridge");
		return true;
	}

	bool getVendorString(char *text) override {
		snprintf(text, kVstMaxProductStrLen, "VCV");
		return true;
	}

	bool getProductString(char *text) override {
		snprintf(text, kVstMaxVendorStrLen, "VCV Bridge");
		return true;
	}

	VstInt32 getVendorVersion() override {
		return 0;
	}

	void open() override {
	}
	void close() override {
	}
	void suspend() override {
	}
	void resume() override {
	}

	void setSampleRate(float sampleRate) override {
		AudioEffectX::setSampleRate(sampleRate);
		client->setSampleRate((int) sampleRate);
	}

	VstInt32 processEvents(VstEvents *events) override {
		for (int i = 0; i < events->numEvents; i++) {
			VstEvent *event = events->events[i];
			if (event->type == kVstMidiType) {
				VstMidiEvent *midiEvent = (VstMidiEvent*) event;
				// debug("MIDI %02x %02x %02x", midiEvent->midiData[0], midiEvent->midiData[1], midiEvent->midiData[2]);
			}
		}
		return 0;
	}
};


AudioEffect *createEffectInstance (audioMasterCallback audioMaster) {
	return new BridgeEffect(audioMaster);
}
