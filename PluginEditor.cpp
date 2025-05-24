/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
using namespace juce;

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), 

   // lowCutSlider(audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
    //highCutSlider(audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
    //peakFreqSlider(audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
    //peakGainSlider(audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
   // peakQualitySlider(audioProcessor.apvts.getParameter("Peak Quality"), " "),

    lowCutSliderAttach(audioProcessor.apvts, "LowCut Freq", lowCutSlider),
    highCutSliderAttach(audioProcessor.apvts, "HighCut Freq", highCutSlider),
    peakFreqSliderAttach(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
    peakGainSliderAttach(audioProcessor.apvts, "Peak Gain", peakGainSlider),
    peakQualitySliderAttach(audioProcessor.apvts, "Peak Quality", peakQualitySlider)

//lowCutSliderAttach, highCutSliderAttach, peakFreqSliderAttach, peakGainSliderAttach, peakQualitySliderAttach;
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    for (auto* eachComp : getComponets()) {
        addAndMakeVisible(eachComp);
    }
    setSize (700, 450);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
   // g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll(juce::Colours::darkgreen);   // filling with black background
    auto displayBox = getLocalBounds();

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void NewProjectAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    // Define some proportions for layout distribution

  /*  auto area = getLocalBounds();
    auto responseDisplayArea = area.removeFromTop(area.getHeight() * 0.3f);
  
  
    auto lowCutSliderArea = area.removeFromLeft(area.getWidth() * 0.33f);
    auto highCutSliderArea = area.removeFromRight(area.getWidth() * 0.5f);
  
    lowCutSlider.setBounds(lowCutSliderArea);
    highCutSlider.setBounds(highCutSliderArea);
  
    peakFreqSlider.setBounds(area.removeFromTop(area.getHeight() * 0.33f));
    peakGainSlider.setBounds(area.removeFromTop(area.getHeight() * 0.5f));
    peakQualitySlider.setBounds(area);*/
    


    //area now represents the drawable area for placing sliders and knobs.
    auto area = getLocalBounds();
    int sliderWidth = area.getWidth() / 5;      // dividing the total width into three equal parts for top three sliders
    int sliderHeight = area.getHeight() * 0.6;   //we allocate 60% of the vertical space for the top row of sliders

    peakFreqSlider.setBounds(0, 0, sliderWidth, sliderHeight);   // first slider goes at the top left corner ( x = 0, y = 0) with the defined width/height
    peakGainSlider.setBounds(sliderWidth*2, 0, sliderWidth, sliderHeight); // second slider is placed immidiatly to the right of the first one ( x= sliderWidth, y = sliderWidth)
    peakQualitySlider.setBounds(sliderWidth * 4, 0, sliderWidth, sliderHeight);  


    //Bottom knobs
    int knowWidth = area.getWidth() / 2;   //dividing the total width into two halves for the two rotary knobs underneath
    int knowHeight = area.getHeight() * 0.4;  // you assigh 40% of the height to the two bottom knobs

    lowCutSlider.setBounds(0, sliderHeight, knowWidth, knowHeight); // first knob starts at the bottom left (x = 0, y = sliderheight(right below the top slider) 
    highCutSlider.setBounds(knowWidth, sliderHeight, knowWidth, knowHeight);
}
std::vector<juce::Component*>NewProjectAudioProcessorEditor::getComponets() {
    return{
        &lowCutSlider, &highCutSlider, &peakFreqSlider, &peakGainSlider, &peakQualitySlider
    };
}
