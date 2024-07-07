/*
  ==============================================================================

    Camera.h
    Created: 7 Apr 2024 8:19:55pm
    Author:  simone.bertolini

  ==============================================================================
*/

#pragma once


#include <JuceHeader.h>
#include "opencv2/opencv.hpp"
#include "MainComponent.h"

class Camera : public juce::CameraDevice::Listener //public juce::Thread, 
{

private:
    juce::Image frameImg = { juce::Image::PixelFormat::RGB,640, 480, true };
    std::mutex frameMtx;
    //juce::Component* context;


public:

    /*Camera(juce::Component* _ctx) : Thread("My Camera Thread")
    {
        context = _ctx;
    }*/

    Camera() //:Camera(nullptr)
    {

    }

    /*void setCameraProperties(cv::VideoCapture& cap) {
        cap.set(cv::CAP_PROP_GAIN, -1); // Set gain
        cap.set(cv::CAP_PROP_BRIGHTNESS, 100); // Set brightness
        cap.set(cv::CAP_PROP_CONTRAST, 60); // Set contrast
    }*/

    void imageReceived(const juce::Image& image)
    {
        // This method is called whenever a new frame is available from the camera
        frameMtx.lock();
        copyImageData(image, frameImg);
        frameMtx.unlock();
        //cameraView = image;
        //repaint();
    }


    /*std::string getMatType(const cv::Mat& mat) {
        int type = mat.type();
        uchar depth = type & CV_MAT_DEPTH_MASK;
        uchar chans = 1 + (type >> CV_CN_SHIFT);

        std::string r;

        switch (depth) {
        case CV_8U:  r = "8U"; break;
        case CV_8S:  r = "8S"; break;
        case CV_16U: r = "16U"; break;
        case CV_16S: r = "16S"; break;
        case CV_32S: r = "32S"; break;
        case CV_32F: r = "32F"; break;
        case CV_64F: r = "64F"; break;
        default:     r = "User"; break;
        }

        r += "C";
        r += (chans + '0');

        return r;
    }*/


    /*void cvMatToJuceImage(const cv::Mat& frame) {
        int width = frame.cols;
        int height = frame.rows;

        // Create a Juce image with the same size and format as the OpenCV frame
        juce::Image juceImage(juce::Image::PixelFormat::RGB, width, height, false);

        // Copy data from OpenCV Mat to Juce Image
        for (int y = 0; y < height; ++y) {
            const cv::Vec3b* cvPixel = frame.ptr<cv::Vec3b>(y);

            for (int x = 0; x < width; ++x) {
                frameImg.setPixelAt(x, y, juce::Colour(cvPixel[x][2], cvPixel[x][1], cvPixel[x][0]));
            }
        }
    }*/


    /*void run() override
    {
        cv::VideoCapture cap(0);

        setCameraProperties(cap);
        if (!cap.isOpened()) {
            std::cerr << "Error opening camera\n";
            return;
        }

        cv::Mat frame;
        cap >> frame;
        if (frame.empty())
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                "Empty", "Empty", "ok");
        }

        double frameWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
        double frameHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        double fps = cap.get(cv::CAP_PROP_FPS);
        double brightness = cap.get(cv::CAP_PROP_BRIGHTNESS);
        double contrast = cap.get(cv::CAP_PROP_CONTRAST);
        double saturation = cap.get(cv::CAP_PROP_SATURATION);
        double hue = cap.get(cv::CAP_PROP_HUE);
        double gain = cap.get(cv::CAP_PROP_GAIN);
        double exposure = cap.get(cv::CAP_PROP_EXPOSURE);
        double whiteBalance = cap.get(cv::CAP_PROP_WHITE_BALANCE_BLUE_U);
        double format = cap.get(cv::CAP_PROP_FORMAT);
        std::string frameType = getMatType(frame);

        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
            "Start", "Camera Properties:\r\n" 
            "Frame Width: " + juce::String(frameWidth) + "\r\n" 
            "Frame Height: " + juce::String(frameHeight) + "\r\n"
            "FPS: " + juce::String(fps) + "\r\n"
            "Brightness: " + juce::String(brightness) + "\r\n"
            "Contrast: " + juce::String(contrast) + "\r\n"
            "Saturation: " + juce::String(saturation) + "\r\n"
            "Hue: " + juce::String(hue) + "\r\n"
            "Gain: " + juce::String(gain) + "\r\n"
            "Exposure: " + juce::String(exposure) + "\r\n"
            "White Balance (Blue U): " + juce::String(whiteBalance) + "\r\n"
            "Channels: " + juce::String(frame.channels()) + "\r\n"
            "Format: " + frameType + "\r\n", "ok");


        frameImg = { juce::Image::PixelFormat::RGB,frame.cols, frame.rows, true };
        while (!threadShouldExit())
        {    
            if (frameMtx.try_lock())
            {
                
                cap >> frame;
                cvMatToJuceImage(frame);

                //cv::imshow("Camera", frame);

                //jassert(frame.cols == frameImg.getWidth());
                //jassert(frame.rows == frameImg.getHeight());
                //memcpy(juce::Image::BitmapData(frameImg, juce::Image::BitmapData::ReadWriteMode::readWrite).data, frame.data, frame.cols * frame.rows * 3);
                frameMtx.unlock();
            }
            
            
            juce::Thread::sleep(20);
        }
        cap.release();

        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
            "Fine", "STOP", "ok");

    }*/

    void getImage(juce::Image* dst)
    {
        frameMtx.lock();
        copyImageData(frameImg, *dst);
        
        frameMtx.unlock();
    }


    void copyImageData(const juce::Image& sourceImage, juce::Image& destImage) {
        // Check if the source and destination images are of the same size and format
        if (sourceImage.getWidth() == destImage.getWidth() &&
            sourceImage.getHeight() == destImage.getHeight() &&
            sourceImage.getFormat() == destImage.getFormat()) {

            // Access the raw image data
            juce::Image::BitmapData sourceData(sourceImage, juce::Image::BitmapData::readOnly);
            juce::Image::BitmapData destData(destImage, juce::Image::BitmapData::writeOnly);

            // Copy pixel data
            for (int y = 0; y < sourceImage.getHeight(); ++y) {
                for (int x = 0; x < sourceImage.getWidth(); ++x) {
                    // Directly copy the pixel data
                    destData.setPixelColour(x, y, sourceData.getPixelColour(x, y));
                }
            }
        }
        else {
            destImage = { sourceImage.getFormat(), sourceImage.getWidth() ,sourceImage.getHeight(),  true};
            // Handle error: incompatible image formats or sizes
            juce::Logger::writeToLog("Error: Source and destination images are not compatible.");
        }
    }


};