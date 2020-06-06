
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

// See highlighted comments below to customize program for your network and DNS testing.

// Example coded 5/10/2020 by Frank Prindle.
// Additional code added by TheWebMachine 6/6/2020 onward

#include "MAKERphone.h"
MAKERphone mp;

void setup()
{
  mp.begin(1);

  /*-----------------------------------------------------------*/
  /* Set the following two strings to match your WiFi network. */
  /*-----------------------------------------------------------*/
  char *SSID = "MyNetwork";
  char *WPAPassword = "abcde12345";

  /*-------------------------------------------------------------------*/
  /* Disable the following line to use DHCP supplied DNS server.       */
  /* Enable the following line to use a well-known DNS server.         */
  /*    Set the first IP address to a valid static IP on your network. */
  /*    Set the second IP address to the IP address of your router.    */
  /*    Set the third IP address to your network mask.                 */
  /*    Leave the fourth IP address alone (Google DNS).                */
  /*-------------------------------------------------------------------*/
  //WiFi.config(IPAddress(192,168,1,177),IPAddress(192,168,1,1),IPAddress(255,255,255,0),IPAddress(8,8,8,8));
  
  WiFi.begin(SSID,WPAPassword);
  int count=100;
  while(WiFi.status() != WL_CONNECTED && count--) delay(100);
  mp.display.setTextFont(1);
  mp.display.fillScreen(TFT_BLACK);
  if(WiFi.status() != WL_CONNECTED)
  {
    Serial.printf("WiFi cannot connect to given SSID with given password\n");
    statusline("WiFi Connect Failure", true);
    delay(5000);
    statusline("WiFi Connect Failure", false);
  }
  else
  {
    Serial.printf("WiFi is connected\n");
    statusline("WiFi Is Connected", true);
    delay(2000);
    statusline("WiFi Is Connected", false);
  }
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
    delay(20);
  }
  statusline("NTP Waiting For Response", false);
  if(count)
  {
    // NTP request honored - time is in packet
    unsigned long ms = millis();
    udp.read(inPacket, sizeof(inPacket));
    unsigned long secsSince1900 = (inPacket[40]<<24) | (inPacket[41]<<16) | (inPacket[42]<<8) | inPacket[43];
    long secsSinceEpoch = secsSince1900 - 2208988800UL;

    // Extrapolate displayed time over 10 seconds
    while(millis()-ms < 10000)
    {
      // Extrapolate time now
      long sse = secsSinceEpoch+(millis()-ms)/1000;
      char *msg = ctime(&sse);
      msg[24]='\0';
      Serial.printf("%s UTC\n",msg);

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
          // Do a little cleanup (close UDP stream) before we leave
          udp.stop();
          mp.loader();
          break;
        }
        
    }
  }
  else
  {
    // NTP request not honored (either outgoing packet or incoming packet lost)
    statusline("NTP No Response - Retry", true);
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
