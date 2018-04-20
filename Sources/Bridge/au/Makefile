VERSION = master

RACK_DIR ?= ../..
include $(RACK_DIR)/arch.mk

ifneq ($(ARCH), mac)
$(error The AU plugin is only available for MacOS)
endif

AU_SDK = AudioUnitExamplesAudioUnitEffectGeneratorInstrumentMIDIProcessorandOffline

FLAGS += -fPIC
FLAGS += -arch i386 -arch x86_64
LDFLAGS += -arch i386 -arch x86_64

FLAGS += -I$(RACK_DIR)/include
FLAGS += -I$(AU_SDK)/AUPublic/AUBase -I$(AU_SDK)/AUPublic/OtherBases -I$(AU_SDK)/AUPublic/Utility -I$(AU_SDK)/PublicUtility
LDFLAGS += -framework CoreServices -framework AudioUnit -framework AudioToolbox
LDFLAGS += -bundle

SOURCES += auplugin.cpp
SOURCES += $(AU_SDK)/AUPublic/OtherBases/AUEffectBase.cpp
SOURCES += $(AU_SDK)/AUPublic/Utility/AUBaseHelper.cpp
SOURCES += $(AU_SDK)/AUPublic/Utility/AUBuffer.cpp
SOURCES += $(AU_SDK)/AUPublic/AUBase/AUBase.cpp
SOURCES += $(AU_SDK)/AUPublic/AUBase/AUDispatch.cpp
SOURCES += $(AU_SDK)/AUPublic/AUBase/AUInputElement.cpp
SOURCES += $(AU_SDK)/AUPublic/AUBase/AUOutputElement.cpp
SOURCES += $(AU_SDK)/AUPublic/AUBase/AUPlugInDispatch.cpp
SOURCES += $(AU_SDK)/AUPublic/AUBase/AUScopeElement.cpp
SOURCES += $(AU_SDK)/AUPublic/AUBase/ComponentBase.cpp
SOURCES += $(AU_SDK)/PublicUtility/CAAudioChannelLayout.cpp
SOURCES += $(AU_SDK)/PublicUtility/CABufferList.cpp
SOURCES += $(AU_SDK)/PublicUtility/CAHostTimeBase.cpp
SOURCES += $(AU_SDK)/PublicUtility/CAMutex.cpp
SOURCES += $(AU_SDK)/PublicUtility/CAStreamBasicDescription.cpp
SOURCES += $(AU_SDK)/PublicUtility/CAVectorUnit.cpp

TARGET = VCV-Bridge
BUNDLE = VCV-Bridge.component


all: $(TARGET)

include $(RACK_DIR)/compile.mk

clean:
	rm -rfv build $(TARGET) dist

dist: all
	rm -rf dist
	mkdir -p dist
	mkdir -p dist/$(BUNDLE)/Contents
	mkdir -p dist/$(BUNDLE)/Contents/MacOS
	cp Info.plist dist/$(BUNDLE)/Contents/
	cp $(TARGET) dist/$(BUNDLE)/Contents/MacOS
	strip -S dist/$(BUNDLE)/Contents/MacOS/$(TARGET)
	cd dist && zip -5 -r VCV-Bridge-$(VERSION)-$(ARCH)-au.zip $(BUNDLE)

install: dist
	sudo cp -R dist/$(BUNDLE) /Library/Audio/Plug-Ins/Components/

uninstall:
	sudo rm -rfv /Library/Audio/Plug-Ins/Components/$(BUNDLE)

val: install
	auval -v aufx BRDG VCV_ -strict -de
