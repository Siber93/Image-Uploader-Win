#include "MainComponent.h"
#include "opencv2/opencv.hpp"
#include "Settings.h"

//#include <include_juce_core.cpp>

//==============================================================================
MainComponent::MainComponent()
{

    // Disable buttons
    addImgBtn.setEnabled(false);
    sendImgBtn.setEnabled(false);
    resetBtn.setEnabled(false);


    //Initialize ImageButtons array
    imageToSend = new juce::ImageButton[Settings::getInstance().maxImages];

    // Creating add images routine
    addImgBtn.setButtonText("+");

    addImgBtn.onClick = [this]() {
        // Check if the max number of images has been reached
        if (imageIndex >= Settings::getInstance().maxImages)
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                "Maximum size exceeded",
                "You have reached the maximum number of images",
                "OK");
        }
        else {
            
            // Set the content for the new image
            imageToSend[imageIndex].setImages(false, true, true, cameraView.createCopy(), 1, juce::Colours::transparentWhite, cameraView.createCopy(), 0.5f, juce::Colours::transparentWhite, cameraView.createCopy(), 0.3f, juce::Colours::transparentWhite);
            
            // Let's create the imagebutton onclick routine to remove this image from the list
            int ind = imageIndex;
            imageToSend[imageIndex].onClick = [this, ind]() {
                // Check if user is sure

                const auto callback = juce::ModalCallbackFunction::create([this, ind](int result) {
                    if (result == 0) {}// result == 0 means you click Cancel
                    if (result == 1) {
                        // Processing delete by left shifting 
                        for (int i = ind; i < Settings::getInstance().maxImages - 1; i++)
                        {
                            imageToSend[i].setImages(false, true, true, imageToSend[i + 1].getNormalImage(), 1, juce::Colours::transparentWhite, imageToSend[i + 1].getOverImage(), 0.5f, juce::Colours::transparentWhite, imageToSend[i + 1].getDownImage(), 0.3f, juce::Colours::transparentWhite);
                        }
                        // Descrese the image counter
                        imageIndex--;
                        // Remove from the video
                        removeChildComponent(&imageToSend[imageIndex]);
                    }// result == 1 means you click OK
                });

                juce::AlertWindow::showOkCancelBox(juce::AlertWindow::QuestionIcon,
                    "Deleting image",
                    "Do you want to proceed deleting the selected image? ",
                    "Yes", "No", nullptr, callback);
                
            };
            // Make the just added image visible
            addAndMakeVisible(imageToSend[imageIndex]);
            // Increase the image counter
            imageIndex++;
            // Let's trig UI Update
            resized();
        }
    };

    // Make the add image button visible
    addAndMakeVisible(addImgBtn);

    // Showing reset image
    resetBtn.setButtonText("Reset");
    resetBtn.onClick = [this]() {
        // Stop service thread
        imgUploaderService.stopThread(2000);

        // Clear images
        for (int i = 0; i < imageIndex; i++)
        {
            // Remove from the video
            removeChildComponent(&imageToSend[i]);
        }
        // Reset the image counter
        imageIndex = 0;
        
        
        // Restart service thread
        imgUploaderService.startThread();
    };
    addAndMakeVisible(resetBtn);

    // Showing send btn
    sendImgBtn.setButtonText("Send");
    sendImgBtn.onClick = [this]() {
        const auto callback = juce::ModalCallbackFunction::create([this](int result) {
            if (result == 0) {}// result == 0 means you click Cancel
            if (result == 1) {
                // Processing saving
                // id_docYY_MM_dd_HH-mm-ss_N_term_barcode_f2_f3_N.jpg
                juce::Time currentTime = juce::Time::getCurrentTime();

                // Format the current time according to the specified pattern
                // Patterns: YY (year), MM (month), dd (day), HH (hour), mm (minute), ss (second)
                juce::String formattedTime = currentTime.formatted("%y_%m_%d_%H-%M-%S");


                // Apply the font to the Graphics context    
                juce::String bc = imgUploaderService.getBarcode();
                bc = bc.removeCharacters("\r");
                bc = bc.removeCharacters("\n");
                // Check if the ftp service has gotten the main file content
                juce::StringArray part;
                if (bc != "")
                {
                    // Let's write content on the screen
                    part = juce::StringArray::fromTokens(bc, "*", "\"");
                }
                else
                {
                    part.add("0000");
                    part.add("0000");
                    part.add("0000");
                }
                // Get file lock
                imgUploaderService.filesMtx.lock();

                juce::String id_doc = "";
                for (int i = 0; i < imageIndex; i++)
                {
                    juce::String imageFileName = id_doc + formattedTime + "_" + Settings::getInstance().nTerm + "_" + part[0] + "_" + part[1] + "_" + part[2] + "_" + juce::String(i + 1) + ".jpg";
                    if (!saveImageToFile(getClippedImage(imageToSend[i].getNormalImage(),100 - Settings::getInstance().imgZoom), imageFileName, (100 - Settings::getInstance().imgCompression) / 100))
                    {
                        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                            "Error during saving",
                            "Something went wrong during saving",
                            "Yes");
                        // Release mutex
                        imgUploaderService.filesMtx.unlock();
                        return;
                    }
                }

                // Release mutex
                imgUploaderService.filesMtx.unlock();

                // Reset
                resetBtn.triggerClick();
                
            }// result == 1 means you click OK
            });

        juce::AlertWindow::showOkCancelBox(juce::AlertWindow::QuestionIcon,
            "Sending image",
            "Do you want to proceed by sending the images? ",
            "Yes", "No", nullptr, callback);
    };
    addAndMakeVisible(sendImgBtn);

    // Run the Camera capture thread
    //cam = new Camera(this);
    //cam->startThread();

    cameraDevice.reset(juce::CameraDevice::openDevice(Settings::getInstance().cameraIndex , 128, 64, 10000, 10000, true));

    if (cameraDevice != nullptr)
    {
       

        // Connect the camera to the VideoComponent
        if (auto* camera = cameraDevice.get())
        {
            camera->addListener(&cam);
        }
    }

    // Run the FTP Client Service thread
    imgUploaderService.startThread();

    setSize(600, 400);

    // Run the Camera ui picture refreshing Thread
    startTimer(40);

    
}


juce::Image MainComponent::getClippedImage(const juce::Image& originalImage, float k)
{
    // Ensure k is within the range [0, 100]
    k = juce::jlimit(0.0f, 100.0f, k);

    // Calculate the scaling factor based on k
    float scale = k / 100.0f;

    int originalWidth = originalImage.getWidth();
    int originalHeight = originalImage.getHeight();

    // Calculate new dimensions
    int newWidth = static_cast<int>(originalWidth * scale);
    int newHeight = static_cast<int>(originalHeight * scale);

    // Calculate the position to center the new image
    int xOffset = (originalWidth - newWidth) / 2;
    int yOffset = (originalHeight - newHeight) / 2;


    return  originalImage.getClippedImage(juce::Rectangle<int>(xOffset, yOffset, newWidth, newHeight));

    // Create a new image with the same format as the original
    /*juce::Image clippedImage = juce::Image(originalImage.getFormat(), originalWidth, originalHeight, true);

    // Create a Graphics context to draw onto the new image
    juce::Graphics g(clippedImage);

    // Clear the image to make sure it is fully transparent
    g.fillAll(juce::Colours::transparentBlack);

    // Draw the scaled original image onto the new image
    g.drawImage(originalImage,
        juce::Rectangle<int>(xOffset, yOffset, newWidth, newHeight).toFloat(),
        juce::RectanglePlacement(juce::RectanglePlacement::centred));

    return clippedImage;*/
}

MainComponent::~MainComponent()
{
    //cam.stopThread(0);
    imgUploaderService.stopThread(0);
    
    cv::destroyAllWindows();
}


void MainComponent::timerCallback()
{
    // Repaint the image sector only
    //repaint(0, 0, cameraView.getWidth() * getWidth() / 1000, cameraView.getHeight() * getWidth() / 1000);
    repaint();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));    


    g.setColour (juce::Colours::white);
    // Set proportional font size
    g.setFont(juce::Font(getWidth() / 35));
    //font.setBold(true);

    // Apply the font to the Graphics context    
    juce::String bc = imgUploaderService.getBarcode();
    // Check if the ftp service has gotten the main file content    
    if (bc != "")
    {
        // Let's write content on the screen
        auto part = juce::StringArray::fromTokens(bc, "*", "\"");
        for (int i = 0; i < part.size(); i++)
        {
            g.drawText(part[i], juce::Rectangle<int>(getWidth() * 3 / 4, i*70 + 30, getWidth() / 3, 30), juce::Justification::topLeft, true);
        }
        // Enable buttons
        addImgBtn.setEnabled(true);
        sendImgBtn.setEnabled(true);
        resetBtn.setEnabled(true);
    }
    else {
        // Disable buttons
        addImgBtn.setEnabled(false);
        sendImgBtn.setEnabled(false);
        resetBtn.setEnabled(false);
    }

    // Let's write terminal number on the screen
    g.setFont(juce::Font(getWidth() / 60));
    g.drawText(Settings::getInstance().nTerm, juce::Rectangle<int>(20, getHeight() - 20, getWidth() / 3, 15), juce::Justification::bottomLeft, true);


    // Refresh Camera image
    cam.getImage(&cameraView);
    if(&cameraView !=nullptr)
        g.drawImageAt(cameraView.rescaled(cameraView.getWidth() * getHeight() / 1200, cameraView.getHeight() * getHeight() / 1200), 0, 0, false);
    
    // Show the busy indicator if the ftp service has not gotten the main file content yet
    if(bc == "")
        getLookAndFeel().drawSpinningWaitAnimation(g, juce::Colours::red, getWidth() / 2-50, getHeight() / 2-50, 100, 100);
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    sendImgBtn.setBounds(juce::Rectangle<int>(getWidth() - 80, getHeight() - 80, 60, 60));
    resetBtn.setBounds(juce::Rectangle<int>(getWidth() - 160, getHeight() - 80, 60, 60));
    addImgBtn.setBounds(juce::Rectangle<int>(getWidth() - 80, getHeight() - 180, 60, 60));
    
    for (int i = 0; i < imageIndex; i++)
    {
        int size = getWidth() / Settings::getInstance().maxImages;
        int diff = 80 / Settings::getInstance().maxImages;
        imageToSend[i].setBounds(juce::Rectangle<int>(size * i + 10 - diff * i, getHeight()-size, size-20 - diff, size-20));
    }
}

bool MainComponent::saveImageToFile(const juce::Image& image, juce::String fName, float quality)
{
    // Specify the file path to save the image
    juce::File file(juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory().getChildFile("images"));
    // Check if images directory exists
    if (!file.exists())
    {
        file.createDirectory();
    }
        
    file = file.getChildFile(fName);

    // Create a FileOutputStream to write to this file
    juce::FileOutputStream outputStream(file);

    // Check if the stream opened successfully
    if (!outputStream.openedOk())
    {
        DBG("Failed to open the file for writing.");
        return false;
    }

    // Write the image to the stream as a JPEG
    juce::JPEGImageFormat jpegFormat;
    jpegFormat.setQuality(quality); // Set quality from 0.0 (most compression) to 1.0 (least compression)
    bool result = jpegFormat.writeImageToStream(image, outputStream);

    // Check if the image was written successfully
    if (result) {
        DBG("Image saved successfully to " + file.getFullPathName());
        return true;
    }
    else {
        DBG("Failed to save image.");
        return false;
    }


   
}
