/*
  ==============================================================================

    Network.h
    Created: 6 Apr 2024 8:16:49am
    Author:  simone.bertolini

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Network {
public:
    static bool isConnectedToInternet()
    {
        juce::URL url("http://www.google.com"); // Use a reliable URL
        std::unique_ptr<juce::InputStream> stream(url.createInputStream(false));
        return stream.get() != nullptr; // If the stream is not null, internet is likely available
    }

};