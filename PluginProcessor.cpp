/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName(int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;
    leftChain.prepare(spec);
    rightChain.prepare(spec);

    auto chainSettings = getChainSettings(apvts);
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peakFreq, chainSettings.peakQuality,
        juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));


    if (peakCoefficients)
    {
        auto& peak = rightChain.get<ChainPosition::Peak>();
        *peak.coefficients = *peakCoefficients;
    }

    if (peakCoefficients)
    {
        auto& peak = leftChain.get<ChainPosition::Peak>();
        *peak.coefficients = *peakCoefficients;
    }
   
   // *leftChain.get<ChainPosition::Peak().coefficients = *peakCoefficients;
   // *leftChain.get <ChainPosition::Peak().coefficients = *peakCoefficients;

    auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, 
                                                                                                       sampleRate, 
                                                                                                      2 * (chainSettings.lowCutSlope + 1));
    auto& leftLowCut = leftChain.get < ChainPosition::LowCut>();
    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);

    switch (chainSettings.lowCutSlope)
    {
    case Slope12:
        
        *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        break;
    case Slope24:

        *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
        leftLowCut.setBypassed<1>(false);
        break;
    case Slope36:
        *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
        leftLowCut.setBypassed<1>(false);
        *leftLowCut.get<2>().coefficients = *cutCoefficients[2];
        leftLowCut.setBypassed<2>(false);
        break;
    case Slope48:
        *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
        leftLowCut.setBypassed<1>(false);
        *leftLowCut.get<2>().coefficients = *cutCoefficients[2];
        leftLowCut.setBypassed<2>(false);
        *leftLowCut.get<3>().coefficients = *cutCoefficients[3];
        leftLowCut.setBypassed<3>(false);

        break;
    }


    auto& rightLowCut = rightChain.get < ChainPosition::LowCut>();
    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);

    switch (chainSettings.lowCutSlope)
    {
    case Slope12:

        *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        break;
    case Slope24:

        *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
        rightLowCut.setBypassed<1>(false);
        break;
    case Slope36:
        *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
        rightLowCut.setBypassed<1>(false);
        *rightLowCut.get<2>().coefficients = *cutCoefficients[2];
        rightLowCut.setBypassed<2>(false);
        break;
    case Slope48:
        *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
        rightLowCut.setBypassed<1>(false);
        *rightLowCut.get<2>().coefficients = *cutCoefficients[2];
        rightLowCut.setBypassed<2>(false);
        *rightLowCut.get<3>().coefficients = *cutCoefficients[3];
        rightLowCut.setBypassed<3>(false);

        break;
    }

    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq,
                                                                                                          getSampleRate(),
                                                                                                          2 * (chainSettings.highCutSlope + 1));
    auto& leftHighCut = leftChain.get<ChainPosition::HighCut>();
    leftHighCut.setBypassed<0>(true);
    leftHighCut.setBypassed<1>(true);
    leftHighCut.setBypassed<2>(true);
    leftHighCut.setBypassed<3>(true);

    switch (chainSettings.highCutSlope)
    {
    case Slope12:
        *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
        leftHighCut.setBypassed<0>(false);
        break;
    case Slope24:
        *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
        leftHighCut.setBypassed<0>(false);
        *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
        leftHighCut.setBypassed<1>(false);
        break;
    case Slope36:
        *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
        leftHighCut.setBypassed<0>(false);
        *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
        leftHighCut.setBypassed<1>(false);
        *leftHighCut.get<2>().coefficients = *highCutCoefficients[2];
        leftHighCut.setBypassed<2>(false);
        break;
    case Slope48:
        *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
        leftHighCut.setBypassed<0>(false);
        *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
        leftHighCut.setBypassed<1>(false);
        *leftHighCut.get<2>().coefficients = *highCutCoefficients[2];
        leftHighCut.setBypassed<2>(false);
        *leftHighCut.get<3>().coefficients = *highCutCoefficients[3];
        leftHighCut.setBypassed<3>(false);
        break;
    }

    auto& rightHighCut = rightChain.get<ChainPosition::HighCut>();
    rightHighCut.setBypassed<0>(true);
    rightHighCut.setBypassed<1>(true);
    rightHighCut.setBypassed<2>(true);
    rightHighCut.setBypassed<3>(true);

    switch (chainSettings.highCutSlope)
    {
    case Slope12:
        *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
        rightHighCut.setBypassed<0>(false);
        break;
    case Slope24:
        *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
        rightHighCut.setBypassed<0>(false);
        *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
        rightHighCut.setBypassed<1>(false);
        break;
    case Slope36:
        *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
        rightHighCut.setBypassed<0>(false);
        *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
        rightHighCut.setBypassed<1>(false);
        *rightHighCut.get<2>().coefficients = *highCutCoefficients[2];
        rightHighCut.setBypassed<2>(false);
        break;
    case Slope48:
        *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
        rightHighCut.setBypassed<0>(false);
        *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
        rightHighCut.setBypassed<1>(false);
        *rightHighCut.get<2>().coefficients = *highCutCoefficients[2];
        rightHighCut.setBypassed<2>(false);
        *rightHighCut.get<3>().coefficients = *highCutCoefficients[3];
        rightHighCut.setBypassed<3>(false);
        break;
    }

}

void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void NewProjectAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.


    auto chainSettings = getChainSettings(apvts);
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), 
                                                                                chainSettings.peakFreq,
                                                                                chainSettings.peakQuality,
                                                                                juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));

    if (peakCoefficients)
    {
        auto& peak = rightChain.get<ChainPosition::Peak>();
        *peak.coefficients = *peakCoefficients;
    }

    if (peakCoefficients)
    {
        auto& peak = leftChain.get<ChainPosition::Peak>();
        *peak.coefficients = *peakCoefficients;
    }

    // *leftChain.get<ChainPosition::Peak().coefficients = *peakCoefficients;
    // *leftChain.get <ChainPosition::Peak().coefficients = *peakCoefficients;



    auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
                                                                                                       getSampleRate(),
                                                                                                       2 * (chainSettings.lowCutSlope + 1));
    auto& leftLowCut = leftChain.get < ChainPosition::LowCut>();
    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);

    switch (chainSettings.lowCutSlope)
    {
    case Slope12:

        *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        break;
    case Slope24:

        *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
        leftLowCut.setBypassed<1>(false);
        break;
    case Slope36:
        *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
        leftLowCut.setBypassed<1>(false);
        *leftLowCut.get<2>().coefficients = *cutCoefficients[2];
        leftLowCut.setBypassed<2>(false);
        break;
    case Slope48:
        *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
        leftLowCut.setBypassed<0>(false);
        *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
        leftLowCut.setBypassed<1>(false);
        *leftLowCut.get<2>().coefficients = *cutCoefficients[2];
        leftLowCut.setBypassed<2>(false);
        *leftLowCut.get<3>().coefficients = *cutCoefficients[3];
        leftLowCut.setBypassed<3>(false);

        break;
    }


    auto& rightLowCut = rightChain.get < ChainPosition::LowCut>();
    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);

    switch (chainSettings.lowCutSlope)
    {
    case Slope12:

        *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        break;
    case Slope24:

        *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
        rightLowCut.setBypassed<1>(false);
        break;
    case Slope36:
        *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
        rightLowCut.setBypassed<1>(false);
        *rightLowCut.get<2>().coefficients = *cutCoefficients[2];
        rightLowCut.setBypassed<2>(false);
        break;
    case Slope48:
        *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
        rightLowCut.setBypassed<0>(false);
        *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
        rightLowCut.setBypassed<1>(false);
        *rightLowCut.get<2>().coefficients = *cutCoefficients[2];
        rightLowCut.setBypassed<2>(false);
        *rightLowCut.get<3>().coefficients = *cutCoefficients[3];
        rightLowCut.setBypassed<3>(false);

        break;
    }

    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq,
        getSampleRate(),
        2 * (chainSettings.highCutSlope + 1));
    auto& leftHighCut = leftChain.get<ChainPosition::HighCut>();
    leftHighCut.setBypassed<0>(true);
    leftHighCut.setBypassed<1>(true);
    leftHighCut.setBypassed<2>(true);
    leftHighCut.setBypassed<3>(true);

    switch (chainSettings.highCutSlope)
    {
    case Slope12:
        *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
        leftHighCut.setBypassed<0>(false);
        break;
    case Slope24:
        *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
        leftHighCut.setBypassed<0>(false);
        *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
        leftHighCut.setBypassed<1>(false);
        break;
    case Slope36:
        *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
        leftHighCut.setBypassed<0>(false);
        *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
        leftHighCut.setBypassed<1>(false);
        *leftHighCut.get<2>().coefficients = *highCutCoefficients[2];
        leftHighCut.setBypassed<2>(false);
        break;
    case Slope48:
        *leftHighCut.get<0>().coefficients = *highCutCoefficients[0];
        leftHighCut.setBypassed<0>(false);
        *leftHighCut.get<1>().coefficients = *highCutCoefficients[1];
        leftHighCut.setBypassed<1>(false);
        *leftHighCut.get<2>().coefficients = *highCutCoefficients[2];
        leftHighCut.setBypassed<2>(false);
        *leftHighCut.get<3>().coefficients = *highCutCoefficients[3];
        leftHighCut.setBypassed<3>(false);
        break;
    }

    auto& rightHighCut = rightChain.get<ChainPosition::HighCut>();
    rightHighCut.setBypassed<0>(true);
    rightHighCut.setBypassed<1>(true);
    rightHighCut.setBypassed<2>(true);
    rightHighCut.setBypassed<3>(true);

    switch (chainSettings.highCutSlope)
    {
    case Slope12:
        *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
        rightHighCut.setBypassed<0>(false);
        break;
    case Slope24:
        *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
        rightHighCut.setBypassed<0>(false);
        *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
        rightHighCut.setBypassed<1>(false);
        break;
    case Slope36:
        *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
        rightHighCut.setBypassed<0>(false);
        *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
        rightHighCut.setBypassed<1>(false);
        *rightHighCut.get<2>().coefficients = *highCutCoefficients[2];
        rightHighCut.setBypassed<2>(false);
        break;
    case Slope48:
        *rightHighCut.get<0>().coefficients = *highCutCoefficients[0];
        rightHighCut.setBypassed<0>(false);
        *rightHighCut.get<1>().coefficients = *highCutCoefficients[1];
        rightHighCut.setBypassed<1>(false);
        *rightHighCut.get<2>().coefficients = *highCutCoefficients[2];
        rightHighCut.setBypassed<2>(false);
        *rightHighCut.get<3>().coefficients = *highCutCoefficients[3];
        rightHighCut.setBypassed<3>(false);
        break;
    }


    juce::dsp::AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);
}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
   return new NewProjectAudioProcessorEditor(*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NewProjectAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}
ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts) {    //getter function that pulls the current values from the plugin parameters 

    ChainSettings settings; // this struct will be filled with the current values from the plugin AudioProcessorValueTreeState parameters

    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();   // looks up the parameter " LowCut Freq" from the AudioProcessorValueTreeState , loads its cuttent value, and stores it in settings.lowCutFreq. The point of using .load() here is to safely read the value fromm a std::atomic<float>, which is the type returned by getRawParameterValue() in Juce
    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
    settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
    settings.peakQuality = apvts.getRawParameterValue("Peak Quality")->load();
    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());

    // settings.lowCutBypassed = apvts.getRawParameterValue("LowCut Bypassed")->load() > 0.5f;
    // settings.peakBypassed = apvts.getRawParameterValue("Peak Bypassed")->load() > 0.5f;
    // settings.highCutBypassed = apvts.getRawParameterValue("HighCut Bypassed")->load() > 0.5f;


    return settings;
}
juce::AudioProcessorValueTreeState::ParameterLayout NewProjectAudioProcessor::createParameterLayout()
{

    //This function defines the list of parameters that our plugin will use, getChainSettings method reads the current values of the parameters that we defined in this method. 
    //It fetches the current user-defined or automated values and stores them in your own ChainSettings struct
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq",
        "LowCut Freq",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        20.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq",
        "HighCut Freq",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        20000.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq",
        "Peak Freq",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        750.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain",
        "Peak Gain",
        juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
        0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Quality",
        "Peak Quality",
        juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
        1.f));
    // peak bend quality control ( how narrow or wide band is)(q factor)
    juce::StringArray stringArray;
    for (int i = 0; i < 4; ++i)
    {
        juce::String str;
        str << (12 + i * 12);
        str << " db/Oct";
        stringArray.add(str);
    }

    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));   // Creating AudioParameterChoice object. Where the user could select one option out of many ( like a drop-down menu or combo box)
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", stringArray, 0));   // Creating AudioParameterChoice object. Where the user could select one option out of many ( like a drop-down menu or combo box)

    layout.add(std::make_unique<juce::AudioParameterBool>("LowCut Bypassed", "LowCut Bypassed", false));   // Creating AudioParameterBool object. Represents a true/false parameter(like a toggle switch or on/off button)  false or true - defaul value( bypassed or not, enabled or not)
    layout.add(std::make_unique<juce::AudioParameterBool>("Peak Bypassed", "Peak Bypassed", false));       // Creating AudioParameterBool object. Represents a true/false parameter(like a toggle switch or on/off button)  false or true - defaul value( bypassed or not, enabled or not)
    layout.add(std::make_unique<juce::AudioParameterBool>("HighCut Bypassed", "HighCut Bypassed", false));    // Creating AudioParameterBool object. Represents a true/false parameter(like a toggle switch or on/off button)  false or true - defaul value( bypassed or not, enabled or not)
    layout.add(std::make_unique<juce::AudioParameterBool>("Analyzer Enabled", "Analyzer Enabled", true));     // Creating AudioParameterBool object. Represents a true/false parameter(like a toggle switch or on/off button)  false or true - defaul value( bypassed or not, enabled or not)

    return layout;
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}