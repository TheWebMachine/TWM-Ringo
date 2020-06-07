First off, huge thanks to the CircuitMess team and shoutout to Frank Prindle (https://community.circuitmess.com/u/frankprindle) for the
first draft of this code. It is definitely a WIP, but the hope is to expand it into a full WiFi utility for Ringo.

Here's what we have so far:

@ On start, it searches for and displays available WiFi networks, you select and enter your password. This was taken directly from the
  Main Firmware's settingsApp. The only thing missing is the help screen for text input (just haven't sorted that minor bit out yet)
  
@ Then it will take you to Frank's *mostly* original code to test connection and packet send/receive with time.nist.gov

@ Once there, you can press B or HOME to go back to the Main Launcher. You can also press A to search for and connect to a different WiFi
  Network. After selecting a new network, it goes back to the NTP test
  

Planned changes:

@ Addition of a main menu so we can begin adding other network tests

@ Addition of a settings screen to show after wifiConnect so you can specify DHCP or manual IP/DNS setup, NTP server, and other future
  options
  
  
  
