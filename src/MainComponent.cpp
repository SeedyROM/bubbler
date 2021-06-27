#include <cmath>

#include "MainComponent.h"

MainComponent::MainComponent() : AudioAppComponent()
{
    setSize(600, 400);
    setAudioChannels(0, 1);
}

MainComponent::~MainComponent() {
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double newSampleRate) {
    sampleRate = newSampleRate;
    expectedSamplesPerBlock = samplesPerBlockExpected;

    // Create process spec for oscillators
    juce::dsp::ProcessSpec voiceSpecification = { sampleRate, (unsigned int) expectedSamplesPerBlock, 1 };
    juce::dsp::ProcessSpec filterSpecification = { sampleRate, (unsigned int) expectedSamplesPerBlock, 1 };

    // Initialize the oscillators
    for(size_t i = 0; i < voices.size(); ++i) {
        voices[i].prepare(voiceSpecification);
        voices[i].initialise([](float x) { return std::sin(x); }, 1024);
    }

    // Initialize the filter
    filter.prepare(filterSpecification);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);

}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    bufferToFill.clearActiveBufferRegion();

    for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
    {
        auto* channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);

        for (auto i = 0; i < bufferToFill.numSamples; ++i)
        {
            for(size_t j = 0; j < voices.size(); ++j) {
                channelData[i] += voices[j].processSample(0.0f) / voices.size();
                channelData[i] *= 0.7;
            }
            channelData[i]  = filter.processSample(1, channelData[i]);
        }
    }
}

void MainComponent::releaseResources() {}

void MainComponent::mouseMove(const juce::MouseEvent& event) {
    for(size_t i = 0; i < voices.size(); ++i) {
        voices[i].setFrequency((event.getPosition().x * 2) * (i + 1.0), true);
    }
    filter.setCutoffFrequency((std::abs(event.getPosition().y) * 8));
    repaint();
}

void MainComponent::paint(juce::Graphics& g)
{
    //(Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setFont(juce::Font(30.0f));
    g.setColour(juce::Colours::white);
    g.drawText(
        juce::String::formatted(
            "Freq: %f\nCutoff: %f",
            voices[3].getFrequency(),
            filter.getCutoffFrequency()
        ), 
        getLocalBounds(), 
        juce::Justification::centred,
        true
    );

}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
