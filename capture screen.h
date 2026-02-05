#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <cstdlib>
#include <fstream>

// Gerekli Fonksiyonlar
cv::Mat ekranYakala(HWND hwnd);
cv::Mat CaptureAdbScreen(bool grayscale);

// Scan Fonksiyonlarý
cv::Mat screenscan(const std::string& templatePath, int& tarlakonumuX, int& tarlakonumuY);
cv::Mat wheatscan(const std::string& templatePath, int& wheatkonumuX, int& wheatkonumuY);
cv::Mat sicklescan(const std::string& templatePath, int& sicklekonumuX, int& sicklekonumuY);
cv::Mat grownscan(const std::string& templatePath, int& grownkonumuX, int& grownkonumuY);
cv::Mat shopscan(const std::string& templatePath, int& shopkonumuX, int& shopkonumuY);
// Genel Arama
cv::Mat FindTemplateMatches(
    const std::string& templatePath,
    float threshold,
    std::vector<cv::Point>& matches,
    cv::Point& bestMatch,
    cv::Size& templSize,
    double* outBestScore = nullptr
);