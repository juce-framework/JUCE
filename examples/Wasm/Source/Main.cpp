#include <JuceHeader.h>
#include <iostream>
#include <emscripten.h>
#include <emscripten/html5.h>
void initEngine() {
    juce::initialiseJuce_GUI();
}

void mainLoop() {}
class MyAudioCallback : public juce::AudioIODeviceCallback {
public:
    MyAudioCallback() {}
    void audioDeviceIOCallbackWithContext(const float *const *inputChannelData, int numInputChannels, float *const *outputChannelData, int numOutputChannels, int numSamples, const juce::AudioIODeviceCallbackContext &context) override {
        float* output = (float *)outputChannelData;
        for(int i =0;i < 128;i++) {
            (output[i]) = emscripten_random() * 0.2 - 0.1;
        }
    }
    void audioDeviceAboutToStart(juce::AudioIODevice *device) override {}
    void audioDeviceStopped() override {}
    void audioDeviceError(const juce::String &errorMessage) override {}
};

EM_BOOL OnCanvasClick(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
    std::cout << "Starting Device Works" << std::endl;
    AudioIODevice* device = (AudioIODevice*)userData;
    device->open(1,1,44100,128);
    device->start(new MyAudioCallback());
    return true;
}
int main (int argc, char* argv[])
{
    initEngine();
    auto deviceManager = new juce::AudioDeviceManager();
    auto& types = deviceManager->getAvailableDeviceTypes();
    auto webAudio = types[0];
    auto device = webAudio->createDevice("O","I");

    emscripten_set_click_callback("canvas", device, 0, OnCanvasClick);
    juce::ignoreUnused (argc, argv);
    emscripten_set_main_loop(mainLoop,30,1);
    emscripten_resume_main_loop();
    return 0;
}

