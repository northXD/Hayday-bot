// gui.cpp : ImGui + GLFW + OpenGL3 - Clean GUI Only

#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <string>
#include <vector>
#include <ctime>
#include <thread>
#include <chrono>
#include <atomic>
#include <random>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>

#ifdef _WIN32
#include <Windows.h>
#include <Shellapi.h> // For ShellExecute
#include <commdlg.h> // For file dialog
#pragma comment(lib, "comdlg32.lib")
#endif

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <glad/glad.h>

#define GLFW_DLL
#include <GLFW/glfw3.h>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include "capture screen.h"
#include "Discord.h"

void OpenURL(const char* url) {
#ifdef _WIN32
    ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
#endif
}

std::chrono::steady_clock::time_point g_LastAdTime;
bool g_IsFirstRun = true;
static const char* kAdbPath = "C:\\Program Files\\Microvirt\\MEmu\\adb.exe";
static const char* kMEmuExePath = "C:\\Program Files\\Microvirt\\MEmu\\MEmu.exe";

// Template thresholds
struct TemplateThresholds {
    float soilThreshold = 0.70f;
    float grownThreshold = 0.7f;
    float wheatThreshold = 0.70f;
    float sickleThreshold = 0.70f;
    float nmsThreshold = 0.3f;
    float shopThreshold = 0.80f;
    float crateThreshold = 0.80f;
    float arrowsThreshold = 0.85f;
    float plusThreshold = 0.80f;
    float crossThreshold = 0.80f;
    float advertiseThreshold = 0.72f;
    float createSaleThreshold = 0.80f;
    // --- CORN THRESHOLDS ---
    float cornThreshold = 0.70f;
    float grownCornThreshold = 0.70f;
};
TemplateThresholds g_Thresholds;

// Template paths
std::string f_templatePath = "templates\\field.png";
std::string w_templatePath = "templates\\wheat.png";
std::string s_templatePath = "templates\\sickle.png";
std::string g_templatePath = "templates\\grown.png";
std::string shop_templatePath = "templates\\shop.png";
std::string wheatshop_templatePath = "templates\\wheat_shop.png";
std::string soldcrate_templatePath = "templates\\sold_crate.png";
std::string crate_templatePath = "templates\\shop_crate.png";
std::string arrows_templatePath = "templates\\arrows.png";
std::string plus_templatePath = "templates\\plus.png";
std::string cross_templatePath = "templates\\cross.png";
std::string advertise_templatePath = "templates\\advertise.png";
std::string create_sale_templatePath = "templates\\create_sale.png";
std::string c_templatePath = "templates\\corn.png";
std::string gc_templatePath = "templates\\grown_corn.png"; // gc stands for grown corn
std::string cornshop_templatePath = "templates\\corn_shop.png";
std::string barn_market_templatePath = "templates\\barn_market.png";
std::string silo_market_templatePath = "templates\\silo_market.png";

// Template path buffers for ImGui input
char g_fieldPathBuf[260] = "templates\\field.png";
char g_wheatPathBuf[260] = "templates\\wheat.png";
char g_sicklePathBuf[260] = "templates\\sickle.png";
char g_grownPathBuf[260] = "templates\\grown.png";
char g_shopPathBuf[260] = "templates\\shop.png";
char g_wheatshopPathBuf[260] = "templates\\wheat_shop.png";
char g_soldcratePathBuf[260] = "templates\\sold_crate.png";
char g_cratePathBuf[260] = "templates\\shop_crate.png";
char g_arrowsPathBuf[260] = "templates\\arrows.png";
char g_plusPathBuf[260] = "templates\\plus.png";
char g_crossPathBuf[260] = "templates\\cross.png";
char g_advertisePathBuf[260] = "templates\\advertise.png";
char g_createSalePathBuf[260] = "templates\\create_sale.png";
char g_cornPathBuf[260] = "templates\\corn.png";
char g_grownCornPathBuf[260] = "templates\\grown_corn.png";
char g_cornShopPathBuf[260] = "templates\\corn_shop.png";
char g_barnMarketBuf[260] = "templates\\barn_market.png";
char g_siloMarketBuf[260] = "templates\\silo_market.png";

// Selected Crop 
int g_SelectedCropMode = 0;
const char* g_CropModes[] = { "Wheat (2 min)", "Corn (5 min)" };

double GetDistance(cv::Point p1, cv::Point p2) {
    return std::sqrt(std::pow(p1.x - p2.x, 2) + std::pow(p1.y - p2.y, 2));
}

std::vector<cv::Point> FilterUniqueMatches(const std::vector<cv::Point>& rawMatches, int minDist) {
    std::vector<cv::Point> uniqueMatches;
    for (const auto& pt : rawMatches) {
        bool isTooClose = false;
        for (const auto& existing : uniqueMatches) {
            if (GetDistance(pt, existing) < minDist) {
                isTooClose = true;
                break;
            }
        }
        if (!isTooClose) {
            uniqueMatches.push_back(pt);
        }
    }
    return uniqueMatches;
}

int f_konumX = 0;
int f_konumY = 0;
int w_konumX = 0;
int w_konumY = 0;
int s_konumX = 0;
int s_konumY = 0;
int g_konumX = 0;
int g_konumY = 0;

bool g_EnableDiscordRPC = true; // On by default, can be toggled off in settings
std::atomic<bool> g_BotRunning{ false };
//webhook stuff
// --- WEBHOOK & CYCLE VARIABLES ---
int g_CycleCount = 0; // Cycle counter

// Webhook Settings
bool g_EnableWebhook = false;
bool g_EnableDiscord = false;
bool g_EnableTelegram = false;
char g_DiscordWebhookBuf[512] = "";
char g_TelegramTokenBuf[128] = "";
char g_TelegramChatIdBuf[128] = "";


// ============================================================================
// FILE DIALOG HELPER
// ============================================================================
#ifdef _WIN32
std::string OpenFileDialog(const char* filter = "PNG Files\0*.png\0All Files\0*.*\0") { // Default filter for PNG files when loading a template.
    OPENFILENAMEA ofn;
    char fileName[MAX_PATH] = "";
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "png";

    if (GetOpenFileNameA(&ofn)) {
        return std::string(fileName);
    }
    return "";
}
#endif
// ============================================================================
// Start, Stop bot button management helper function.
// ============================================================================
bool SmartSleep(int milliseconds) {
    int step = 100; // Check every 100ms
    for (int i = 0; i < milliseconds; i += step) {
        if (!g_BotRunning) return false; // Stop signal received
        std::this_thread::sleep_for(std::chrono::milliseconds(step));
    }
    return true;
}

// ============================================================================
// LOG SYSTEM
// ============================================================================
std::string GetTimeStr() {
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
    return std::string(buffer);
}

struct LogEntry {
    std::string timestamp;
    std::string message;
    ImVec4 color;
};

std::vector<LogEntry> g_Logs;

void AddLog(std::string message, ImVec4 color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f)) {
    LogEntry newLog;
    newLog.timestamp = GetTimeStr();
    newLog.message = message;
    newLog.color = color;
    g_Logs.push_back(newLog);

    if (g_Logs.size() > 500) {
        g_Logs.erase(g_Logs.begin(), g_Logs.begin() + 100);
    }
}
// Check helper
bool IsBotStopped() {
    if (!g_BotRunning) {
        AddLog("Stopping...", ImVec4(1, 0.2f, 0.2f, 1));
        return true;
    }
    return false;
}
// ============================================================================
// PROCESS HELPERS
// ============================================================================
bool RunCmdHidden(const std::string& command) // this helps to run cmd on background because without this the app will start cmd window every time it needs to run an adb command. 
{
    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    std::string cmd = "cmd /c " + command;

    BOOL ok = CreateProcessA(
        nullptr,
        cmd.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    if (!ok) return false;

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}
// Helper: Send Image via Curl (Background Thread)
void SendWebhookImage(std::string imagePath, std::string caption) {
    std::string cmd = "";

    // Discord Sending
    if (g_EnableDiscord && strlen(g_DiscordWebhookBuf) > 5) {
        // Discord multipart upload
        cmd = "curl -i -H \"Content-Type: multipart/form-data\" -X POST -F \"file1=@" + imagePath + "\" \"" + std::string(g_DiscordWebhookBuf) + "\"";
        RunCmdHidden(cmd);
    }

    // Telegram Sending
    if (g_EnableTelegram && strlen(g_TelegramTokenBuf) > 5 && strlen(g_TelegramChatIdBuf) > 1) {
        // Telegram sendPhoto API
        std::string token = g_TelegramTokenBuf;
        std::string chat_id = g_TelegramChatIdBuf;
        cmd = "curl -F photo=@" + imagePath + " -F caption=\"" + caption + "\" https://api.telegram.org/bot" + token + "/sendPhoto?chat_id=" + chat_id;
        RunCmdHidden(cmd);
    }
}
void SendWebhookMessage(std::string message) {
    std::string cmd = "";

    // Discord Message
    if (g_EnableDiscord && strlen(g_DiscordWebhookBuf) > 5) {
        // Basit JSON payload
        // Tırnak işaretlerini kaçırmak için basit bir yöntem (daha karmaşık metinler için escape gerekebilir)
        std::string json = "{\"content\":\"" + message + "\"}";
        cmd = "curl -H \"Content-Type: application/json\" -X POST -d \"" + json + "\" \"" + std::string(g_DiscordWebhookBuf) + "\"";
        RunCmdHidden(cmd);
    }

    // Telegram Message
    if (g_EnableTelegram && strlen(g_TelegramTokenBuf) > 5 && strlen(g_TelegramChatIdBuf) > 1) {
        std::string token = g_TelegramTokenBuf;
        std::string chat_id = g_TelegramChatIdBuf;
        // Boşlukları + ile değiştir (Basit URL encode)
        std::replace(message.begin(), message.end(), ' ', '+');
        cmd = "curl -s -X POST https://api.telegram.org/bot" + token + "/sendMessage -d chat_id=" + chat_id + " -d text=\"" + message + "\"";
        RunCmdHidden(cmd);
    }
}
bool StartProcessDetached(const std::string& exePath)
{
    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    std::string cmd = "\"" + exePath + "\"";

    BOOL ok = CreateProcessA(
        nullptr,
        cmd.data(),
        nullptr,
        nullptr,
        FALSE,
        DETACHED_PROCESS,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    if (!ok) return false;

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

bool RunAdbCommandHidden(const std::string& args) // this runs adb commands on background without opening a cmd window every time. It uses the kAdbPath constant to locate adb.exe and runs the provided arguments. Returns true if the command executed successfully, false otherwise.
{
    std::string fullCmd = std::string("\"") + kAdbPath + "\" " + args;
    return RunCmdHidden(fullCmd);
}

void AdbTap(int x, int y) // this function helps tapping on (x,y) position on the screen.
{
    RunAdbCommandHidden("shell input tap " + std::to_string(x) + " " + std::to_string(y));
}
std::string g_InputDevice = "/dev/input/event1";
char g_InputDeviceBuf[128] = "/dev/input/event1";

// THIS FUNCTION WILL HELP YOU TO FIND CORRECT INPUT DEVICE FOR THE EMULATOR YOUR USING.
void AutoDetectTouchDevice() {
    AddLog("Detecting Touch Device...", ImVec4(1, 1, 0, 1));

    // Geçici dosya
    std::string tempFile = "C:\\Users\\Public\\devicelist.txt";
    remove(tempFile.c_str());

    // 'getevent -pl' command lists the all devices.
	// ABS_MT_POSITION_X (Multi-touch X ekseni) device is our main target to find.
    std::string cmd = "cmd /c \"\"" + std::string(kAdbPath) + "\" shell getevent -pl > \"" + tempFile + "\"\"";
    RunCmdHidden(cmd);

	std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Wait for output to be written

    std::ifstream file(tempFile);
    if (!file.is_open()) {
        AddLog("Error: Could not read device list.", ImVec4(1, 0, 0, 1));
        return;
    }

    std::string line;
    std::string currentDevice = "";
    bool found = false;

    while (std::getline(file, line)) {
        // catch "add device X: /dev/input/eventX" 
        if (line.find("add device") != std::string::npos && line.find("/dev/input/") != std::string::npos) {
            size_t startPos = line.find("/dev/input/");
            currentDevice = line.substr(startPos);
            // Clear spaces in the line if theres any
            currentDevice.erase(std::remove(currentDevice.begin(), currentDevice.end(), '\r'), currentDevice.end());
            currentDevice.erase(std::remove(currentDevice.begin(), currentDevice.end(), '\n'), currentDevice.end());
        }

        // Checks if theres "ABS_MT_POSITION_X" or "0035" (Hex code) on the cmd output.
        if (!currentDevice.empty()) {
            if (line.find("ABS_MT_POSITION_X") != std::string::npos || line.find("0035") != std::string::npos) {
                g_InputDevice = currentDevice;
                strncpy(g_InputDeviceBuf, g_InputDevice.c_str(), sizeof(g_InputDeviceBuf));
                AddLog("Touch Device Found: " + g_InputDevice, ImVec4(0, 1, 0, 1));
                found = true;
                break; // Bulduk, çıkabiliriz.
            }
        }
    }

    file.close();
    if (!found) {
        AddLog("Could not auto-detect. Using default: " + g_InputDevice, ImVec4(1, 0.5f, 0, 1));
    }
}
// ===========================================================
// SALES SECTION
// ===========================================================
void RunSalesCycle() {
    AddLog("--- Entering Sales Mode ---", ImVec4(0, 1, 1, 1));

    int shopX = -1, shopY = -1;
    bool shopFound = false;
    int maxRetries = 3;

    for (int i = 0; i < maxRetries; i++) {
        cv::Mat shopFrame = shopscan(shop_templatePath, shopX, shopY);
        if (shopX != -1) {
            AddLog("Shop Found.", ImVec4(0, 1, 0, 1));
            AdbTap(shopX, shopY);
            SmartSleep(1500);
            shopFound = true;
            break;
        }
        else {
            AddLog("Shop not found. Retrying...", ImVec4(1, 0.5f, 0, 1));
            SmartSleep(2000);
        }
    }

    if (!shopFound) {
        AddLog("ABORTING SALES: Shop could not be opened.", ImVec4(1, 0, 0, 1));
        return;
    }

    // --- WEBHOOK LOGIC (Every 2 Cycles: 1, 3, 5...) ---
    bool doWebhook = g_EnableWebhook && (g_CycleCount % 2 != 0);

    if (doWebhook) {
        // Find a crate to click (Empty or Sold) to enter the shop menu
        std::vector<cv::Point> matches;
        cv::Point bestMatch;
        cv::Size size;
        double score = 0;

        // Try finding an empty crate first
        FindTemplateMatches(crate_templatePath, 0.80f, matches, bestMatch, size, &score);

        // If no empty crate, try finding a sold crate
        if (matches.empty()) {
            FindTemplateMatches(soldcrate_templatePath, 0.80f, matches, bestMatch, size, &score);
        }

        if (!matches.empty()) {
            int crateX = matches[0].x + size.width / 2;
            int crateY = matches[0].y + size.height / 2;
            AdbTap(crateX, crateY); // Open Menu
            SmartSleep(1000);

            // Now look for BARN icon to switch tabs
            int barnX = -1, barnY = -1;
            shopscan(barn_market_templatePath, barnX, barnY);

            if (barnX != -1) {
                // Barn Found -> Click and capture
                AdbTap(barnX, barnY);
                SmartSleep(1500); // Wait for tab switch animation

                // Capture Screenshot
                cv::Mat fullScreen = CaptureAdbScreen(false);
                if (!fullScreen.empty()) {
                    // ROI Crop (Center of the screen for the menu)
                    int w = fullScreen.cols;
                    int h = fullScreen.rows;
                    int roiW = (int)(w * 0.70); // 70% width
                    int roiH = (int)(h * 0.60); // 60% height
                    int roiX = (w - roiW) / 2;
                    int roiY = (h - roiH) / 2;

                    // Safety check for ROI
                    if (roiX >= 0 && roiY >= 0 && roiX + roiW <= w && roiY + roiH <= h) {
                        cv::Rect roi(roiX, roiY, roiW, roiH);
                        cv::Mat cropped = fullScreen(roi);

                        std::string webhookImgPath = "webhook_capture.png";
                        cv::imwrite(webhookImgPath, cropped);

                        // Send Image via Webhook
                        std::string msg = "NXRTH Bot - Cycle #" + std::to_string(g_CycleCount) + " Status";
                        std::thread([=]() { SendWebhookImage(webhookImgPath, msg); }).detach();
                        AddLog("Webhook sent!", ImVec4(0, 1, 0, 1));
                    }
                }

                // Switch back to Silo to continue sales
                int siloX = -1, siloY = -1;
                shopscan(silo_market_templatePath, siloX, siloY);
                if (siloX != -1) {
                    AdbTap(siloX, siloY);
                    SmartSleep(1000);
                }
            }
            else {
                // --- BARN NOT FOUND ERROR HANDLING ---
                std::string err = "Barn_market template not found so cannot send screenshot!";
                AddLog(err, ImVec4(1, 0, 0, 1)); // Log to GUI
                // Send text message to Discord/Telegram
                std::thread([=]() { SendWebhookMessage(err); }).detach();
            }

            // Close menu to reset state for the sales loop below
            int crossX = -1, crossY = -1;
            shopscan(cross_templatePath, crossX, crossY);
            if (crossX != -1) {
                AdbTap(crossX, crossY);
                SmartSleep(1000);
            }
        }
    }
    // ------------------------------------------------------

    bool moreWheat = true;
    int salesCount = 0;

    while (moreWheat && salesCount < 10) {
        if (!g_BotRunning) break;

        std::vector<cv::Point> matches;
        cv::Point bestMatch;
        cv::Size size;
        double score = 0;

        FindTemplateMatches(crate_templatePath, 0.80f, matches, bestMatch, size, &score);

        if (matches.empty()) {
            std::vector<cv::Point> soldMatches;
            FindTemplateMatches(soldcrate_templatePath, 0.80f, soldMatches, bestMatch, size, &score);

            if (!soldMatches.empty()) {
                std::vector<cv::Point> uniqueSoldCrates = FilterUniqueMatches(soldMatches, 60);
                AddLog("Collecting " + std::to_string(uniqueSoldCrates.size()) + " crates...", ImVec4(0, 1, 0, 1));

                for (const auto& pt : uniqueSoldCrates) {
                    int cx = pt.x + size.width / 2;
                    int cy = pt.y + size.height / 2;
                    AdbTap(cx, cy);
                    SmartSleep(300);
                }
                AddLog("Coins collected. Re-scanning...", ImVec4(0, 1, 1, 1));
                SmartSleep(1000);
                continue;
            }
            else {
                AddLog("Shop is FULL (No empty or sold crates).", ImVec4(1, 0.5f, 0, 1));
                break;
            }
        }

        int crateX = matches[0].x + size.width / 2;
        int crateY = matches[0].y + size.height / 2;

        AdbTap(crateX, crateY);
        SmartSleep(800);

        std::string currentProductTemplate;
        if (g_SelectedCropMode == 0) {
            currentProductTemplate = wheatshop_templatePath;
        }
        else {
            currentProductTemplate = cornshop_templatePath;
        }
        matches.clear();
        FindTemplateMatches(currentProductTemplate, 0.80f, matches, bestMatch, size, &score);

        if (matches.empty()) {
            AddLog("No crop left. Closing sub-menu...", ImVec4(1, 0.5f, 0, 1));
            int subCrossX = -1, subCrossY = -1;
            shopscan(cross_templatePath, subCrossX, subCrossY);
            if (subCrossX != -1) {
                AdbTap(subCrossX, subCrossY);
                SmartSleep(1000);
            }
            moreWheat = false;
            break;
        }

        int prodX = matches[0].x + size.width / 2;
        int prodY = matches[0].y + size.height / 2;
        AdbTap(prodX, prodY);
        SmartSleep(300);

        matches.clear();
        FindTemplateMatches(arrows_templatePath, 0.85f, matches, bestMatch, size, &score);
        if (!matches.empty()) {
            AdbTap(matches[0].x + size.width / 2, matches[0].y + size.height / 2);
            SmartSleep(200);
        }

        matches.clear();
        FindTemplateMatches(plus_templatePath, 0.8f, matches, bestMatch, size, &score);

        std::string debugMsg = "Plus Search: Best Score=" + std::to_string(score);
        if (score > 0.1) {
            debugMsg += " at X=" + std::to_string(bestMatch.x) + " Y=" + std::to_string(bestMatch.y);
        }
        AddLog(debugMsg, score >= 0.70f ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0.5f, 0, 1));

        if (!matches.empty()) {
            std::sort(matches.begin(), matches.end(), [](const cv::Point& a, const cv::Point& b) {
                return a.y < b.y;
                });

            int pX = matches[0].x + size.width / 2;
            int pY = matches[0].y + size.height / 2;

            AddLog("Clicking Top Plus at Y=" + std::to_string(pY), ImVec4(0, 1, 0, 1));

            for (int k = 0; k < 5; k++) {
                AdbTap(pX, pY);
                SmartSleep(150);
            }
        }
        else {
            AddLog("Plus button NOT found. Check Score in log.", ImVec4(1, 0, 0, 1));
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsedMinutes = std::chrono::duration_cast<std::chrono::minutes>(now - g_LastAdTime).count();

        if (g_IsFirstRun || elapsedMinutes >= 5) {
            matches.clear();
            FindTemplateMatches(advertise_templatePath, 0.72f, matches, bestMatch, size, &score);

            if (!matches.empty()) {
                AddLog("Placing ADVERTISEMENT...", ImVec4(0, 1, 0, 1));
                AdbTap(matches[0].x + size.width / 2, matches[0].y + size.height / 2);
                g_LastAdTime = std::chrono::steady_clock::now();
                g_IsFirstRun = false;
                SmartSleep(300);
            }
            else {
                AddLog("Advertise button not found.", ImVec4(1, 0.5f, 0, 1));
            }
        }

        matches.clear();
        FindTemplateMatches(create_sale_templatePath, 0.80f, matches, bestMatch, size, &score);
        if (!matches.empty()) {
            AdbTap(matches[0].x + size.width / 2, matches[0].y + size.height / 2);
            AddLog("Item sold.", ImVec4(0, 1, 0, 1));
            salesCount++;
            SmartSleep(1200);
        }
        else {
            AddLog("Create Sale Button not found!", ImVec4(1, 0, 0, 1));
            break;
        }
    }

    int crossX = -1, crossY = -1;
    shopscan(cross_templatePath, crossX, crossY);
    if (crossX != -1) {
        AdbTap(crossX, crossY);
        AddLog("Shop closed.", ImVec4(0.7f, 0.7f, 0.7f, 1));
    }
    else {
        AddLog("Cross button not found.", ImVec4(1, 0.5f, 0, 1));
    }
}

// ============================================================================
// COMPLEX GESTURE HELPER (Dont mind its name :DD not really a complex gesture tbh)
// ============================================================================
void ExecuteComplexGesture(int startX, int startY, int screenW, int screenH) {
    std::string scriptPath = "C:\\Users\\Public\\gesture.sh";
    std::ofstream script(scriptPath);

    if (!script.is_open()) {
        AddLog("ERROR: Cannot create gesture.sh script!", ImVec4(1, 0, 0, 1));
        return;
    }

    // not stable to event1 anymore, will change after auto detect:
    std::string inputDev = g_InputDevice;
    // ------------------------------------------

    int margin = 80;

    auto writeEvent = [&](int type, int code, int value) {
        script << "sendevent " << inputDev << " " << type << " " << code << " " << value << "\n";
        };

    auto writeSync = [&]() {
        writeEvent(0, 0, 0);
        };

    // --- Start point ---
    int startY_Plus = (startY + 1 >= screenH) ? screenH - 1 : startY + 1;

    // Click 1
    writeEvent(1, 330, 1);
    writeEvent(3, 47, 0);
    writeEvent(3, 57, 100);
    writeEvent(3, 53, startX);
    writeEvent(3, 54, startY);
    writeEvent(3, 48, 5);
    writeEvent(3, 50, 5);
    writeEvent(3, 58, 15);

    // Click 2 (Multi-touch gerekliyse)
    writeEvent(3, 47, 1);
    writeEvent(3, 57, 101);
    writeEvent(3, 53, startX);
    writeEvent(3, 54, startY_Plus);
    writeEvent(3, 48, 5);
    writeEvent(3, 50, 5);
    writeEvent(3, 58, 15);

    writeSync();

    // --- Swipe Action ---
    int steps = 28;
    int leftX = margin;
    int rightX = screenW - margin;
    int bottomY = screenH - margin;

    for (int i = 0; i <= steps; ++i) {
        float t = (float)i / steps;
        int curX1 = startX + (leftX - startX) * t;
        int curY1 = startY + (bottomY - startY) * t;
        int curX2 = startX + (rightX - startX) * t;
        int curY2 = startY_Plus + (bottomY - startY_Plus) * t;

        writeEvent(3, 47, 0);
        writeEvent(3, 53, curX1);
        writeEvent(3, 54, curY1);
        writeEvent(3, 47, 1);
        writeEvent(3, 53, curX2);
        writeEvent(3, 54, curY2);
        writeSync();
    }

    // Swipe up
    int topY = margin;
    for (int i = 0; i <= steps; ++i) {
        float t = (float)i / steps;
        int curY_Up = bottomY + (topY - bottomY) * t;

        writeEvent(3, 47, 0);
        writeEvent(3, 53, leftX);
        writeEvent(3, 54, curY_Up);
        writeEvent(3, 47, 1);
        writeEvent(3, 53, rightX);
        writeEvent(3, 54, curY_Up);
        writeSync();
    }

    // IF corn is selected, swipe down once more
    if (g_SelectedCropMode == 1) {
        for (int i = 0; i <= steps; ++i) {
            float t = (float)i / steps;
            int curY_Down = topY + (bottomY - topY) * t;

            writeEvent(3, 47, 0);
            writeEvent(3, 53, leftX);
            writeEvent(3, 54, curY_Down);
            writeEvent(3, 47, 1);
            writeEvent(3, 53, rightX);
            writeEvent(3, 54, curY_Down);
            writeSync();
        }
    }

    // --- Release ---
    writeEvent(3, 47, 0);
    writeEvent(3, 57, -1);
    writeEvent(3, 47, 1);
    writeEvent(3, 57, -1);
    writeEvent(1, 330, 0);
    writeSync();
    script.close();

    RunAdbCommandHidden("push C:\\Users\\Public\\gesture.sh /data/local/tmp/");
    RunAdbCommandHidden("shell sh /data/local/tmp/gesture.sh");
}


// ============================================================================
// BOT LOGIC
// ============================================================================
bool RunPlantHarvestCycle() {

    // Initial check before doing anything
    if (!g_BotRunning) return false;

    // --- CYCLE COUNT INCREMENT ---
    g_CycleCount++;
    std::string cycleLog = "--- Starting Cycle #" + std::to_string(g_CycleCount) + " ---";
    AddLog(cycleLog, ImVec4(0, 1, 1, 1));
    // -----------------------------

    std::string seedTemplate;
    std::string grownTemplate;
    int growthTimeSeconds = 120; // 120 by default because default plant is wheat. will be set to 300 if user selects corn.

    if (g_SelectedCropMode == 0) {
        seedTemplate = w_templatePath;
        grownTemplate = g_templatePath;
        growthTimeSeconds = 120;
        AddLog("Mode: WHEAT (2 min)", ImVec4(1, 1, 0, 1));
    }
    else {
        seedTemplate = c_templatePath;
        grownTemplate = gc_templatePath;
        growthTimeSeconds = 300;
        AddLog("Mode: CORN (5 min)", ImVec4(1, 0.8f, 0.2f, 1));
    }

    int fieldX = -1, fieldY = -1;
    cv::Mat fieldFrame = screenscan(f_templatePath, fieldX, fieldY); // Finds Fields on the screen using screenscan().

    if (fieldFrame.empty()) return false; // adds field not found log if screenscan returns empty frame, this means it couldnt find any field on the screen.

    if (fieldX == -1 || fieldY == -1) {
        AddLog("Field not found. Retrying...", ImVec4(1, 0.4f, 0.4f, 1));
        return false;
    }

    // Check stop before waiting
    if (!g_BotRunning) return false;

    AdbTap(fieldX, fieldY); // taps on the found field positions and waits 1500 ms (1.5 seconds) for menu animation.
    if (!SmartSleep(1500)) return false;

    int seedX = -1, seedY = -1;
    bool seedFound = false;
    int maxRetries = 10;

    for (int i = 0; i < maxRetries; ++i) { // this function helps to retry 10 times, sometimes bot cant find objects on first run.
        if (!g_BotRunning) return false;

        wheatscan(seedTemplate, seedX, seedY); // Function name is wheatscan but its also scanning for corn seeds based on the template passed
        if (seedX != -1 && seedY != -1) {
            seedFound = true;
            break;
        }
        if (!SmartSleep(500)) return false;
    }

    if (!seedFound) {
        AddLog("Seed icon NOT found!", ImVec4(1, 0.2f, 0.2f, 1));
        return false;
    }

    if (!g_BotRunning) return false;

    AddLog("Planting...", ImVec4(0, 1, 1, 1));
    ExecuteComplexGesture(seedX, seedY, fieldFrame.cols, fieldFrame.rows); // starts planting gesture.
    if (!SmartSleep(500)) return false;

    auto plantTime = std::chrono::steady_clock::now();
    AddLog("Growth timer started (" + std::to_string(growthTimeSeconds) + "). Going to Sales...", ImVec4(0, 1, 0, 1));

    // enters sales mode and sells crops.
    if (g_BotRunning) {
        RunSalesCycle();
    }

    while (g_BotRunning) {
        auto now = std::chrono::steady_clock::now();
        auto elapsedSec = std::chrono::duration_cast<std::chrono::seconds>(now - plantTime).count(); // calculates elapsed time since planting in seconds.

        if (elapsedSec >= growthTimeSeconds) {
            AddLog("Growth time over. Ready to harvest.", ImVec4(0, 1, 0, 1));
            break;
        }
        // Using SmartSleep(1000) instead of sleep_for to allow immediate stop
        if (!SmartSleep(1000)) return false;
    }

    if (!g_BotRunning) return false;

    int grownX = -1, grownY = -1;
    bool grownFound = false;

    for (int i = 0; i < 10; ++i) { // as you can tell by reading AddLog function this scans for grown crops, if cant find retries for 10 times with 1 second intervals.
        if (!g_BotRunning) return false;

        AddLog("Checking grown crop... (" + std::to_string(i + 1) + ")", ImVec4(1, 1, 0, 1));
        grownscan(grownTemplate, grownX, grownY);
        if (grownX != -1 && grownY != -1) {
            grownFound = true;
            break;
        }
        if (!SmartSleep(1000)) return false;
    }

    // --- FALLBACK MECHANISM (tries to harvest even if can't find template) ---
    if (!grownFound) { // if you have grown crops and get this message, this is also going to break your bot because it can't find empty fields anymore because all are planted.
        if (!g_BotRunning) return false;

        AddLog("No grown crop found. Tapping the field position before planting. Please Try Grown Tests on status tab and make sure bot detects them.", ImVec4(1, 0.6f, 0.2f, 1));

        AdbTap(fieldX, fieldY); // first clicked field position, if user swipes a bit from the screen, it will tap wrong places. 
        if (!SmartSleep(1500)) return false; // Waiting for menu animation

        int sickleX = -1, sickleY = -1;
        bool sickleFound = false;

        for (int i = 0; i < 5; ++i) {
            if (!g_BotRunning) return false;
            sicklescan(s_templatePath, sickleX, sickleY);
            if (sickleX != -1 && sickleY != -1) {
                sickleFound = true;
                break;
            }
            if (!SmartSleep(1000)) return false;
        }

        if (sickleFound) {
            AddLog("Harvesting...", ImVec4(0, 1, 1, 1));
            ExecuteComplexGesture(sickleX, sickleY, fieldFrame.cols, fieldFrame.rows);
            if (!SmartSleep(1500)) return false;
        }
        else {
            AddLog("Sickle NOT found.", ImVec4(1, 0.4f, 0.4f, 1));
            return false;
        }

        AddLog("Cycle Complete (via Fallback).", ImVec4(0, 1, 1, 1));
        return true;
    }
    // --------------------------------------------------------------------------

    // Keep usual harvesting process
    if (!g_BotRunning) return false;

    AdbTap(fieldX, fieldY); // clicks on the found position 
    if (!SmartSleep(1500)) return false;

    int sickleX = -1, sickleY = -1;
    bool sickleFound = false;

    for (int i = 0; i < 5; ++i) { // scans for sickle 5 times.
        if (!g_BotRunning) return false;
        sicklescan(s_templatePath, sickleX, sickleY);
        if (sickleX != -1 && sickleY != -1) {
            sickleFound = true;
            break;
        }
        if (!SmartSleep(1000)) return false;
    }

    if (sickleFound) { // harvests using same gesture when planted.
        AddLog("Harvesting...", ImVec4(0, 1, 1, 1));
        ExecuteComplexGesture(sickleX, sickleY, fieldFrame.cols, fieldFrame.rows);
        if (!SmartSleep(1500)) return false;
    }
    else {
        AddLog("Sickle NOT found.", ImVec4(1, 0.4f, 0.4f, 1));
        return false;
    }

    AddLog("Cycle Complete.", ImVec4(0, 1, 1, 1)); // Loops.
    return true;
}

void StartBotLoop()
{
    if (g_BotRunning) {
        AddLog("Bot already running.", ImVec4(1, 0.6f, 0.2f, 1));
        return;
    }

    g_BotRunning = true;
    g_CycleCount = 0;
    std::thread([]() {
        AddLog("Bot started.", ImVec4(0, 1, 0, 1));
        while (g_BotRunning) {
            // If RunPlantHarvestCycle returns false (stopped or error), loop continues
            // but checks g_BotRunning again immediately.
            bool result = RunPlantHarvestCycle();

            if (!g_BotRunning) break; // Exit immediately if stopped

            // Small delay between cycles using SmartSleep
            SmartSleep(2000);
        }
        AddLog("Bot stopped.", ImVec4(1, 0.2f, 0.2f, 1));
        }).detach();
}

void StopBotLoop()
{
    if (!g_BotRunning) {
        AddLog("Bot is not running.", ImVec4(1, 0.6f, 0.2f, 1));
        return;
    }
    g_BotRunning = false; // This triggers SmartSleep to return false immediately
    AddLog("Stopping bot...", ImVec4(1, 1, 0, 1));
}

void StartHayDay() // this starts hay day by first launching MEmu then waits 10 seconds for it to load fully then uses adb command to launch hay day inside the emulator. You can change the wait time if your pc is slower or faster.
{
    AddLog("Starting MEmu...", ImVec4(0.6f, 0.8f, 1.0f, 1));
    StartProcessDetached(kMEmuExePath);
    std::this_thread::sleep_for(std::chrono::seconds(10));

    AddLog("Launching Hay Day via ADB monkey...", ImVec4(0.6f, 0.8f, 1.0f, 1));
    RunAdbCommandHidden("shell monkey -p com.supercell.hayday -c android.intent.category.LAUNCHER 1");
}

// ============================================================================
// THEME
// ============================================================================
void SetModernTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.28f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.20f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.24f, 0.30f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.28f, 0.35f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.35f, 0.65f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.50f, 0.90f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.50f, 0.90f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.20f, 0.25f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
}

// ============================================================================
// HELPER: Template Input Row with Load Button
// ============================================================================
void RenderTemplateRow(const char* label, char* buffer, size_t bufferSize, std::string& targetPath, float* threshold = nullptr) {
    ImGui::PushID(label);

    // Calculate widths
    float availWidth = ImGui::GetContentRegionAvail().x;
    float loadButtonWidth = 60.0f;
    float thresholdWidth = threshold ? 150.0f : 0.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float inputWidth = availWidth - loadButtonWidth - thresholdWidth - spacing * (threshold ? 2 : 1);

    // Label
    ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "%s", label);

    // Input field
    ImGui::SetNextItemWidth(inputWidth);
    if (ImGui::InputText("##path", buffer, bufferSize)) {
        targetPath = buffer;
    }

    ImGui::SameLine();

    // Load button
    if (ImGui::Button("Load", ImVec2(loadButtonWidth, 0))) {
#ifdef _WIN32
        std::string filePath = OpenFileDialog();
        if (!filePath.empty()) {
            strncpy(buffer, filePath.c_str(), bufferSize - 1);
            buffer[bufferSize - 1] = '\0';
            targetPath = filePath;
            AddLog("Loaded template: " + filePath, ImVec4(0, 1, 0, 1));
        }
#endif
    }

    // Optional threshold slider
    if (threshold) {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(thresholdWidth);
        ImGui::SliderFloat("##threshold", threshold, 0.1f, 0.99f, "%.2f");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Detection threshold for %s", label);
        }
    }

    ImGui::PopID();
}

// ============================================================================
// USER INTERFACE
// ============================================================================
void RenderUI() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGui::Begin("BotMainUI", nullptr, window_flags);

    // Sync buffers to strings
    f_templatePath = g_fieldPathBuf;
    w_templatePath = g_wheatPathBuf;
    s_templatePath = g_sicklePathBuf;
    g_templatePath = g_grownPathBuf;
    shop_templatePath = g_shopPathBuf;
    wheatshop_templatePath = g_wheatshopPathBuf;
    soldcrate_templatePath = g_soldcratePathBuf;
    crate_templatePath = g_cratePathBuf;
    arrows_templatePath = g_arrowsPathBuf;
    plus_templatePath = g_plusPathBuf;
    cross_templatePath = g_crossPathBuf;
    advertise_templatePath = g_advertisePathBuf;
    create_sale_templatePath = g_createSalePathBuf;
    // Corn Paths
    c_templatePath = g_cornPathBuf;
    gc_templatePath = g_grownCornPathBuf;
    cornshop_templatePath = g_cornShopPathBuf;

    // Header with status indicator
    ImGui::BeginChild("Header", ImVec2(0, 70), true);
    {
        ImGui::SetCursorPosY(10);
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "HAY DAY BOT");
        ImGui::SameLine(ImGui::GetWindowWidth() - 200);
        ImGui::Spacing();

        if (g_BotRunning) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: RUNNING");
        }
        else {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Status: IDLE");
        }
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "This app is %%100 Free and open source; if you paid for this, you have been scammed.");
    }
    ImGui::EndChild();

    ImGui::Spacing();

    if (ImGui::BeginTabBar("MainTabBar")) {
        // TAB 1: STATUS
        if (ImGui::BeginTabItem("Status")) {
            ImGui::Spacing();
            
            // --- GENERAL TESTS ---
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "GENERAL TESTS:");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Columns(3, "GenTests", false);
            
            // FIELD TEST
            if (ImGui::Button("Field Test", ImVec2(100, 40))) {
                cv::Mat resultFrame = screenscan(f_templatePath, f_konumX, f_konumY);
                if (!resultFrame.empty() && f_konumX != -1) {
                    AdbTap(f_konumX, f_konumY); // Tap
                    AddLog("Field Found! Tap at: " + std::to_string(f_konumX) + ", " + std::to_string(f_konumY), ImVec4(0, 1, 0, 1));
                } else AddLog("Field NOT Found", ImVec4(1, 0, 0, 1));
            }
            ImGui::NextColumn();

            // SICKLE TEST
            if (ImGui::Button("Sickle Test", ImVec2(100, 40))) {
                int sx = -1, sy = -1;
                sicklescan(s_templatePath, sx, sy);
                if (sx != -1) {
                    AdbTap(sx, sy); // Tap
                    AddLog("Sickle Found! Tap at: " + std::to_string(sx) + ", " + std::to_string(sy), ImVec4(0, 1, 0, 1));
                } else AddLog("Sickle NOT Found", ImVec4(1, 0, 0, 1));
            }
            ImGui::NextColumn();

            // SHOP TEST
            if (ImGui::Button("Shop Test", ImVec2(100, 40))) {
                int sx = -1, sy = -1;
                shopscan(shop_templatePath, sx, sy);
                if (sx != -1) {
                    AdbTap(sx, sy); // Tap
                    AddLog("Shop Found! Tap at: " + std::to_string(sx) + ", " + std::to_string(sy), ImVec4(0, 1, 0, 1));
                } else AddLog("Shop NOT Found", ImVec4(1, 0, 0, 1));
            }
            ImGui::NextColumn();
            ImGui::Columns(1); // Reset

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            // --- CROP TESTS ---
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "CROP TESTS (Uses Thresholds):");
            ImGui::Separator();
            ImGui::Spacing();
            
            ImGui::Columns(2, "CropTests", false);

            // WHEAT COLUMN
            ImGui::Text("WHEAT");
            if (ImGui::Button("Wheat Seed", ImVec2(140, 40))) {
                std::vector<cv::Point> m; cv::Point b; cv::Size s; double sc=0;
                FindTemplateMatches(w_templatePath, g_Thresholds.wheatThreshold, m, b, s, &sc);
                
                if(!m.empty()){
                     cv::Point p = m[0] + cv::Point(s.width/2, s.height/2);
                     AdbTap(p.x, p.y); // Tap
                     AddLog("Wheat Seed Found! Score: " + std::to_string(sc) + " Tap at: " + std::to_string(p.x) + "," + std::to_string(p.y), ImVec4(0,1,0,1));
                } else AddLog("Wheat Seed NOT Found (Score: " + std::to_string(sc) + ")", ImVec4(1,0,0,1));
            }

            if (ImGui::Button("Grown Wheat", ImVec2(140, 40))) {
                std::vector<cv::Point> m; cv::Point b; cv::Size s; double sc=0;
                FindTemplateMatches(g_templatePath, g_Thresholds.grownThreshold, m, b, s, &sc);
                
                if(!m.empty()){
                     cv::Point p = m[0] + cv::Point(s.width/2, s.height/2);
                     AdbTap(p.x, p.y); // Tap
                     AddLog("Grown Wheat Found! Score: " + std::to_string(sc) + " Tap at: " + std::to_string(p.x) + "," + std::to_string(p.y), ImVec4(0,1,0,1));
                } else AddLog("Grown Wheat NOT Found (Score: " + std::to_string(sc) + ")", ImVec4(1,0,0,1));
            }

            ImGui::NextColumn();

            // CORN COLUMN
            ImGui::Text("CORN");
            if (ImGui::Button("Corn Seed", ImVec2(140, 40))) {
                std::vector<cv::Point> m; cv::Point b; cv::Size s; double sc=0;
                FindTemplateMatches(c_templatePath, g_Thresholds.cornThreshold, m, b, s, &sc);
                
                if(!m.empty()){
                     cv::Point p = m[0] + cv::Point(s.width/2, s.height/2);
                     AdbTap(p.x, p.y); // Tap
                     AddLog("Corn Seed Found! Score: " + std::to_string(sc) + " Tap at: " + std::to_string(p.x) + "," + std::to_string(p.y), ImVec4(0,1,0,1));
                } else AddLog("Corn Seed NOT Found (Score: " + std::to_string(sc) + ")", ImVec4(1,0,0,1));
            }

            if (ImGui::Button("Grown Corn", ImVec2(140, 40))) {
                std::vector<cv::Point> m; cv::Point b; cv::Size s; double sc=0;
                FindTemplateMatches(gc_templatePath, g_Thresholds.grownCornThreshold, m, b, s, &sc);
                
                if(!m.empty()){
                     cv::Point p = m[0] + cv::Point(s.width/2, s.height/2);
                     AdbTap(p.x, p.y); // Tap
                     AddLog("Grown Corn Found! Score: " + std::to_string(sc) + " Tap at: " + std::to_string(p.x) + "," + std::to_string(p.y), ImVec4(0,1,0,1));
                } else AddLog("Grown Corn NOT Found (Score: " + std::to_string(sc) + ")", ImVec4(1,0,0,1));
            }
            ImGui::NextColumn();
            ImGui::Columns(1); // Reset

            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

            // --- SALES ---
            if (ImGui::Button("Start Sales Cycle", ImVec2(200, 45))) {
                std::thread([]() { RunSalesCycle(); }).detach();
            }

            ImGui::EndTabItem();
        }

        // TAB 2: AUTOMATION
        if (ImGui::BeginTabItem("Automation")) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "MAIN CONTROLS:");
            ImGui::Separator();
            ImGui::Spacing();

            // Start Hay Day button
            if (ImGui::Button("Start Hay Day", ImVec2(200, 50))) {
                StartHayDay();
            }

            ImGui::Spacing();
            ImGui::Spacing();

            // Toggle Start/Stop Bot button with color indication
            if (g_BotRunning) {
                // Red stop button when running
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));

                if (ImGui::Button("STOP BOT", ImVec2(200, 50))) {
                    StopBotLoop();
                }

                ImGui::PopStyleColor(3);
            }
            else {
                // Green start button when stopped
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.5f, 0.15f, 1.0f));

                if (ImGui::Button("START BOT", ImVec2(200, 50))) {
                    StartBotLoop();
                }

                ImGui::PopStyleColor(3);
            }

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();

            // --- PLANT MODE COMBO BOX ---
            ImGui::Text("Select Crop Mode:");
			// Dropdown width 
            ImGui::SetNextItemWidth(200);
			// Combo Box Creation
            ImGui::Combo("##CropMode", &g_SelectedCropMode, g_CropModes, IM_ARRAYSIZE(g_CropModes));

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Bot statistics panel
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "BOT INFO:");
            ImGui::Spacing();

            ImGui::BeginChild("BotInfo", ImVec2(0, 100), true);
            {
                ImGui::Text("Bot Status: %s", g_BotRunning ? "Running" : "Stopped");
                ImGui::Text("First Run Ad: %s", g_IsFirstRun ? "Pending" : "Done");

                if (!g_IsFirstRun) {
                    auto now = std::chrono::steady_clock::now();
                    auto elapsedMin = std::chrono::duration_cast<std::chrono::minutes>(now - g_LastAdTime).count();
                    ImGui::Text("Time since last ad: %lld min", elapsedMin);
                }
            }
            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        // TAB 3: TEMPLATES (Completely redesigned)
        if (ImGui::BeginTabItem("Templates")) {
            ImGui::Spacing();

            // Scrollable area for all templates
            ImGui::BeginChild("TemplatesScroll", ImVec2(0, 0), false);

            // ===== FARMING TEMPLATES =====
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "FARMING TEMPLATES");
            ImGui::Separator();
            ImGui::Spacing();

            RenderTemplateRow("Field", g_fieldPathBuf, IM_ARRAYSIZE(g_fieldPathBuf), f_templatePath, &g_Thresholds.soilThreshold);
            ImGui::Spacing();

            RenderTemplateRow("Wheat", g_wheatPathBuf, IM_ARRAYSIZE(g_wheatPathBuf), w_templatePath, &g_Thresholds.wheatThreshold);
            ImGui::Spacing();

            RenderTemplateRow("Sickle", g_sicklePathBuf, IM_ARRAYSIZE(g_sicklePathBuf), s_templatePath, &g_Thresholds.sickleThreshold);
            ImGui::Spacing();

            RenderTemplateRow("Grown Wheat", g_grownPathBuf, IM_ARRAYSIZE(g_grownPathBuf), g_templatePath, &g_Thresholds.grownThreshold);
            ImGui::Spacing();

            // --- ADDED CORN ---
            RenderTemplateRow("Corn Seed", g_cornPathBuf, IM_ARRAYSIZE(g_cornPathBuf), c_templatePath, &g_Thresholds.cornThreshold);
            ImGui::Spacing();

            RenderTemplateRow("Grown Corn", g_grownCornPathBuf, IM_ARRAYSIZE(g_grownCornPathBuf), gc_templatePath, &g_Thresholds.grownCornThreshold);
            ImGui::Spacing();
            // --------------------

            ImGui::Spacing();
            ImGui::Spacing();

            // ===== SHOP TEMPLATES =====
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "SHOP TEMPLATES");
            ImGui::Separator();
            ImGui::Spacing();

            RenderTemplateRow("Shop", g_shopPathBuf, IM_ARRAYSIZE(g_shopPathBuf), shop_templatePath, &g_Thresholds.shopThreshold);
            ImGui::Spacing();

            RenderTemplateRow("Wheat Shop", g_wheatshopPathBuf, IM_ARRAYSIZE(g_wheatshopPathBuf), wheatshop_templatePath);
            ImGui::Spacing();

            RenderTemplateRow("Corn Shop", g_cornShopPathBuf, IM_ARRAYSIZE(g_cornShopPathBuf), cornshop_templatePath);
            ImGui::Spacing();

            RenderTemplateRow("Crate", g_cratePathBuf, IM_ARRAYSIZE(g_cratePathBuf), crate_templatePath, &g_Thresholds.crateThreshold);
            ImGui::Spacing();

            RenderTemplateRow("Sold Crate", g_soldcratePathBuf, IM_ARRAYSIZE(g_soldcratePathBuf), soldcrate_templatePath);

            ImGui::Spacing();
            ImGui::Spacing();

            // ===== UI ELEMENT TEMPLATES =====
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "UI ELEMENT TEMPLATES");
            ImGui::Separator();
            ImGui::Spacing();

            RenderTemplateRow("Arrows", g_arrowsPathBuf, IM_ARRAYSIZE(g_arrowsPathBuf), arrows_templatePath, &g_Thresholds.arrowsThreshold);
            ImGui::Spacing();

            RenderTemplateRow("Plus", g_plusPathBuf, IM_ARRAYSIZE(g_plusPathBuf), plus_templatePath, &g_Thresholds.plusThreshold);
            ImGui::Spacing();

            RenderTemplateRow("Cross", g_crossPathBuf, IM_ARRAYSIZE(g_crossPathBuf), cross_templatePath, &g_Thresholds.crossThreshold);
            ImGui::Spacing();

            RenderTemplateRow("Advertise", g_advertisePathBuf, IM_ARRAYSIZE(g_advertisePathBuf), advertise_templatePath, &g_Thresholds.advertiseThreshold);
            ImGui::Spacing();

            RenderTemplateRow("Create Sale", g_createSalePathBuf, IM_ARRAYSIZE(g_createSalePathBuf), create_sale_templatePath, &g_Thresholds.createSaleThreshold);

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Global threshold settings
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "GLOBAL SETTINGS");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::SliderFloat("NMS Threshold", &g_Thresholds.nmsThreshold, 0.1f, 0.99f, "%.2f");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Non-Maximum Suppression threshold for filtering overlapping detections");
            }

            ImGui::Spacing();
            ImGui::Spacing();

            // Batch operations
            ImGui::Columns(2, "BatchOps", false);

            if (ImGui::Button("Reset All to Defaults", ImVec2(180, 35))) {
                g_Thresholds = TemplateThresholds(); // Reset to defaults
                AddLog("All thresholds reset to defaults.", ImVec4(1, 1, 0, 1));
            }

            ImGui::NextColumn();

            if (ImGui::Button("Load All Templates", ImVec2(180, 35))) {
                AddLog("Use individual Load buttons to select templates.", ImVec4(0.6f, 0.8f, 1.0f, 1));
            }

            ImGui::Columns(1);

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        // TAB 4: MAKE NEW TEMPLATE
        if (ImGui::BeginTabItem("Make new template")) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "CREATE YOUR OWN TEMPLATES");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextWrapped("If the bot doesn't click correctly or not finding objects, use this tool.");
            ImGui::TextWrapped("-----Watch my youtube video if you don't understand.-----");
            ImGui::TextWrapped("1. Click the button below to save a screenshot.");
            ImGui::TextWrapped("2. Open the screenshot in Paint, crop the item, and overwrite the file in 'templates' folder.");

            ImGui::Spacing();
            ImGui::Spacing();

			// static variable for status message 
            static std::string screenshotStatus = "Ready.";
            static ImVec4 statusColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

            // Screenshot Button
            if (ImGui::Button("TAKE SCREENSHOT (Save to /templates) ", ImVec2(280, 70))) {

                // doing with thread so ui doesnt get stuck
                std::thread([&]() {
                    screenshotStatus = "Capturing...";
                    statusColor = ImVec4(1, 1, 0, 1);

                    // 1. Take screenshot (Color)
                    cv::Mat rawScreen = CaptureAdbScreen(false);

                    if (rawScreen.empty()) {
                        screenshotStatus = "Error: Could not capture screen!";
                        statusColor = ImVec4(1, 0, 0, 1);
                        AddLog("Screenshot failed: Empty frame.", ImVec4(1, 0, 0, 1));
                        return;
                    }

                    // 2. Take current time (HHMMSS format)
                    time_t now = time(0);
                    struct tm tstruct;
                    char timeBuf[80];
                    localtime_s(&tstruct, &now);
                    strftime(timeBuf, sizeof(timeBuf), "%H%M%S", &tstruct);

                    // 3. Dosya yolunu oluştur: templates/screenshot_132634.png
                    std::string filename = "screenshot_" + std::string(timeBuf) + ".png";
                    std::string fullPath = "templates\\" + filename;

                    // 4. Kaydet
                    try {
                        bool success = cv::imwrite(fullPath, rawScreen);
                        if (success) {
                            screenshotStatus = "Saved: " + fullPath;
                            statusColor = ImVec4(0, 1, 0, 1);
                            AddLog("Screenshot saved: " + fullPath, ImVec4(0, 1, 0, 1));
                        }
                        else {
                            screenshotStatus = "Error: Could not write file (Check folder permissions)";
                            statusColor = ImVec4(1, 0, 0, 1);
                        }
                    }
                    catch (...) {
                        screenshotStatus = "Exception: File write error.";
                        statusColor = ImVec4(1, 0, 0, 1);
                    }

                    }).detach();
            }

            ImGui::Spacing();
            ImGui::TextColored(statusColor, "%s", screenshotStatus.c_str());
            ImGui::Spacing();
            ImGui::Spacing();
			ImGui::Separator();
            if (ImGui::Button("Youtube Video", ImVec2(100, 50))) {
				OpenURL("https://youtu.be/2gfU65gIaEE");
            }
            ImGui::EndTabItem();
        }
        // TAB: WEBHOOK (Logs sekmesinden önceye veya sonraya koyabilirsin)
        if (ImGui::BeginTabItem("Webhook")) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "REMOTE MONITORING (Every 2 Cycles)");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Checkbox("Enable Webhook System", &g_EnableWebhook);
            ImGui::Spacing();

            if (g_EnableWebhook) {
                ImGui::Indent();

                // DISCORD
                ImGui::Checkbox("Enable Discord", &g_EnableDiscord);
                if (g_EnableDiscord) {
                    ImGui::Text("Webhook URL:");
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 20);
                    ImGui::InputText("##discordurl", g_DiscordWebhookBuf, IM_ARRAYSIZE(g_DiscordWebhookBuf));
                }

                ImGui::Spacing();

                // TELEGRAM
                ImGui::Checkbox("Enable Telegram", &g_EnableTelegram);
                if (g_EnableTelegram) {
                    ImGui::Text("Bot Token:");
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 20);
                    ImGui::InputText("##tgtoken", g_TelegramTokenBuf, IM_ARRAYSIZE(g_TelegramTokenBuf));

                    ImGui::Text("Chat ID:");
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 20);
                    ImGui::InputText("##tgchatid", g_TelegramChatIdBuf, IM_ARRAYSIZE(g_TelegramChatIdBuf));
                }

                ImGui::Unindent();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextWrapped("The bot will take a screenshot of your Barn (Storage) every 2nd cycle and send it to you.");

            ImGui::EndTabItem();
        }
        // TAB 5: LOGS
        if (ImGui::BeginTabItem("Logs")) {
            // Log controls
            ImGui::Columns(3, "LogControls", false);

            if (ImGui::Button("Clear Logs", ImVec2(100, 30))) {
                g_Logs.clear();
            }

            ImGui::NextColumn();

            static bool autoScroll = true;
            ImGui::Checkbox("Auto-scroll", &autoScroll);

            ImGui::NextColumn();

            ImGui::Text("Entries: %zu", g_Logs.size());

            ImGui::Columns(1);

            ImGui::Separator();
            ImGui::BeginChild("LogArea", ImVec2(0, 0), true);

            for (const auto& log : g_Logs) {
                ImGui::TextColored(log.color, "[%s] %s", log.timestamp.c_str(), log.message.c_str());
            }

            if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20) {
                ImGui::SetScrollHereY(1.0f);
            }

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        // TAB 6: SETTINGS (New tab for additional options)
        if (ImGui::BeginTabItem("Settings")) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "APPLICATION SETTINGS");
            ImGui::Separator();
            ImGui::Spacing();

            // ADB Path
            static char adbPathBuf[260];
            static bool adbInitialized = false;
            if (!adbInitialized) {
                strncpy(adbPathBuf, kAdbPath, sizeof(adbPathBuf));
                adbInitialized = true;
            }

            ImGui::Text("ADB Path:");
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 70);
            ImGui::InputText("##adbpath", adbPathBuf, IM_ARRAYSIZE(adbPathBuf));
            ImGui::SameLine();
            if (ImGui::Button("Browse##adb", ImVec2(60, 0))) {
#ifdef _WIN32
                std::string path = OpenFileDialog("Executable\0*.exe\0All Files\0*.*\0");
                if (!path.empty()) {
                    strncpy(adbPathBuf, path.c_str(), sizeof(adbPathBuf) - 1);
                }
#endif
            }

            ImGui::Spacing();

            // MEmu Path
            static char memuPathBuf[260];
            static bool memuInitialized = false;
            if (!memuInitialized) {
                strncpy(memuPathBuf, kMEmuExePath, sizeof(memuPathBuf));
                memuInitialized = true;
            }

            ImGui::Text("MEmu Path:");
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 70);
            ImGui::InputText("##memupath", memuPathBuf, IM_ARRAYSIZE(memuPathBuf));
            ImGui::SameLine();
            if (ImGui::Button("Browse##memu", ImVec2(60, 0))) {
#ifdef _WIN32
                std::string path = OpenFileDialog("Executable\0*.exe\0All Files\0*.*\0");
                if (!path.empty()) {
                    strncpy(memuPathBuf, path.c_str(), sizeof(memuPathBuf) - 1);
                }
#endif
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Checkbox("Enable Discord RPC", &g_EnableDiscordRPC);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Show bot status on your Discord profile");
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // --- INPUT DEVICE SETTINGS ---
            ImGui::Text("Input Device (Touchscreen):");
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 140);
            if (ImGui::InputText("##inputdev", g_InputDeviceBuf, IM_ARRAYSIZE(g_InputDeviceBuf))) {
                g_InputDevice = g_InputDeviceBuf;
            }
            ImGui::SameLine();

            // AUTO DETECT BUTTON
            if (ImGui::Button("Auto Detect", ImVec2(130, 0))) {
                std::thread([]() { AutoDetectTouchDevice(); }).detach();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Attempts to find the correct /dev/input/eventX for touch controls");
            }
            // -----------------------------
            ImGui::Spacing();
			ImGui::Separator();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // --- ADB CONNECTION HELPERS ---
            ImGui::Text("ADB Connection Helper:");
            ImGui::Spacing();

			// BlueStacks uses port 5555 most of the time, Nox also uses 5555 by default
            if (ImGui::Button("Connect BlueStacks / Nox (Port 5555)", ImVec2(220, 30))) {
                std::thread([]() {
                    // Restart server first
                    RunCmdHidden("\"" + std::string(kAdbPath) + "\" kill-server");
					// Try connecting to localhost:5555
                    RunCmdHidden("\"" + std::string(kAdbPath) + "\" connect 127.0.0.1:5555");
                    AddLog("Trying to connect to localhost:5555...", ImVec4(1, 1, 0, 1));
                    }).detach();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Connects to standard BlueStacks/Nox port");

            ImGui::SameLine();

            // MEmu Using port 21503 most of the time (Alternative)
            if (ImGui::Button("Connect MEmu (Default)", ImVec2(180, 30))) {
                std::thread([]() {
                    RunCmdHidden("\"" + std::string(kAdbPath) + "\" connect 127.0.0.1:21503");
                    AddLog("Trying to connect to MEmu...", ImVec4(1, 1, 0, 1));
                    }).detach();
            }
			ImGui::Spacing();
            if (ImGui::Button("Connect MuMu (Port 7555)", ImVec2(220, 30))) {
                std::thread([]() {
                    // MuMu bazen bağlantıyı koparabiliyor, önce kill-server yapmak iyi olur
                    RunCmdHidden("\"" + std::string(kAdbPath) + "\" kill-server");
                    // MuMu Portuna Bağlan
                    RunCmdHidden("\"" + std::string(kAdbPath) + "\" connect 127.0.0.1:7555");
                    AddLog("Trying to connect to MuMu (7555)...", ImVec4(1, 1, 0, 1));
                    }).detach();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Connects to MuMu Player default port");

            ImGui::SameLine();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "ABOUT");
            ImGui::Text("Hay Day Bot v1.2.4 - Now supports many emulators");
            ImGui::Text("Built with ImGui + OpenGL3 + GLFW");
			ImGui::Text("-Made by North.");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
// ============================================================================
// MAIN
// ============================================================================
int main() {
    if (!glfwInit()) {
        return 1;
    }
    Discord::Initialize();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(950, 750, "Hay Day Bot [discord.gg/nxrth]", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    SetModernTheme();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    AddLog("=== Hay Day Bot ===", ImVec4(0, 1, 1, 1));
    AddLog("Please Join My Discord and Give me Feedbacks.", ImVec4(1, 1, 0, 1));

    while (!glfwWindowShouldClose(window)) {
        Discord::Update(g_EnableDiscordRPC);
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        RenderUI();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(0.11f, 0.12f, 0.15f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
