# 🌾 NXRTH Hayday Bot (Free & Open Source Edition)

🔥 **I have released the Multi-Bot Premium version!** You can watch the showcase video here:  
👉 [Watch Showcase](https://streamable.com/2qauwj)



## 📖 About
A fully automated farming bot application with a modern GUI, written in C++ for Hay Day. 

*This application will work with any version of Hay Day. It will remain 100% Open Source and Free Forever.*

💬 **Need Help?** Join the Discord: [discord.gg/nxrth](https://discord.gg/nxrth)
 alternative invite link = discord.gg/invite/TdjpdEtSbA
---

## ⚙️ How to Use

### 📥 Installation
1. Download the latest release `.zip` file and drag it to your desktop.
2. Right-click the `.zip` file and extract it to a folder.
3. Run `bot.exe`.

### 🛠️ Required MEmu Emulator Settings
> **⚠️ IMPORTANT:** I assume your ADB and MEmu paths, as well as the Input Device (event number), are correct in the Settings tab. If the bot logs "Planting..." but nothing happens on the screen, go to the Bot Manager tab and click **AUTO DETECT**!

* **Root:** Enable Root in MEmu settings.
* **Display:** Set resolution to **640x480**, DPI to **100**.
* **Render Mode:** Run MEmu on **DirectX** (Do NOT use OpenGL or Vulkan).
* **Game Language:** Set Hay Day language to **English**.
* **Camera Position:** Make sure the *Roadside Shop* and *Fields* are in the same frame, otherwise the bot cannot detect them.

### 🤖 Bot Configuration
* Click **Settings** on the bot app. Make sure the MEmu and ADB paths are correct. 
  *(Default ADB path is `C:\Program Files\Microvirt\MEmu\adb.exe`)*
* Always check the **Logs** tab. If the bot cannot find objects on the screen, try lowering the threshold in the **Templates** tab. If it still fails to find them, capture your own template using the **Template Maker**.

---

## 💡 Pro Suggestions
* Sell everything in your silo before starting.
* **Use a new/alt account (Level 7+).** Do not use your main account to avoid ban risks.
* Set your farm name to something natural and undetectable (e.g., *Sunny Farm, Happy Farm, John's Farm*).

---

## 🧠 How it Works (Bot Logic)
1. **Scan:** It first scans the screen for empty fields, then calculates the best match position (x, y).
2. **Open Menu:** Clicks on the field, waits for the menu animation, and finds the wheat/seed template.
3. **Plant:** Plants seeds using ADB commands.
4. **Sell (While Waiting):** Instead of idling for 2 minutes, it clicks the Roadside Shop. It scans for empty crates to sell items, or clicks on sold crates to collect coins.
5. **Harvest Scan:** Once the 2-minute growth timer finishes, it scans for grown crops.
6. **Harvest:** Clicks the grown crop, waits for the animation, finds the sickle, and harvests via ADB swipe commands.
7. **Loop:** Returns to step 1 and repeats the cycle flawlessly.

Thanks and Enjoy!  
*- North aka Nxrth*
