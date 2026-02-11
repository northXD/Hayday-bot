# Hayday-bot
<br />A bot application with Gui &amp; written in C++ for Hay Day. 
<br />
<br /> Watch the youtube showcase video : https://youtu.be/SzghBBcKAG8
# Join my discord for feedbacks. i really need it.
<br />discord.gg/nxrth
<br />Or you can click discord server on main tab.


# How to use.
<br /> Right click installed zip file and extract to folder.
<br /> Start bot.exe
<br /> Things you must to: 
<br /> 1) Enable Root in Memu settings.
<br />2) Set your resolution to 640x480, DPI to 100.
<br />3) Run MEmu on DirectX! not OpenGL or Vulkan.
<br />4) Make sure Roadside Shop and Fields are in the same frame, or bot can't detect.
<br />5) Click Settings on the bot app, make sure MEmu and adb Path is correct(adb.exe located in the same path as MEmu. Default adb & memu path is C:\Program Files\Microvirt\MEmu).
<br />6) Always check Logs tab, if bot cannot find objects on the screen, try lowering threshold at Templates tab. If still can't find, add your own template(it's very easy, just take a screenshot and crop the image. You can check templates folder for examples).

<br />My Suggestions:

<br />Sell everything in your silo
<br />Use new account (Level 7) because your main account can get banned.
<br />Set your farm name to something thats not detectable; For example, Sunny Farm, Happy Farm, My Farm etc. etc.
# How it works (Bot logic)
<br />1)It first scans the fields using screenscan() and find fields, then clicks on best match position (x,y).
<br />2)After clicking on the field it will wait for menu animation, then finds wheat template on the screen.
<br />3)Plants wheats using adb commands (it creates a temporary txt file with adb sendevent command).
<br />4)After planting, waits 2 minutes, in that 2 minutes instead of waiting, it clicks on the roadside shop (all this done with shopscan().)and sell stuff(it first scans for empty crates, if cant then clicks on sold crates &amp; collects coins).
<br />5) When selling phase is done, it will still wait for wheats to grow, when 2 minute wait is done, it scans the grown wheats using grownscan()
<br />6) Clicks on the grown wheat, waits for animation, finds sickle using sicklescan(), then harvest with adb commands (same as i mentioned in 3rd step)
<br />7) Returns to step 1.


<br />Thank you and enjoy!.

-North.
