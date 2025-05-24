/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.








   Audio processing means:
1. Capturing audio input (from a microphone, instrument, etc.)
2. Processing the digital samples (applying gain, filters, effects, etc.)
3. Sending audio output (to speakers or files)
4. Doing this in real-time, with low latency and no dropouts

To do this correctly, a framework or app must:
• Communicate with the operating system’s audio drivers (like ASIO, CoreAudio, WASAPI, etc.)
• Manage audio buffers (chunks of audio data in memory)
• Keep strict timing constraints (audio must be processed on time)
• Use threads safely (usually real-time safe threads)
• Avoid memory allocation or blocking inside the audio callback

General-purpose frameworks (like Qt, React, Django, etc.):
• Can do some of it — especially GUI, file handling, and cross-platform builds.
• Cannot do specialized audio/MIDI processing well or easily.

 JUCE is a domain-specific framework:
• It is designed specifically for audio applications.
• That’s why it comes with audio I/O, real-time audio processing, and plugin support built in.
• You don’t need to deal with low-level APIs like CoreAudio (macOS), WASAPI (Windows), ALSA (Linux), etc. JUCE wraps all that for you.


So What Makes a Framework Able to Handle Audio Processing?
It depends on whether the framework:
1)Has access to audio drivers (Can talk directly to CoreAudio, WASAPI, ALSA, etc.)
Has real-time safe threading (Can guarantee code runs quickly and on time)
Handles audio buffers (Provides APIs to read/write audio data efficiently)
Minimizes latencY (Is designed to avoid delays in input/output)
Abstracts low-level details (Provides simple APIs so you don’t write raw driver code)
JUCE has all of the above, and that’s why it’s excellent for audio.

Behind the Scenes: How JUCE Does It
1. JUCE includes wrappers for platform-specific audio APIs:
• CoreAudio (macOS)
• WASAPI / ASIO (Windows)
• ALSA / JACK (Linux)
2. It creates an audio callback thread:
• This function runs repeatedly every few milliseconds (often 512 samples or less)
• Inside it, you implement your processBlock() method
3. JUCE passes you the audio buffer, and you apply your processing.
4. The output buffer is sent back to the OS’s audio system and played.



This flow is precise, fast, and real-time, and that’s what most frameworks do not offer out of the box


At its core, an audio buffer is just a chunk (block) of sound samples held in memory temporarily while being recorded, processed, or played back. You’re recording audio. Your mic is capturing sound 44,100 times per second (standard sample rate).
But instead of processing each of those 44,100 samples one-by-one, the system puts them into blocks like: [ 256 samples ] → [ 256 samples ] → [ 256 samples ] → ... Each [ ... ] is an audio buffer.

What’s Inside a Buffer?
An audio buffer is just a multidimensional array (usually float numbers between -1.0 and 1.0) representing sound pressure.
For stereo (2-channel) audio:
AudioBuffer<float> buffer;
buffer[channel][sample] = value;

It might look like:
Left Channel:   [0.12,  0.18, 0.03, -0.09, ...]
Right Channel:  [0.10,  0.16, 0.05, -0.07, ...]

If the buffer size is 512 samples:
• Each channel has 512 values.
• You process these inside your plugin (like applying gain or EQ).
• Then the processed buffer is sent to the speaker

void MyProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] *= gain; // Apply gain to each sample
        }
    }
}
You’re literally manipulating raw sample values in the buffer for each frame of audio.

Why It Matters
Buffers allow:
• Efficient processing (instead of per-sample)
• Synchronization with the audio hardware
• Control of latency: smaller buffer = lower delay, but higher CPU usage
Buffer Size
Pros
Cons
Small (64–128)
Low latency (good for live)
More CPU-intensive
Large (512–1024)
CPU-efficient
Higher latency (good for playback
[Mic / Input Device]
        ↓
[Audio Driver / OS Audio API]
        ↓
[Audio Buffer (Input)]
        ↓
[Your App (JUCE plugin or app) — processBlock()]
        ↓
[Audio Buffer (Output)]
        ↓
[Audio Driver / OS Audio API]
        ↓
[Speakers / Output Device]

In-Depth Signal Flow Example
Let’s say you’re recording voice and applying a reverb effect:
1. Microphone
• Captures your voice as an analog waveform.
2.  Audio Interface (ADC)
• Converts analog waveform → stream of digital samples (e.g., 44,100 samples/sec).
• Groups them into a buffer (e.g., 512 samples) and hands it to the OS.
3. Operating System + Driver
• Sends the buffer to your app through JUCE (using platform-specific APIs).
4. Your App (JUCE Plugin)

• Inside processBlock():
• Reads the buffer
• Applies gain, EQ, reverb, etc.
• Writes the processed samples into the output buffer
5. JUCE Engine / Host
• Sends the processed buffer to the audio output system.
6. Audio Interface (DAC)
• Converts digital buffer → analog signal.
7.  Speakers / Headphones
• Outputs the final sound you hear.
Real-Time Processing: Timing Matters!


Example with 512-sample buffer at 44.1kHz:
• Duration of buffer = 512 / 44100 ≈ 11.6 milliseconds
• That means every 11.6 ms, a new buffer arrives.
• Your app must finish processing each buffer before the next one comes, or you’ll get glitches/dropouts.


This is why:
• JUCE gives you tight control over buffers
• You can’t allocate memory or do slow tasks in processBlock()

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct CustomRotarySlider : juce::Slider {
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox) {

    }
};

//==============================================================================
/**
*/
class NewProjectAudioProcessorEditor  : public juce::AudioProcessorEditor   
{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;  // if we have any child components (buttoms, sliders, spectrum analizer, responce curve) they should go in this function

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NewProjectAudioProcessor& audioProcessor;

    CustomRotarySlider lowCutSlider, highCutSlider, peakFreqSlider, peakGainSlider, peakQualitySlider;

    using APTVS = juce::AudioProcessorValueTreeState;
    using AttachmentToParam = APTVS::SliderAttachment;
    AttachmentToParam lowCutSliderAttach, highCutSliderAttach, peakFreqSliderAttach, peakGainSliderAttach, peakQualitySliderAttach;


    std::vector<juce::Component*>getComponets();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};


























// Why Debug Builds Often Fail in Ableton :
// Debug ≠ Optimized :
//Debug builds are meant for internal testing and stepping through code, not real - world use.
//They include extra debugging symbols and no compiler optimizations, which can make them unstable or incompatible with DAWs like Ableton.
//
//Some DAWs Reject Debug Plugins :
//Ableton(and some other DAWs) may silently refuse to load plugins that :
//Aren’t signed correctly
//Depend on debug - only DLLs(e.g., MSVC debug runtime)
//Throw debug assertions on startup
//
//Dependency Issues :
//Debug builds sometimes require debug versions of JUCE, runtime libraries, etc.
//These are not available or expected in production environments like Ableton.
// 
//Caching Confusion :
//If you scanned a broken debug version first, Ableton might cache that bad state.
//Then even after switching to Release, it might keep misbehaving until you fully clear / rescan.
//
//
//
//
//Debug build
//You're debugging with breakpoints
//You want verbose error messages and stack traces
//You're testing your code logic — not the actual plugin in a real-world host
//
//Release build
//You're exporting your plugin to be used in a DAW
//You're testing performance or UI responsiveness
//You're packaging it for release or sharing
//
//
//Why use C : \Program Files\Common Files\VST3\ ?
//DAWs expect it
//Most major DAWs on Windows(Ableton Live, FL Studio, Cubase, Studio One, etc.) scan this folder by default when looking for VST3 plugins.If your plugin isn’t there, the DAW often won’t find it unless you manually point it to the custom path.
//
//It's the official VST3 location
//This folder is defined by Steinberg(the creators of the VST standard) as the default global location for VST3 plugins :
//
//
//
//C : \Program Files\Common Files\VST3\
//Admin - level install
//Plugins here are available to all users on the system.If you only copy to a personal folder or build / debug path, it's not a proper "install" from the DAW's perspective.
//
//Avoids multiple confusing copies
//When testing or rebuilding your plugin, if you have duplicates in other folders(like build / Release), it’s easy to accidentally load the wrong version or cause cache conflicts.