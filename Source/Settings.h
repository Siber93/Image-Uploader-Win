/*
  ==============================================================================

    Settings.h
    Created: 6 Apr 2024 5:46:49pm
    Author:  simone.bertolini

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class Settings {


public:
    juce::String nTerm;

    juce::String val1;
    int threshold1;
    juce::String val2;
    int threshold2;
    juce::String val3;
    int threshold3;

    juce::String field1Txt;
    uint16_t minLenField1;
    uint16_t maxLenField1;
    bool subdir1;

    juce::String field2Txt;
    uint16_t minLenField2;
    uint16_t maxLenField2;
    bool subdir2;

    juce::String field3Txt;
    uint16_t minLenField3;
    uint16_t maxLenField3;
    bool subdir3;


    uint16_t imgCompression;
    uint16_t imgZoom;

    juce::String ftpServer;
    juce::String ftpUser;
    juce::String ftpPassword ;
    bool showBarcodeButton;
    juce::String ftpBarcodeFile;;
    uint16_t maxImages;
    bool deleteImgBeforeReset;
    int cameraIndex;


    // Delete copy constructor and assignment operation
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;


    static Settings& getInstance() {
        static Settings instance; // Guaranteed to be destroyed and instantiated on first use
        return instance;
    }

private:
    juce::File myJsonConfigFile { juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory().getChildFile("setting.json") };

    Settings()
    {
        readOrCreateJson();
    }


    void readOrCreateJson() {
        if (!myJsonConfigFile.exists()) {
            
            // Create a new DynamicObject for your JSON structure
            auto* jsonObject = new juce::DynamicObject();

            // Use the setProperty method to assign values
            // Basic settings
            jsonObject->setProperty("nTerm", "Term0003");
            jsonObject->setProperty("val1", "Value 1");
            jsonObject->setProperty("threshold1", 10);
            jsonObject->setProperty("val2", "Value 2");
            jsonObject->setProperty("threshold2", 20);
            jsonObject->setProperty("val3", "Value 3");
            jsonObject->setProperty("threshold3", 30);

            // Detailed fields settings
            jsonObject->setProperty("field1Txt", "Field 1 Text");
            jsonObject->setProperty("minLenField1", 1);
            jsonObject->setProperty("maxLenField1", 100);
            jsonObject->setProperty("subdir1", true);

            jsonObject->setProperty("field2Txt", "Field 2 Text");
            jsonObject->setProperty("minLenField2", 2);
            jsonObject->setProperty("maxLenField2", 200);
            jsonObject->setProperty("subdir2", true);

            jsonObject->setProperty("field3Txt", "Field 3 Text");
            jsonObject->setProperty("minLenField3", 3);
            jsonObject->setProperty("maxLenField3", 300);
            jsonObject->setProperty("subdir3", true);

            // Image settings
            jsonObject->setProperty("imgCompression", 85);
            jsonObject->setProperty("imgZoom", 100);
            jsonObject->setProperty("cameraIndex", 0);

            // FTP settings
            jsonObject->setProperty("ftpServer", "ftp.agribologna.it:21");
            jsonObject->setProperty("ftpUser", "CEM");
            jsonObject->setProperty("ftpPassword", "C1e2M3");
            jsonObject->setProperty("ftpBarcodeFile", "IMPOSTAZIONI/"+ jsonObject->getProperty("nTerm").toString() +".txt");
            
            // Additional settings
            jsonObject->setProperty("showBarcodeButton", true);
            jsonObject->setProperty("maxImages", 5);
            jsonObject->setProperty("deleteImgBeforeReset", false);
            

            // Initialize the file with default values
            juce::var obj(jsonObject);

            // Write the default JSON to file
            juce::FileOutputStream outputStream(myJsonConfigFile);
            if (!outputStream.failedToOpen()) {
                outputStream.writeText(juce::JSON::toString(obj), false, false, "\n");
            }
            return; // Exit after initializing file with defaults
        }

        // File exists, read and parse JSON
        juce::String content = myJsonConfigFile.loadFileAsString();
        juce::var parsedJson = juce::JSON::parse(content);
        if (!parsedJson.isVoid()) {
            // Assuming the JSON structure matches the class structure directly
            nTerm = parsedJson["nTerm"];
            val1 = parsedJson["val1"];
            threshold1 = static_cast<int>(parsedJson["threshold1"]);

            val2 = parsedJson["val2"];
            threshold2 = static_cast<int>(parsedJson["threshold2"]);
            val3 = parsedJson["val3"];
            threshold3 = static_cast<int>(parsedJson["threshold3"]);

            field1Txt = parsedJson["field1Txt"];
            minLenField1 = static_cast<uint16_t>(int(parsedJson["minLenField1"]));
            maxLenField1 = static_cast<uint16_t>(int(parsedJson["maxLenField1"]));
            subdir1 = parsedJson["subdir1"];

            field2Txt = parsedJson["field2Txt"];
            minLenField2 = static_cast<uint16_t>(int(parsedJson["minLenField2"]));
            maxLenField2 = static_cast<uint16_t>(int(parsedJson["maxLenField2"]));
            subdir2 = parsedJson["subdir2"];

            field3Txt = parsedJson["field3Txt"];
            minLenField3 = static_cast<uint16_t>(int(parsedJson["minLenField3"]));
            maxLenField3 = static_cast<uint16_t>(int(parsedJson["maxLenField3"]));
            subdir3 = parsedJson["subdir3"];

            imgCompression = static_cast<uint16_t>(int(parsedJson["imgCompression"]));
            imgZoom = static_cast<uint16_t>(int(parsedJson["imgZoom"]));
            cameraIndex = static_cast<uint16_t>(int(parsedJson["cameraIndex"]));

            ftpServer = parsedJson["ftpServer"];
            ftpUser = parsedJson["ftpUser"];
            ftpPassword = parsedJson["ftpPassword"];
            ftpBarcodeFile = parsedJson["ftpBarcodeFile"];

            showBarcodeButton = parsedJson["showBarcodeButton"];
            maxImages = static_cast<uint16_t>(int(parsedJson["maxImages"]));
            deleteImgBeforeReset = parsedJson["deleteImgBeforeReset"];
            
        }
    }
};
