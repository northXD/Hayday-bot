#include "Discord.h"
#include <time.h>
#include <cstring>
#include <cstdio> 
#include <chrono> // To calculate elapsed time

static const char* APPLICATION_ID = "1469448659916816548";

static int64_t StartTime;

void Discord::Initialize() {
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    Discord_Initialize(APPLICATION_ID, &handlers, 1, NULL);

    StartTime = time(0);
}

void Discord::Update(bool isEnabled) {

    Discord_RunCallbacks();
    static bool wasEnabled = true;
    if (!isEnabled) {
        if (wasEnabled) {
            
            Discord_ClearPresence();
            wasEnabled = false;
        }
        return; 
    }
    wasEnabled = true;
    
    // 2. Time For Disocrd RPC

    static auto lastUpdate = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastUpdate).count();

	if (elapsed < 2) return; // if less than 2 seconds, skip update

    lastUpdate = now; // Reset time

	//Discord Rich Presence stuff
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));

    discordPresence.state = "Running Automation";
    discordPresence.details = "Farming Wheat";

    discordPresence.startTimestamp = StartTime;

    discordPresence.largeImageKey = "rpc_logo";
    discordPresence.largeImageText = "NXRTH Hay Day Bot";

    Discord_UpdatePresence(&discordPresence);
}