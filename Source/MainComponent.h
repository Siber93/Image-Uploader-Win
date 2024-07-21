#pragma once

#include <JuceHeader.h>
#include "Camera.h"
#include "ImageUploaderService.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::Component, private juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    // Your private member variables go here...
    juce::Image cameraImage = { juce::Image::PixelFormat::RGB,640, 480, true };
    ImageUploaderService imgUploaderService;

    juce::TextButton addImgBtn;
    juce::TextButton sendImgBtn;
    juce::TextButton resetBtn;
    juce::ImageButton* imageToSend;
    int imageIndex = 0;
    juce::CameraDevice* cameraDevice;
    juce::Component* cameraVideoComp;
    Camera camListener;
    bool  firstResizing = false;
private:
    //std::unique_ptr<juce::CameraDevice> cameraDevice;

    void timerCallback() override;

    /// <summary>
    /// Save the image to the local dir
    /// </summary>
    bool saveImageToFile(const juce::Image& image, juce::String fName, float quality);

    /// <summary>
    /// Return the image clipped by specified zoom value
    /// </summary>
    /// <param name="originalImage">Image to clip</param>
    /// <param name="k">zoom</param>
    /// <returns></returns>
    juce::Image MainComponent::getClippedImage(const juce::Image& originalImage, float k);


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
