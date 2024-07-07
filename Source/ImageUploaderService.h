/*
  ==============================================================================

    ImageUploaderService.h
    Created: 6 Apr 2024 8:06:13am
    Author:  simone.bertolini

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Network.h"
#include "ftplib.h"
#include "Settings.h"


class ImageUploaderService : public juce::Thread
{
public:
    // Mutex to access local image files
    std::mutex filesMtx;

private:
    // Images queue to be uploaded on the FTP
    std::queue<juce::Image> imagToBeSaved;

    bool showingDlgBox = false;

    const int DEFAULT_SLEEP = 1000;
    

    // Local directory in which to save the images
    juce::File imgDir{ (juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory().getChildFile("images")) };

    // Client FTP
    ftplib ftpClient;


    // FTP barcode file content
    juce::String ftpBarcodeFileContent;    

    // Reading barcode file content from FTP
    bool readBarcodeFileFromFtp()
    {
        try
        {
            ftpBarcodeFileContent = "";
            if (!ftpClient.Connect(Settings::getInstance().ftpServer.toRawUTF8()))
            {
                // Error Ftp server
                return false;
            }
            if (!ftpClient.Login(Settings::getInstance().ftpUser.toRawUTF8(), Settings::getInstance().ftpPassword.toRawUTF8()))
            {
                // Error User + Psw
                return false;
            }

            

            // Opening the file in READ ONLY mode
            auto ftpHandle = ftpClient.RawOpen(Settings::getInstance().ftpBarcodeFile.toRawUTF8(), ftplib::accesstype::fileread, ftplib::transfermode::ascii);

            if(ftpHandle == NULL)
                return false;
            
            juce::MemoryBlock mb(1024, true);

            // Reading file content                                    
            if (!ftpClient.RawRead(mb.getData(), mb.getSize(), ftpHandle))
            {
                // Error upload, closing the connection
                ftpClient.RawClose(ftpHandle);
                ftpClient.Quit();
                return false;
            }

            // We can proceed by deleting barcode file
            if (!ftpClient.Delete(Settings::getInstance().ftpBarcodeFile.toRawUTF8()))
            {
                // Error deleting, closing the connection
                ftpClient.RawClose(ftpHandle);
                ftpClient.Quit();
                return false;
            }

            // Save the string content
            ftpBarcodeFileContent = juce::String((const char*)mb.getData());
            // Close the connection
            ftpClient.RawClose(ftpHandle);
            ftpClient.Quit();
            return true;
        }
        catch (exception e)
        {
            return false;
        }
    }

public:

    ImageUploaderService() : Thread("My Background Thread") {}

    juce::String getBarcode()
    {
        return ftpBarcodeFileContent;
    }

    

    void run() override
    {
        // Connection check
        while(!Network::isConnectedToInternet())
            juce::Thread::sleep(1000);
        


        // Main loop on local images
        while (!threadShouldExit())
        {
            // Barcode file downloading
            if (ftpBarcodeFileContent == "")
            {
                if (!readBarcodeFileFromFtp())
                {

                    showingDlgBox = true;
                    const auto callback = juce::ModalCallbackFunction::create([this](int result) {
                        if (result == 0) {
                            if (juce::JUCEApplicationBase::isStandaloneApp())
                                juce::JUCEApplicationBase::quit();  // Closing current application

                        }// result == 0 means you click Cancel
                        if (result == 1) {
                            showingDlgBox = false;
                        }// result == 1 means you click OK
                        });

                    juce::AlertWindow::showOkCancelBox(juce::AlertWindow::WarningIcon,
                        "No Term file found",
                        "Try again?",
                        "OK", "Close", nullptr,
                        callback
                    );
                    while (showingDlgBox)
                    {
                        juce::Thread::sleep(DEFAULT_SLEEP);
                    }
                }
            }
                        
            try {
                // Check the connection
                if (Network::isConnectedToInternet()) {
                    // Wait till getting lock on images directory
                    if (filesMtx.try_lock()) {
                        // Check if the directory exists, release mutex and continue
                        if (!imgDir.exists() || !imgDir.isDirectory())
                        {
                            filesMtx.unlock();
                            juce::Thread::sleep(DEFAULT_SLEEP);
                            continue;
                            
                        }

                        // Initializing iterator over img files
                        juce::DirectoryIterator imgFilesIter(imgDir, false, "*.jpg");
                        std::vector<juce::File> imgFiles;

                        // Iterate over the files in the directory
                        while (imgFilesIter.next())
                        {
                            imgFiles.push_back(imgFilesIter.getFile());
                        }

                        // Check if there's any image to upload
                        if (imgFiles.empty())
                        {
                            // Nothing to do
                            filesMtx.unlock();
                            juce::Thread::sleep(DEFAULT_SLEEP);
                            continue;
                        }                       


                        // Try to upload file by file
                        try {
                            if (!ftpClient.Connect(Settings::getInstance().ftpServer.toRawUTF8()))
                            {
                                // Error Ftp server
                                filesMtx.unlock();
                                juce::Thread::sleep(DEFAULT_SLEEP);
                                continue;
                            }
                            if (!ftpClient.Login(Settings::getInstance().ftpUser.toRawUTF8(), Settings::getInstance().ftpPassword.toRawUTF8()))
                            {
                                // Error User + Psw
                                filesMtx.unlock();
                                juce::Thread::sleep(DEFAULT_SLEEP);
                                continue;
                            }
                            // Back to root folder on ftp
                            ftpClient.Chdir("/");

                            int deepLvl = 0;

                            for (int i = 0; i < imgFiles.size(); i++) {
                                deepLvl = 0;
                                try {                                    

                                    // Get barcode and field1 and field2 from file name
                                    auto s = juce::StringArray::fromTokens(imgFiles[i].getFileName(), "_", "\"");
                                    juce::String barcode = s[s.size() - 4];
                                    juce::String f2 = s[s.size() - 3];
                                    juce::String f3 = s[s.size() - 2];

                                    // Check if subdir 1 is required by Settings
                                    if (Settings::getInstance().subdir1) {
                                        // Check if the folder with the same name of the barcode already exists, otherwise let's create it
                                        if (!ftpClient.Chdir(barcode.toRawUTF8()))
                                        {
                                            ftpClient.Mkdir(barcode.toRawUTF8());
                                            ftpClient.Chdir(barcode.toRawUTF8());
                                        }
                                        deepLvl++;
                                    }

                                    // Check if subdir 2 is required by Settings
                                    if (Settings::getInstance().field2Txt.length() > 0 // Only if field 2 has been set on Settings
                                        && Settings::getInstance().subdir2)
                                    {
                                        // Check if the folder with the same name of the field2 already exists, otherwise let's create it
                                        if (!ftpClient.Chdir(f2.toRawUTF8()))
                                        {
                                            ftpClient.Mkdir(f2.toRawUTF8());
                                            ftpClient.Chdir(f2.toRawUTF8());
                                        }
                                        deepLvl++;
                                    }

                                    // Check if subdir 3 is required by Settings
                                    if (Settings::getInstance().field3Txt.length() > 0 // Only if field 3 has been set on Settings
                                        && Settings::getInstance().subdir3)
                                    {
                                        // Check if the folder with the same name of the field3 already exists, otherwise let's create it
                                        if (!ftpClient.Chdir(f3.toRawUTF8()))
                                        {
                                            ftpClient.Mkdir(f3.toRawUTF8());
                                            ftpClient.Chdir(f3.toRawUTF8());
                                        }
                                        deepLvl++;
                                    }




                                    // Begin the file uploading core


                                    // Reading i-file content
                                    auto stream = imgFiles[i].createInputStream();

                                    // Open the file on the ftserver
                                    auto ftpHandle = ftpClient.RawOpen(imgFiles[i].getFileName().toRawUTF8(), ftplib::accesstype::filewrite, ftplib::transfermode::image);

                                    juce::MemoryBlock mb;
                                    stream->readIntoMemoryBlock(mb);

                                    // Uploading image content                                 
                                    if (!ftpClient.RawWrite(mb.getData(), mb.getSize(), ftpHandle))
                                    {
                                        // Error upload
                                    }
                                    else {
                                        // Upload ok
                                        // Delete local file copy
                                        if (imgFiles[i].deleteFile())
                                        {
                                            // Delete ok
                                        }
                                        else
                                        {
                                            // Local delete error
                                        }
                                    }

                                    ftpClient.RawClose(ftpHandle);

                                    // Get back to the root folder
                                    for (int k = 0; k < deepLvl; k++)
                                    {
                                        ftpClient.Chdir("../");
                                    }

                                    

                                }
                                catch (exception e) {
                                    // if any fails, move forward with other files
                                    cout << "Exception during upload: " << e.what();
                                    // Get back to the root folder
                                    for (int k = 0; k < deepLvl; k++)
                                    {
                                        ftpClient.Chdir("../");
                                    }
                                }

                            }                           


                            // Closing FTP tunnel
                            ftpClient.Quit();
                        }
                        catch (exception e) {
                            // FTP connection error
                            cout << "Error";
                        }
                        filesMtx.unlock();
                    }
                }
                    
                // Attendo 10 secondi prima di ripartire con il while
                juce::Thread::sleep(DEFAULT_SLEEP);
                
            }
            catch (exception e)
            {
                // Errore sullo sleep
                filesMtx.unlock();
            }
            
        }
    }
};