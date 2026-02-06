#include "capture screen.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>

// Define ADB path here
static const char* kAdbPath = "C:\\Program Files\\Microvirt\\MEmu\\adb.exe";

// Helper: Is file valid?
bool IsFileValid(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;
    return file.tellg() > 1024; // Valid if larger than 1KB
}

// ----------------------------------------------------------------------------
// ENHANCED SCREEN CAPTURE (With Retry Mechanism)
// ----------------------------------------------------------------------------
cv::Mat CaptureAdbScreen(bool grayscale)
{
    std::string tempFile = "C:\\Users\\Public\\adb_temp_screen.png";
    int maxRetries = 3;
    cv::Mat img;

    for (int i = 0; i < maxRetries; ++i) {

        // 1. Delete old file
        remove(tempFile.c_str());

        // 2. Prepare command (Protected against quote errors)
        std::string cmd = "cmd /c \"\"" + std::string(kAdbPath) + "\" exec-out screencap -p > \"" + tempFile + "\"\"";

        // 3. Execute
        system(cmd.c_str());

        // 4. Small wait for file write
        std::this_thread::sleep_for(std::chrono::milliseconds(400));

        // 5. File check
        if (!IsFileValid(tempFile)) {
            continue;
        }

        // 6. Read image
        int flags = grayscale ? cv::IMREAD_GRAYSCALE : cv::IMREAD_COLOR;
        img = cv::imread(tempFile, flags);

        // 7. Is image valid?
        if (!img.empty()) {
            return img; // Success
        }
    }

    return cv::Mat();
}

// ----------------------------------------------------------------------------
// 1. SCREEN SCAN (FIELD FINDING) - ROI ADDED
// ----------------------------------------------------------------------------
cv::Mat screenscan(const std::string& templatePath, int& tarlakonumuX, int& tarlakonumuY) {
    // Get color image
    cv::Mat fullFrame = CaptureAdbScreen(false);

    if (fullFrame.empty()) return cv::Mat();

    // --- ROI CROP (IGNORE BOTTOM MENU) ---
    int roiHeight = (int)(fullFrame.rows * 0.80); // Top 80%
    cv::Rect searchRect(0, 0, fullFrame.cols, roiHeight);
    cv::Mat croppedFrame = fullFrame(searchRect);

    // Load template
    cv::Mat templ = cv::imread(templatePath, cv::IMREAD_COLOR);
    if (templ.empty()) {
        return fullFrame;
    }

    cv::Mat result;
    cv::matchTemplate(croppedFrame, templ, result, cv::TM_CCOEFF_NORMED);

    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    if (maxVal >= 0.70) {
        // Calculate position (No offset needed since crop starts at 0,0)
        tarlakonumuX = maxLoc.x + (templ.cols / 2);
        tarlakonumuY = maxLoc.y + (templ.rows / 2);
    }
    else {
        tarlakonumuX = -1;
        tarlakonumuY = -1;
    }

    return fullFrame;
}

// ----------------------------------------------------------------------------
// 2. WHEAT SCAN (WHEAT FINDING) - ROI ADDED
// ----------------------------------------------------------------------------
cv::Mat wheatscan(const std::string& templatePath, int& wheatkonumuX, int& wheatkonumuY) {
    cv::Mat fullFrame = CaptureAdbScreen(false);

    if (fullFrame.empty()) return cv::Mat();

    // --- ROI CROP ---
    int roiHeight = (int)(fullFrame.rows * 0.80);
    cv::Rect searchRect(0, 0, fullFrame.cols, roiHeight);
    cv::Mat croppedFrame = fullFrame(searchRect);

    cv::Mat templ = cv::imread(templatePath, cv::IMREAD_COLOR);
    if (templ.empty()) {
        return fullFrame;
    }

    cv::Mat result;
    cv::matchTemplate(croppedFrame, templ, result, cv::TM_CCOEFF_NORMED);

    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    if (maxVal >= 0.70) {
        wheatkonumuX = maxLoc.x + (templ.cols / 2);
        wheatkonumuY = maxLoc.y + (templ.rows / 2);
    }
    else {
        wheatkonumuX = -1;
        wheatkonumuY = -1;
    }

    return fullFrame;
}

// ----------------------------------------------------------------------------
// 3. SICKLE SCAN (SICKLE FINDING) - ROI ADDED
// ----------------------------------------------------------------------------
cv::Mat sicklescan(const std::string& templatePath, int& sicklekonumuX, int& sicklekonumuY) {
    cv::Mat fullFrame = CaptureAdbScreen(false);
    if (fullFrame.empty()) return cv::Mat();

    // --- ROI CROP ---
    int roiHeight = (int)(fullFrame.rows * 0.80);
    cv::Rect searchRect(0, 0, fullFrame.cols, roiHeight);
    cv::Mat croppedFrame = fullFrame(searchRect);

    cv::Mat sicktempl = cv::imread(templatePath, cv::IMREAD_COLOR);
    if (sicktempl.empty()) {
        return fullFrame;
    }

    cv::Mat result;
    cv::matchTemplate(croppedFrame, sicktempl, result, cv::TM_CCOEFF_NORMED);

    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    if (maxVal >= 0.72) {
        sicklekonumuX = maxLoc.x + (sicktempl.cols / 2);
        sicklekonumuY = maxLoc.y + (sicktempl.rows / 2);
    }
    else {
        sicklekonumuX = -1;
        sicklekonumuY = -1;
    }

    return fullFrame;
}

// ----------------------------------------------------------------------------
// 4. GROWN SCAN (GROWN WHEAT FINDING) - ROI ADDED
// ----------------------------------------------------------------------------
cv::Mat grownscan(const std::string& templatePath, int& grownkonumuX, int& grownkonumuY) {
    // 1. Capture screen (Getting color but will convert to grayscale)
    cv::Mat fullFrame = CaptureAdbScreen(false);
    if (fullFrame.empty()) return cv::Mat();

    // 2. ROI Crop
    int roiHeight = (int)(fullFrame.rows * 0.80);
    cv::Rect searchRect(0, 0, fullFrame.cols, roiHeight);
    cv::Mat croppedFrame = fullFrame(searchRect);

    // 3. Load template
    cv::Mat growntempl = cv::imread(templatePath, cv::IMREAD_COLOR);
    if (growntempl.empty()) {
        return fullFrame;
    }

    // --- CRITICAL SETTING: GRAYSCALE CONVERSION ---
    // Color matching fails on swaying wheat in the wind.
    // Grayscale matching ignores light changes, focuses on shape.
    cv::Mat grayFrame, grayTempl;
    cv::cvtColor(croppedFrame, grayFrame, cv::COLOR_BGR2GRAY);
    cv::cvtColor(growntempl, grayTempl, cv::COLOR_BGR2GRAY);

    // 4. Matching (On grayscale images)
    cv::Mat result;
    cv::matchTemplate(grayFrame, grayTempl, result, cv::TM_CCOEFF_NORMED);

    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    // Grayscale matching works perfectly at 0.65 - 0.70 range.
    // If still not found, you can set this to 0.60.
    if (maxVal >= 0.64) {
        grownkonumuX = maxLoc.x + (growntempl.cols / 2);
        grownkonumuY = maxLoc.y + (growntempl.rows / 2);
    }
    else {
        grownkonumuX = -1;
        grownkonumuY = -1;
    }

    // Return color version for display
    return fullFrame;
}

// ----------------------------------------------------------------------------
// 5. FIND TEMPLATE MATCHES (GENERAL SEARCH) - ROI ADDED
// ----------------------------------------------------------------------------
cv::Mat FindTemplateMatches(
    const std::string& templatePath,
    float threshold,
    std::vector<cv::Point>& matches,
    cv::Point& bestMatch,
    cv::Size& templSize,
    double* outBestScore
) {
    // 1. Capture color image
    cv::Mat fullFrame = CaptureAdbScreen(false);
    if (fullFrame.empty()) return cv::Mat();

    // 2. ROI Crop (Remove bottom menu)
    int roiHeight = (int)(fullFrame.rows * 0.80);
    cv::Rect searchRect(0, 0, fullFrame.cols, roiHeight);
    cv::Mat croppedFrame = fullFrame(searchRect);

    // 3. Load template
    cv::Mat templ = cv::imread(templatePath, cv::IMREAD_COLOR);
    if (templ.empty()) {
        return fullFrame;
    }

    templSize = templ.size();

    // 4. Matching
    cv::Mat result;
    cv::matchTemplate(croppedFrame, templ, result, cv::TM_CCOEFF_NORMED);

    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    // Save best match
    bestMatch = maxLoc;
    if (outBestScore) {
        *outBestScore = maxVal;
    }

    // 5. Find multiple matches
    matches.clear();
    for (int y = 0; y < result.rows; y++)
    {
        for (int x = 0; x < result.cols; x++)
        {
            if (result.at<float>(y, x) >= threshold)
            {
                // Add position found within ROI to the list.
                // No offset needed since crop starts at (0,0).
                matches.emplace_back(x, y);
            }
        }
    }

    return fullFrame; // Return original full image (for GUI display)
}

// ----------------------------------------------------------------------------
// 6. WINDOW CAPTURE (Legacy function)
// ----------------------------------------------------------------------------
cv::Mat ekranYakala(HWND hwnd) {
    RECT memuekrani;
    GetClientRect(hwnd, &memuekrani);
    int genislik = memuekrani.right - memuekrani.left;
    int yukseklik = memuekrani.bottom - memuekrani.top;
    HDC hdcWindow = GetDC(hwnd);
    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, genislik, yukseklik);
    SelectObject(hdcMemDC, hBitmap);
    BitBlt(hdcMemDC, 0, 0, genislik, yukseklik, hdcWindow, 0, 0, SRCCOPY);

    BITMAPINFOHEADER bi{};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = genislik;
    bi.biHeight = -yukseklik;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;

    cv::Mat mat(yukseklik, genislik, CV_8UC3);
    GetDIBits(hdcWindow, hBitmap, 0, yukseklik, mat.data, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

    DeleteObject(hBitmap);
    DeleteDC(hdcMemDC);
    ReleaseDC(hwnd, hdcWindow);
    return mat;
}

// ----------------------------------------------------------------------------
// 7. SHOP SCAN (SHOP ICON FINDING)
// ----------------------------------------------------------------------------
cv::Mat shopscan(const std::string& templatePath, int& shopkonumuX, int& shopkonumuY) {
    // 1. Capture screen (Color)
    cv::Mat shopFrame = CaptureAdbScreen(false);
    if (shopFrame.empty()) return cv::Mat();

    // 2. Load template
    cv::Mat shoptempl = cv::imread(templatePath, cv::IMREAD_COLOR);
    if (shoptempl.empty()) {
        return shopFrame;
    }

    // 3. Matching (On full screen)
    cv::Mat result;
    cv::matchTemplate(shopFrame, shoptempl, result, cv::TM_CCOEFF_NORMED);

    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    // Fixed icon, 0.70 - 0.72 threshold is safe.
    if (maxVal >= 0.70) {
        shopkonumuX = maxLoc.x + (shoptempl.cols / 2);
        shopkonumuY = maxLoc.y + (shoptempl.rows / 2);
    }
    else {
        shopkonumuX = -1;
        shopkonumuY = -1;
    }

    return shopFrame;
}
