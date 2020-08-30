First off, huge thanks to the CircuitMess team and shoutout to Frank Prindle (https://community.circuitmess.com/u/frankprindle) for the
first draft of this code. It is definitely a WIP, but the hope is to expand it into a full WiFi utility for Ringo.

![Screenshot](WiFiTest_screenshot_0.4.0.jpg)

**v0.4.1 - Now with Telnet Chat Server**

When the app starts, you're shown the **Main Menu**:
 * **Choose Network** - _Always start here_. You'll need to connect to your Wifi network before you do anything else.
 * **DHCP Settings** - _Not yet implemented_. This will eventually allow you to specify more network settings like IP, DNS, NTP server, etc
 * **NTP Test** - The first test added to this app, originally written by Frank Prindle.
 * **Chat Server** - A Telnet chat server for multiple clients to connect to. PuTTY works well with this (duplicate multiple sessions to see it in action). Messages sent by one client are sent to all other connected clients.
 
If you attempt to access NTP Test or Chat Server without being connected to Wifi, you will receive a prompt telling you to connect first. Pressing the B button from within a test or settings menus will take you back to the Main Menu. Pressing HOME will bring up the homePopup() menu, allowing you to adjust settings or exit the program and return to the Ringo Loader.

-----
**NTP Test Instructions:**
 * This app simply checks in with an NTP server via UDP every 10 seconds. This will allow you to test the relative stability of your wifi connection. Repeated failures to contact the NTP server could indicate a poor connection quality..
 * Press A to select a new WiFi Network
 * Press B to return to the Main Menu
-----
**Chat Server Instructions:**
 * This app will show you the current IP of the Ringo (and a reminder that telnet is port 23). Use a telnet client like PuTTY to connect. 
 * To see it actually work, you need more than one client connected at a time, as messages will not be repeated on the originating client.
 * Press B to return to the Main Menu
 * _KNOWN BUG: After you have run the Chat Server once, you'll have to reboot before it can be ran again. Connections will be refused on subsequent runs without reboot. Still sorting this out. You will be reminded to reboot if you try to run it a second time._
-----
_**Recent Changes:**_
 * Added Telnet Chat Server
 * Added a live signal strength meter to the bottom status line
 * Added back homePopup() when you press the HOME button (fixed the restart issue)
 * mp.inCall (ability to sleep/save power) is now dynamically enabled/disabled as it makes sense to (WILL sleep at general menus, will NOT sleep during a test or while connecting to WiFi)

-----
_**Future Plans:**_
 * _Addition of a settings screen so you can specify DHCP or manual IP/DNS setup, NTP server, and other future options_
 * _More tests, like Ping, Tracenet, etc...and perhaps some more creative tests as they are thought of_
  
