
// Programming example: connecting to a WiFi network and periodically exchanging UDP datagrams with the
// NTP server at time.nist.gov (port 123). UDP is an unreliable protocol in that no UDP datagram is guaranteed
// to reach its destination, so not every request for current time will be honored. Furthermore time.nist.gov
// does not allow requests less than 4 seconds apart. So this program requests the time approximately every
// 10 seconds and extrapolates the time after each 10-second request by using the jiffy clock function millis().

// This program is intended to allow debugging WiFi connection problems including DNS failures. See the highlited
// code below where the user may customize this program to use either the DHCP-supplied DNS server or a known
// good DNS server, and to attempt to connect to a NIST NTP server either by name or by IP address. If the
// connection by name fails to resolve the name, then the DNS server in use is failing.

// If the WiFi connection to the specified SSID with the specified password succeeds, and the NTP server name
// resolves, the program will display the date/time returned by the server. If datagrams get lost, there may be
// momentary pauses in updating the date/time while the UDP request is retried; occasional datagram loss is
// expected as such is the nature of UDP.

// See highlighted comments below to customize IP/DNS settings.

// Example originally coded 5/10/2020 by Frank Prindle.
// Additional code added by TheWebMachine 6/6/2020 onward (most of which sourced from https://github.com/CircuitMess/)

#include "MAKERphone.h"
MAKERphone mp;

void setup()
{
  Serial.begin(115200);
  mp.begin(1);
  mp.inCall=1;  // We need to disable the sleep timer. If allowed to engage sleep, device will reboot upon "wake". Screen doesn't actually turn off, tho.
  mp.display.setTextColor(TFT_BLACK);
  mp.display.setTextSize(1);
  mp.display.setTextFont(2);
  mp.display.drawRect(4, 49, 152, 28, TFT_BLACK);
  mp.display.drawRect(3, 48, 154, 30, TFT_BLACK);
  mp.display.fillRect(5, 50, 150, 26, 0xFD29);
  mp.display.setCursor(47, 54);
  mp.display.printCenter("Searching for networks");
  Serial.println("Searching for networks");
  while(!mp.update());
  wifiConnect();
  /*-------------------------------------------------------------------*/
  /* Disable the following line to use DHCP supplied DNS server.       */
  /* Enable the following line to use a well-known DNS server.         */
  /*    Set the first IP address to a valid static IP on your network. */
  /*    Set the second IP address to the IP address of your router.    */
  /*    Set the third IP address to your network mask.                 */
  /*    Leave the fourth IP address alone (Google DNS).                */
  /*-------------------------------------------------------------------*/
  //WiFi.config(IPAddress(192,168,1,177),IPAddress(192,168,1,1),IPAddress(255,255,255,0),IPAddress(8,8,8,8));
  
}

void loop()
{
  unsigned int localPort = 8888; // Fairly arbitrary
  unsigned char inPacket[48];
  // NTP time request packet
  unsigned char outPacket[48] = {0b11100011, 0, 6, 0xEC, 0, 0 ,0, 0, 0, 0, 0, 0, 49, 0x4E, 49, 52, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0 ,0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  WiFiUDP udp;
  udp.begin(localPort);
  statusline("Trying To Resolve Server", true);
  Serial.println("Trying To Resolve Server");

  /*-------------------------------------------------------------*/
  /* Enable only one of the following two lines.                 */
  /* Enable first line to use DNS to find an NTP server by name. */
  /* Enable second line to use an NTP server's IP address.       */
  /*-------------------------------------------------------------*/
  udp.beginPacket("time.nist.gov", 123); // NTP requests are to port 123
  //udp.beginPacket("129.6.15.28", 123); // NTP requests are to port 123

  statusline("Trying To Resolve Server", false);
  udp.write(outPacket, sizeof(outPacket));
  udp.endPacket();
  int count=100;
  while(udp.parsePacket() < sizeof(inPacket) && --count)
  {
    statusline("NTP Waiting For Response", true);
    Serial.println("NTP Waiting For Response");
    delay(20);
    // Press B or Home to return to loader
      mp.buttons.update();
      if(mp.buttons.released(BTN_B) || mp.buttons.released(BTN_HOME))
        {
          // Do a little cleanup before we leave
          udp.stop();
          WiFi.scanDelete();
          WiFi.disconnect(true); delay(10); // disable WIFI altogether
          WiFi.mode(WIFI_MODE_NULL); delay(10);
          while(!mp.update());
          mp.inCall=0;
          
          //Go Home
          mp.loader();
          break;
        }
      // Press A to select new WiFi Network
      if(mp.buttons.released(BTN_A)) 
        {
          mp.display.setTextColor(TFT_BLACK);
          mp.display.setTextSize(1);
          mp.display.setTextFont(2);
          mp.display.drawRect(4, 49, 152, 28, TFT_BLACK);
          mp.display.drawRect(3, 48, 154, 30, TFT_BLACK);
          mp.display.fillRect(5, 50, 150, 26, 0xFD29);
          mp.display.setCursor(47, 54);
          mp.display.printCenter("Searching for networks");
          Serial.println("Searching for networks");
          while(!mp.update());
          wifiConnect();
          loop();
        }
  }
  statusline("NTP Waiting For Response", false);
  if(count)
  {
    // NTP request honored - time is in packet
    unsigned long ms = millis();
    udp.read(inPacket, sizeof(inPacket));
    unsigned long secsSince1900 = (inPacket[40]<<24) | (inPacket[41]<<16) | (inPacket[42]<<8) | inPacket[43];
    long secsSinceEpoch = secsSince1900 - 2208988800UL;
      long sse = secsSinceEpoch+(millis()-ms)/1000;
      char *msg = ctime(&sse);
      msg[24]='\0';
      Serial.printf("Received: %s UTC\n",msg);
      
    // Extrapolate displayed time over 10 seconds
    while(millis()-ms < 10000)
    {
      // Extrapolate time now
      long sse = secsSinceEpoch+(millis()-ms)/1000;
      char *msg = ctime(&sse);
      msg[24]='\0';

      
      // Display the extrapolated time - if top line is yellow, time is from NTP - if green, time is exratpolated
      if(sse == secsSinceEpoch) mp.display.setTextColor(TFT_YELLOW);
      else                      mp.display.setTextColor(TFT_GREEN);
      mp.display.fillScreen(TFT_BLACK);
      mp.display.setCursor(20,8);
      mp.display.print("FROM: time.nist.gov");
      mp.display.setTextColor(TFT_GREEN);
      mp.display.setCursor(8,50);
      mp.display.print(msg);
      mp.display.print("\n\n            UTC");
      mp.display.pushSprite(0,0);
      // See if user pressed B or Home to return to loader
      mp.buttons.update();
      if(mp.buttons.released(BTN_B) || mp.buttons.released(BTN_HOME))
        {
          // Do a little cleanup before we leave
          udp.stop();
          WiFi.scanDelete();
          WiFi.disconnect(true); delay(10); // disable WIFI altogether
          WiFi.mode(WIFI_MODE_NULL); delay(10);
          while(!mp.update());
          mp.inCall=0;
          
          //Go Home
          mp.loader();
          break;
        }
      // Press A to select new WiFi Network
      if(mp.buttons.released(BTN_A)) 
        {
          mp.display.setTextColor(TFT_BLACK);
          mp.display.setTextSize(1);
          mp.display.setTextFont(2);
          mp.display.drawRect(4, 49, 152, 28, TFT_BLACK);
          mp.display.drawRect(3, 48, 154, 30, TFT_BLACK);
          mp.display.fillRect(5, 50, 150, 26, 0xFD29);
          mp.display.setCursor(47, 54);
          mp.display.printCenter("Searching for networks");
          Serial.println("Searching for networks");
          while(!mp.update());
          wifiConnect();
          loop();
        }
        
    }
  }
  else
  {
    // NTP request not honored (either outgoing packet or incoming packet lost)
    statusline("NTP No Response - Retry", true);
    Serial.println("NTP No Response - Retry");
    delay(1000);
    statusline("NTP No Response - Retry", false);
  }
  // Shut down UDP in preparation for next loop
  udp.stop();
  

}

// Display (on==true) or erase (on==false) transient status line near bottom of display
void statusline(char *msg, bool on)
{
  mp.display.setTextColor(on ? TFT_YELLOW : TFT_BLACK);
  mp.display.setCursor(0,100);
  mp.display.print(msg);
  mp.display.pushSprite(0,0);

}

// Borrowed this from git:CircuitMess/CircuitMess-Ringo-firmware/blob/master/src/settingsApp.cpp
void wifiConnect()
{
  // if(!mp.wifi)
  // {
  //  mp.display.setTextColor(TFT_BLACK);
  //  mp.display.setTextSize(1);
  //  mp.display.setTextFont(2);
  //  mp.display.drawRect(4, 49, 152, 28, TFT_BLACK);
  //  mp.display.drawRect(3, 48, 154, 30, TFT_BLACK);
  //  mp.display.fillRect(5, 50, 150, 26, 0xFD29);
  //  mp.display.setCursor(47, 54);
  //  mp.display.printCenter("WiFi turned off!");
  //  uint32_t tempMillis = millis();
  //  while(millis() < tempMillis + 2000)
  //  {
  //    mp.update();
  //    if(mp.buttons.pressed(BTN_A) || mp.buttons.pressed(BTN_B))
  //    {
  //      while(!mp.buttons.released(BTN_A) && !mp.buttons.released(BTN_B))
  //        mp.update();
  //      break;
  //    }
  //  }
  //  mp.update();
  // }
  bool blinkState = 1;
  unsigned long elapsedMillis = millis();
  bool helpPop;
  String content = ""; //password string
  String prevContent = "";
  delay(500);
  WiFi.begin();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);
  int n = WiFi.scanNetworks();
  // delay(1000);
  Serial.println("scan done");
  Serial.println(n);
  if (n < 1) {
    mp.display.setTextColor(TFT_BLACK);
    mp.display.setTextSize(1);
    mp.display.setTextFont(2);
    mp.display.drawRect(4, 49, 152, 28, TFT_BLACK);
    mp.display.drawRect(3, 48, 154, 30, TFT_BLACK);
    mp.display.fillRect(5, 50, 150, 26, 0xFD29);
    mp.display.setCursor(47, 54);
    mp.display.printCenter("No networks found!");
    WiFi.scanDelete();
    WiFi.disconnect(true); delay(10); // disable WIFI altogether
    WiFi.mode(WIFI_MODE_NULL); delay(10);
    while(!mp.update());
    uint32_t tempMillis = millis();
    WiFi.begin();
    while(millis() < tempMillis + 2000)
    {
      mp.update();
      if(mp.buttons.pressed(BTN_A) || mp.buttons.pressed(BTN_B))
      {
        while(!mp.buttons.released(BTN_A) && !mp.buttons.released(BTN_B))
          mp.update();
        break;
      }
    }
    while(!mp.update());

  }
  else
  {
    String networkNames[n];
    String wifiSignalStrengths[n];
    bool wifiPasswordNeeded[n];
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      networkNames[i] = WiFi.SSID(i);
      wifiSignalStrengths[i] = WiFi.RSSI(i);
      wifiPasswordNeeded[i] = !(WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);
    }
    while(1)
    {

      int8_t selection = wifiNetworksMenu(networkNames, wifiSignalStrengths, n);
      if(selection < 0)
      {
        while(!mp.update());
        return;
      }
      
      if (wifiPasswordNeeded[selection])
      {
        mp.textInput("");
        prevContent = "";
        content = "";
        mp.textPointer = 0;
        while(!mp.update());
        while (1)
        {
          mp.display.fillScreen(TFT_BLACK);
          mp.display.setCursor(8, 0);
          mp.display.printCenter(networkNames[selection]);
          mp.display.setCursor(4, 30);
          mp.display.printCenter("Enter password:");
          mp.display.setCursor(1, 112);
          mp.display.print("Erase");
          mp.display.setCursor(22, 83);
          mp.display.print("Press A to confirm");
          //mp.display.setCursor(133, 112);
          //mp.display.print("Help");
          if (millis() - elapsedMillis >= multi_tap_threshold) //cursor blinking routine
          {
            elapsedMillis = millis();
            blinkState = !blinkState;
          }

          mp.display.setTextFont(2);
          mp.display.fillRect(1, 55, mp.display.width(), 20, TFT_DARKGREY);
          mp.display.setTextColor(TFT_WHITE);
          mp.display.setCursor(1, 6);
          prevContent = content;
          content = mp.textInput(content, 63);
          if (prevContent != content)
          {
            blinkState = 1;
            elapsedMillis = millis();
          }

          mp.display.setCursor(4, 56);
          mp.display.setTextFont(2);
          mp.display.print(content);
          mp.display.setTextWrap(0);
          //Serial.println(mp.display.cursor_x);
          if (mp.display.cursor_x + 8 >= mp.display.width())
          {
            mp.display.fillRect(1, 55, mp.display.width(), 20, TFT_DARKGREY);
            mp.display.setCursor(mp.display.width() - mp.display.cursor_x - 4, 56);
            mp.display.print(content);
          }
          if (blinkState == 1)
            mp.display.drawFastVLine(mp.display.getCursorX(), mp.display.getCursorY(), 16, TFT_WHITE);

          /*if(mp.buttons.released(BTN_FUN_RIGHT)){
              helpPop = !helpPop;
              mp.display.drawIcon(TextHelperPopup, 0, 0, 160, 128, 1, TFT_WHITE); 
              while(!mp.update());
            }
            while (helpPop) {
              if(mp.buttons.released(BTN_FUN_RIGHT) || mp.buttons.released(BTN_B)){
                helpPop = !helpPop;
              }
            mp.update();
            }*/
          if((mp.buttons.released(BTN_A)) && content.length() > 0)
          {
            Serial.println("Password set");
            mp.display.setCursor(20, 50);
            mp.display.fillRect(0, 28, 160, 100, TFT_BLACK);
            mp.display.setCursor(0,40);
            mp.display.printCenter("Connecting");
            mp.display.setCursor(60, 60);
            while(!mp.update());

            char temp[networkNames[selection].length()+1];
            char temp2[content.length()+1];
            networkNames[selection].toCharArray(temp, networkNames[selection].length()+1);
            content.toCharArray(temp2, content.length()+1);
            WiFi.begin(temp, temp2);
            uint8_t counter = 0;
            while (WiFi.status() != WL_CONNECTED)
            {
              delay(1500);
              mp.display.print(".");
              while(!mp.update());
              counter++;
              if (counter >= 8)
              {
                mp.display.fillRect(0, 40, mp.display.width(), 60, TFT_BLACK);
                mp.display.setCursor(0, 45);
                mp.display.printCenter("Wrong password :(");
                uint32_t tempMillis = millis();
                while(millis() < tempMillis + 2000)
                {
                  mp.update();
                  if(mp.buttons.pressed(BTN_A) || mp.buttons.pressed(BTN_B))
                  {
                    while(!mp.buttons.released(BTN_A) && !mp.buttons.released(BTN_B))
                      mp.update();
                    break;
                  }
                }
                while(!mp.update());
                break;
              }
            }
            Serial.print("Wifi status: ");
            Serial.println(WiFi.status());
            // if(WiFi.status() == WL_DISCONNECTED)
            // {
              
            //  mp.display.fillRect(0, 40, mp.display.width(), 60, TFT_BLACK);
            //  mp.display.setCursor(0, 45);
            //  mp.display.printCenter("Wrong password :(");
            //  uint32_t tempMillis = millis();
            //  while(millis() < tempMillis + 2000)
            //  {
            //    mp.update();
            //    if(mp.buttons.pressed(BTN_A) || mp.buttons.pressed(BTN_B))
            //    {
            //      while(!mp.buttons.released(BTN_A) && !mp.buttons.released(BTN_B))
            //        mp.update();
            //      break;
            //    }
            //  }
            //  while(!mp.update());
            //  break;
            // }
            if(WiFi.status() == WL_CONNECTED)
            {
              mp.update();
              mp.display.setCursor(20, 50);
              mp.display.fillRect(0, 28, 160, 100, TFT_BLACK);
              mp.display.setCursor(0,40);
              mp.display.printCenter("Connected!");
              mp.display.setCursor(60, 60);
              while(!mp.update());
              delay(1000);
              mp.display.setTextSize(0);
              mp.display.setTextFont(1);
              mp.display.fillScreen(TFT_BLACK);
              mp.display.setCursor(0, 0);
              return;
            }

          }
          if(mp.buttons.released(BTN_B))
          {
            while(!mp.update());
            break;
          }
          mp.update();
        }
      }
      else
      {
        mp.display.fillScreen(TFT_BLACK);
        mp.display.setCursor(8, 8);
        mp.display.printCenter(networkNames[selection]);
        mp.display.setCursor(0,40);
        mp.display.printCenter("Connecting");
        mp.display.setCursor(60, 60);
        while(!mp.update());

        char temp[networkNames[selection].length()+1];
        networkNames[selection].toCharArray(temp, networkNames[selection].length()+1);
        Serial.println(temp);
        WiFi.begin(temp);
        uint8_t counter = 0;
        while (WiFi.status() != WL_CONNECTED)
        {
          delay(1000);
          mp.display.print(".");
          while(!mp.update());
          counter++;
          if (counter >= 8)
          {
            mp.display.fillRect(0, 40, mp.display.width(), 60, TFT_BLACK);
            mp.display.setCursor(0, 45);
            mp.display.printCenter("Wi-Fi error!");
            uint32_t tempMillis = millis();
            while(millis() < tempMillis + 2000)
            {
              mp.update();
              if(mp.buttons.pressed(BTN_A) || mp.buttons.pressed(BTN_B))
              {
                while(!mp.buttons.released(BTN_A) && !mp.buttons.released(BTN_B))
                  mp.update();
                break;
              }
            }
            while(!mp.update());
            break;
          }
        }
        if(WiFi.status() == WL_DISCONNECTED)
        {
          
          mp.display.fillRect(0, 40, mp.display.width(), 60, TFT_BLACK);
          mp.display.setCursor(0, 45);
          mp.display.printCenter("Wi-Fi error!");
          uint32_t tempMillis = millis();
          while(millis() < tempMillis + 2000)
          {
            mp.update();
            if(mp.buttons.pressed(BTN_A) || mp.buttons.pressed(BTN_B))
            {
              while(!mp.buttons.released(BTN_A) && !mp.buttons.released(BTN_B))
                mp.update();
              break;
            }
          }
          while(!mp.update());
          break;
        }
        else if(WiFi.status() == WL_CONNECTED)
        {
          mp.update();
          mp.display.setCursor(20, 50);
          mp.display.fillRect(0, 28, 160, 100, TFT_BLACK);
          mp.display.setCursor(0,40);
          mp.display.printCenter("Connected!");
          mp.display.setCursor(60, 60);
          while(!mp.update());
          delay(1000);
          mp.display.setTextSize(0);
          mp.display.setTextFont(1);
          mp.display.fillScreen(TFT_BLACK);
          mp.display.setCursor(0, 0);
          return;
        }
      }
    }
  }
}

int8_t wifiNetworksMenu(String* items, String *signals, uint8_t length) {
  int32_t cameraY = 0;
  int32_t cameraY_actual = 0;
  uint8_t offset = 22;
  uint8_t boxHeight = 20;
  int16_t cursor = 0;
  bool blinkState = 0;
  uint32_t blinkMillis = millis();
  if (length > 12) {
    cameraY = -cursor * (boxHeight + 2) - 1;
  }
  while (1) {

    mp.display.fillScreen(TFT_BLACK);
    mp.display.setCursor(0, 0);
    cameraY_actual = (cameraY_actual + cameraY) / 2;
    if (cameraY_actual - cameraY == 1) {
      cameraY_actual = cameraY;
    }
    if(millis() - blinkMillis > 350)
    {
      blinkMillis = millis();
      blinkState = !blinkState;
    }
    for (uint8_t i = 0; i < length; i++)
      wifiDrawBox(items[i], signals[i], i, cameraY_actual);
    if(blinkState)
      wifiDrawCursor(cursor, cameraY_actual);

    mp.display.fillRect(0, 0, mp.display.width(), 20, TFT_DARKGREY);
    mp.display.setTextFont(2);
    mp.display.setCursor(2,1);
    mp.display.drawFastHLine(0, 19, mp.display.width(), TFT_WHITE);
    mp.display.setTextSize(1);
    mp.display.setTextColor(TFT_WHITE);
    mp.display.print("Choose a network");
    mp.display.fillRect(0, 111, mp.display.width(), 30, TFT_DARKGREY);
    mp.display.setTextFont(2);
    mp.display.setCursor(2,112);
    mp.display.drawFastHLine(0, 111, mp.display.width(), TFT_WHITE);
    mp.display.printCenter("Cancel            Select");
    if(mp.released(BTN_FUN_RIGHT))
    {
      while(!mp.update());
      mp.playNotificationSound(cursor);
    }

    if (mp.buttons.released(BTN_A) || mp.buttons.released(BTN_FUN_RIGHT)) {   //BUTTON CONFIRM
      while(!mp.update());
      break;
    }
    if (mp.buttons.released(BTN_UP)) {  //BUTTON UP
      blinkState = 1;
      blinkMillis = millis();
      while(!mp.update());
      if (cursor == 0) {
        cursor = length - 1;
        if (length > 4) {
          cameraY = -(cursor - 3) * (boxHeight + 1);
        }
      }
      else {
        if (cursor > 0 && (cursor * (boxHeight + 1)  + cameraY + offset) <= 30) {
          cameraY += (boxHeight + 1);
        }
        cursor--;
      }

    }

    if (mp.buttons.pressed(BTN_DOWN)) { //BUTTON DOWN
      blinkState = 1;
      blinkMillis = millis();
      while(!mp.update());
      cursor++;
      if ((cursor * (boxHeight + 1) + cameraY + offset) > 100) {
        cameraY -= (boxHeight + 1);
      }
      if (cursor >= length) {
        cursor = 0;
        cameraY = 0;

      }

    }
    if (mp.buttons.released(BTN_B) || mp.buttons.released(BTN_FUN_LEFT)) //BUTTON BACK
    {
      while(!mp.update());
      return -1;
    }
    mp.update();
  }
  return cursor;

}
void wifiDrawBox(String text, String signalStrength, uint8_t i, int32_t y) {
  uint8_t offset = 23;
  uint8_t boxHeight = 20;
  y += i * (boxHeight + 2) + offset;
  if (y < 0 || y > mp.display.height()) {
    return;
  }
  mp.display.fillRect(2, y + 1, mp.display.width() - 4, boxHeight - 1, TFT_DARKGREY);
  mp.display.setTextColor(TFT_WHITE);
  mp.display.setCursor(5, y+2);
  for(uint16_t i = 0; i < text.length(); i++)
  {
    mp.display.print(text[i]);
    if(mp.display.getCursorX() > 120)
    {
      mp.display.print("...");
      break;
    }
  }
  int strength = signalStrength.toInt();
  //> -50 full
  // < -40 && > -60 high
  // < -60 && > -85 low
  // < -95 nosignal
  if(strength > -50)
    mp.display.drawBitmap(140, y+2, signalFullIcon, TFT_WHITE, 2);
  else if(strength <= -50 && strength > -70)
    mp.display.drawBitmap(140, y+2, signalHighIcon, TFT_WHITE, 2);
  else if(strength <= -70 && strength > -95)
    mp.display.drawBitmap(140, y+2, signalLowIcon, TFT_WHITE, 2);
  else if(strength <= -95)
    mp.display.drawBitmap(140, y+2, noSignalIcon, TFT_WHITE, 2);
}
void wifiDrawCursor(uint8_t i, int32_t y) {
  uint8_t offset = 23;
  uint8_t boxHeight = 20;
  y += i * (boxHeight + 2) + offset;
  mp.display.drawRect(1, y, mp.display.width() - 2, boxHeight + 1, TFT_RED);
  mp.display.drawRect(0, y-1, mp.display.width(), boxHeight + 3, TFT_RED);
}
