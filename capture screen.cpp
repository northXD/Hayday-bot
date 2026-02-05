#include "capture screen.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <fstream>
#include <iostream>
#include <thread> // Sleep icin
#include <chrono> // Sleep icin

// Define ADB Path Here
static const char* kAdbPath = "C:\\Program Files\\Microvirt\\MEmu\\adb.exe";

// Yardýmcý: Dosya geçerli mi?
bool IsFileValid(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;
    return file.tellg() > 1024; // 1KB'dan buyukse gecerlidir
}

// ----------------------------------------------------------------------------
// GÜÇLENDÝRÝLMÝÞ EKRAN YAKALAMA (Retry Mekanizmalý)
// ----------------------------------------------------------------------------
cv::Mat CaptureAdbScreen(bool grayscale)
{
    std::string tempFile = "C:\\Users\\Public\\adb_temp_screen.png";
    int maxRetries = 3;
    cv::Mat img;

    for (int i = 0; i < maxRetries; ++i) {

        // 1. Eski dosyayý sil
        remove(tempFile.c_str());

        // 2. Komutu hazýrla (Týrnak hatalarýna karsý korumalý)
        std::string cmd = "cmd /c \"\"" + std::string(kAdbPath) + "\" exec-out screencap -p > \"" + tempFile + "\"\"";

        // 3. Çalýþtýr
        system(cmd.c_str());

        // 4. Dosya yazýmý için minik bekleme
        std::this_thread::sleep_for(std::chrono::milliseconds(400));

        // 5. Dosya kontrolü
        if (!IsFileValid(tempFile)) {
            // std::cout << "[UYARI] Ekran alinadi, tekrar deneniyor..." << std::endl;
            continue;
        }

        // 6. Resmi Oku
        int flags = grayscale ? cv::IMREAD_GRAYSCALE : cv::IMREAD_COLOR;
        img = cv::imread(tempFile, flags);

        // 7. Resim geçerli mi?
        if (!img.empty()) {
            return img; // Baþarýlý
        }
    }

    std::cout << "[HATA] Ekran yakalanamadi (Capture Failed)!" << std::endl;
    return cv::Mat();
}

// ----------------------------------------------------------------------------
// 1. SCREEN SCAN (FIELD BULMA) - ROI EKLENDÝ
// ----------------------------------------------------------------------------
cv::Mat screenscan(const std::string& templatePath, int& tarlakonumuX, int& tarlakonumuY) {
    // Renkli al
    cv::Mat fullFrame = CaptureAdbScreen(false);

    if (fullFrame.empty()) return cv::Mat();

    // --- ROI KESÝMÝ (ALT MENÜYÜ GÖRMEZDEN GEL) ---
    int roiHeight = (int)(fullFrame.rows * 0.80); // Üst %80
    cv::Rect searchRect(0, 0, fullFrame.cols, roiHeight);
    cv::Mat croppedFrame = fullFrame(searchRect);

    // Template yükle
    cv::Mat templ = cv::imread(templatePath, cv::IMREAD_COLOR);
    if (templ.empty()) {
        std::cout << "[HATA] Template yok: " << templatePath << std::endl;
        return fullFrame;
    }

    cv::Mat result;
    cv::matchTemplate(croppedFrame, templ, result, cv::TM_CCOEFF_NORMED);

    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    if (maxVal >= 0.70) {
        // Konum hesapla (Kýrpma 0,0'dan baþladýðý için ofset eklemeye gerek yok)
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
// 2. WHEAT SCAN (BUÐDAY BULMA) - ROI EKLENDÝ
// ----------------------------------------------------------------------------
cv::Mat wheatscan(const std::string& templatePath, int& wheatkonumuX, int& wheatkonumuY) {
    cv::Mat fullFrame = CaptureAdbScreen(false);

    if (fullFrame.empty()) return cv::Mat();

    // --- ROI KESÝMÝ ---
    int roiHeight = (int)(fullFrame.rows * 0.80);
    cv::Rect searchRect(0, 0, fullFrame.cols, roiHeight);
    cv::Mat croppedFrame = fullFrame(searchRect);

    cv::Mat templ = cv::imread(templatePath, cv::IMREAD_COLOR);
    if (templ.empty()) {
        std::cout << "[HATA] Template yok: " << templatePath << std::endl;
        return fullFrame;
    }

    cv::Mat result;
    cv::matchTemplate(croppedFrame, templ, result, cv::TM_CCOEFF_NORMED);

    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    // Eþik deðerini log'a yazalým ki görebilesin
    std::cout << "Wheat Score: " << maxVal << std::endl;

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
// 3. SICKLE SCAN (ORAK BULMA) - ROI EKLENDÝ
// ----------------------------------------------------------------------------
cv::Mat sicklescan(const std::string& templatePath, int& sicklekonumuX, int& sicklekonumuY) {
    std::cout << "[DEBUG] Sickle Scan..." << std::endl;

    cv::Mat fullFrame = CaptureAdbScreen(false);
    if (fullFrame.empty()) return cv::Mat();

    // --- ROI KESÝMÝ ---
    int roiHeight = (int)(fullFrame.rows * 0.80);
    cv::Rect searchRect(0, 0, fullFrame.cols, roiHeight);
    cv::Mat croppedFrame = fullFrame(searchRect);

    // Debug kaydý (Ýsteðe baðlý, çalýþýyorsa silebilirsin)
    // cv::imwrite("C:\\Users\\Public\\debug_sickle_roi.png", croppedFrame);

    cv::Mat sicktempl = cv::imread(templatePath, cv::IMREAD_COLOR);
    if (sicktempl.empty()) {
        std::cout << "[HATA] Template yok: " << templatePath << std::endl;
        return fullFrame;
    }

    cv::Mat result;
    cv::matchTemplate(croppedFrame, sicktempl, result, cv::TM_CCOEFF_NORMED);

    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    std::cout << "Sickle Score: " << maxVal << std::endl;

    if (maxVal >= 0.72) {
        sicklekonumuX = maxLoc.x + (sicktempl.cols / 2);
        sicklekonumuY = maxLoc.y + (sicktempl.rows / 2);
        std::cout << "[BULUNDU] Sickle: " << sicklekonumuX << ", " << sicklekonumuY << std::endl;
    }
    else {
        sicklekonumuX = -1;
        sicklekonumuY = -1;
    }

    return fullFrame;
}
cv::Mat grownscan(const std::string& templatePath, int& grownkonumuX, int& grownkonumuY) {
    std::cout << "[DEBUG] Grown Scan (Grayscale)..." << std::endl;

    // 1. Ekraný Yakala (Renkli alýyoruz ama griye çevireceðiz)
    cv::Mat fullFrame = CaptureAdbScreen(false);
    if (fullFrame.empty()) return cv::Mat();

    // 2. ROI Kesimi
    int roiHeight = (int)(fullFrame.rows * 0.80);
    cv::Rect searchRect(0, 0, fullFrame.cols, roiHeight);
    cv::Mat croppedFrame = fullFrame(searchRect);

    // 3. Þablonu Yükle
    cv::Mat growntempl = cv::imread(templatePath, cv::IMREAD_COLOR);
    if (growntempl.empty()) {
        std::cout << "[HATA] Template yok: " << templatePath << std::endl;
        return fullFrame;
    }

    // --- KRÝTÝK AYAR: GRÝYE ÇEVÝRME ---
    // Renkli tarama rüzgarda sallanan buðdayda hata verir.
    // Gri tarama ise ýþýk deðiþimlerini yok sayar, þekle odaklanýr.
    cv::Mat grayFrame, grayTempl;
    cv::cvtColor(croppedFrame, grayFrame, cv::COLOR_BGR2GRAY);
    cv::cvtColor(growntempl, grayTempl, cv::COLOR_BGR2GRAY);

    // 4. Eþleþtirme (Gri resimler üzerinde)
    cv::Mat result;
    cv::matchTemplate(grayFrame, grayTempl, result, cv::TM_CCOEFF_NORMED);

    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    std::cout << "Grown Score (Gray): " << maxVal << std::endl;

    // Gri taramada 0.65 - 0.70 arasý mükemmel çalýþýr.
    // Eðer yine bulamazsa burayý 0.60 yapabilirsin.
    if (maxVal >= 0.64) {
        grownkonumuX = maxLoc.x + (growntempl.cols / 2);
        grownkonumuY = maxLoc.y + (growntempl.rows / 2);
        std::cout << "[BULUNDU] Grown Wheat!" << std::endl;
    }
    else {
        grownkonumuX = -1;
        grownkonumuY = -1;
    }

    // Ekranda göstermek için renkli halini döndürüyoruz
    return fullFrame;
}
// ----------------------------------------------------------------------------
// 4. FIND TEMPLATE MATCHES (GENEL ARAMA) - ROI EKLENDÝ
// ----------------------------------------------------------------------------
cv::Mat FindTemplateMatches(
    const std::string& templatePath,
    float threshold,
    std::vector<cv::Point>& matches,
    cv::Point& bestMatch,
    cv::Size& templSize,
    double* outBestScore
) {
    // 1. Renkli yakala
    cv::Mat fullFrame = CaptureAdbScreen(false);
    if (fullFrame.empty()) return cv::Mat();

    // 2. ROI Kesimi (Alt menüyü at)
    int roiHeight = (int)(fullFrame.rows * 0.80);
    cv::Rect searchRect(0, 0, fullFrame.cols, roiHeight);
    cv::Mat croppedFrame = fullFrame(searchRect);

    // 3. Þablon yükle
    cv::Mat templ = cv::imread(templatePath, cv::IMREAD_COLOR);
    if (templ.empty()) {
        std::cout << "[HATA] Template yok: " << templatePath << std::endl;
        return fullFrame;
    }

    templSize = templ.size();

    // 4. Eþleþtirme
    cv::Mat result;
    cv::matchTemplate(croppedFrame, templ, result, cv::TM_CCOEFF_NORMED);

    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    // En iyi eþleþmeyi kaydet
    bestMatch = maxLoc;
    if (outBestScore) {
        *outBestScore = maxVal;
    }

    // 5. Çoklu eþleþmeleri bul
    matches.clear();
    for (int y = 0; y < result.rows; y++)
    {
        for (int x = 0; x < result.cols; x++)
        {
            if (result.at<float>(y, x) >= threshold)
            {
                // ROI içinde bulduðumuz konumu listeye ekle.
                // Kýrpma (0,0)'dan baþladýðý için offset eklemeye gerek yok.
                matches.emplace_back(x, y);
            }
        }
    }

    return fullFrame; // Orijinal tam resmi döndür (GUI'de göstermek için)
}

// Diðer eski window capture fonksiyonu (Gerekirse kalsýn)
cv::Mat ekranYakala(HWND hwnd) {
    // ... (Eski kodunun aynýsý kalabilir, kullanýlmýyor ama hata vermesin)
    // Yer kaplamasýn diye buraya tekrar yazmadým, eski kodundaki kalsýn veya sil.
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
cv::Mat shopscan(const std::string& templatePath, int& shopkonumuX, int& shopkonumuY) {
    std::cout << "[DEBUG] Shop Scan (Full Screen)..." << std::endl;

    // 1. Ekraný Yakala (Renkli)
    cv::Mat shopFrame = CaptureAdbScreen(false);
    if (shopFrame.empty()) return cv::Mat();


    // 2. Þablonu Yükle
    cv::Mat shoptempl = cv::imread(templatePath, cv::IMREAD_COLOR);
    if (shoptempl.empty()) {
        std::cout << "[HATA] Template yok: " << templatePath << std::endl;
        return shopFrame;
    }

    // 3. Eþleþtirme (Tam ekran üzerinde yapýlýyor)
    cv::Mat result;
    cv::matchTemplate(shopFrame, shoptempl, result, cv::TM_CCOEFF_NORMED);

    double maxVal;
    cv::Point maxLoc;
    cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    std::cout << "Shop Score: " << maxVal << std::endl;

    // Sabit ikon olduðu için 0.70 - 0.72 arasý güvenlidir.
    if (maxVal >= 0.70) {
        shopkonumuX = maxLoc.x + (shoptempl.cols / 2);
        shopkonumuY = maxLoc.y + (shoptempl.rows / 2);
        // Log mesajýný da "Sickle" yerine "Shop" olarak düzelttim
        std::cout << "[BULUNDU] Shop: " << shopkonumuX << ", " << shopkonumuY << std::endl;
    }
    else {
        shopkonumuX = -1;
        shopkonumuY = -1;
    }

    return shopFrame;

}
