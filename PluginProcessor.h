/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
enum Slope {
    Slope12, 
    Slope24, 
    Slope36,
    Slope48
};
struct  ChainSettings
{
    float peakFreq{ 0 }, peakGainInDecibels{ 0 }, peakQuality{ 1.f };
    float lowCutFreq{ 0 }, highCutFreq{ 0 };
    Slope lowCutSlope{ Slope::Slope12 }, highCutSlope{ Slope::Slope12 };
};


ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);   // helperfunction that will give us all the parameters values in our data sctruct (above)

//==============================================================================
/**
*/
class NewProjectAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    


    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters", createParameterLayout()};

    // dsp namespace uses a lot of tempate metaprogramming nested namespaces, lets create type alias, 
    //to elemenate a lot of that name spaces, and template definitions
  

   
private:

    using Filter = juce::dsp::IIR::Filter<float>;   //It is a digital filter class in the JUCE DSP module that processes audio by applying an IIR (Infinite Impulse Response) filter ( low-pass, high-pass etc)) to it
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;  // slope of our cut filter, each of filters one octave ( loop from createParameterLayout()  method) 
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;
    MonoChain leftChain, rightChain;    //before uning chains we need to prepare through prepareToPlay

   /*            [leftChannel]                       [rightChannel]
                        │                                   │
        ┌───────────────┼───────────────┐     ┌─────────────┼──────────────┐
        ▼                                   ▼
       [CutFilter]  → [Filter]   →    [CutFilter]        [same]
       (low cut)      (peak EQ)        (high cut)
           │              │                 │
       [F][F][F][F]      [F]            [F][F][F][F]

       Where [F] is an instance of juce::dsp::IIR::Filter<float>

       ProcessorChain allows you to chain together multiple DSP modules (filters, compressors, etc.) in a clean and efficient way.
       */


    enum ChainPosition {   // now we have coefficent for our filter, so we can set out coefficent accordingly. enum represents each links position in the chain
        LowCut, 
        Peak,
        HighCut
     };
   

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessor)
};
