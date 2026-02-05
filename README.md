# Hayday-bot
A bot application with Gui &amp; written in C++ for Hay Day. 
# Join my discord for bugs / updates / suggestions.
discord.gg/nxrth

# How it works (Bot logic)
1)It first scans the fields using screenscan() and find fields, then clicks on best match position (x,y).
2)After clicking on the field it will wait for menu animation, then finds wheat template on the screen.
3)Plants wheats using adb commands (it creates a temporary txt file with adb sendevent command).
4)After planting, waits 2 minutes, in that 2 minutes instead of waiting, it clicks on the roadside shop (all this done with shopscan().)and sell stuff(it first scans for empty crates, if cant then clicks on sold crates &amp; collects coins).
5) When selling phase is done, it will still wait for wheats to grow, when 2 minute wait is done, it scans the grown wheats using grownscan()
6) Clicks on the grown wheat, waits for animation, finds sickle using sicklescan(), then harvest with adb commands (same as i mentioned in 3rd step)
7) Returns to step 1.

# How to use.

set your MEmu resolution to 640x480, 100 DPI.
Zoom out as much as you can and make sure both shop and fields are visible. (If fields are visible but shop is not, bot will break in a few minutes(this will be fixed soon)) 
Click on test buttons on the main tab and see if OpenCV is able to scan templates. If opencv can't detect objects try reducing template threshold in templates tab, if this also does not work then make your own templates(You can just take screenshots and crop them).
Also empty your silo.

# Suggestion
Sell everything in your silo
Use new account (Level 7) because your main account can get deleted.
Set your farm name to something thats not detectable; For example, Sunny Farm, Happy Farm, My Farm etc. etc.
