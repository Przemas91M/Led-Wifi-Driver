#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 5
#define LED_COUNT 46

const char *ssid = "YOUR_SSID";
const char *pass = "YOUR_PASSWORD";

ESP8266WebServer server(80);

unsigned long wait = 1000;
unsigned long now = 0, diff = 0;

uint8_t r = 0;
uint8_t g = 0;
uint8_t b = 0;
long firstPixelHue = 0;
int pixelHue;
bool rainbow_enable = false;
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

String header;
String setcolor = "#ffffff";

const char MAIN_page[] PROGMEM = R"=====(
  <!DOCTYPE HTML>
  <html lang="en">
  <head>
    <meta charset="utf-8" />
    <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LED CONTROLLER</title>
    <link href="https://fonts.googleapis.com/css?family=Lato:400,900&subset=latin-ext" rel="stylesheet">
    <link href="https://fonts.googleapis.com/icon?family=Material+Icons" rel="stylesheet">
    <style> 
      
     body {
      background: linear-gradient(90deg, rgba(232,20,20,1) 0%, rgba(5,255,112,1) 100%);
      }
      
      button {
      width: 60px;
      height: 60px;
      font-size: 15px;
      margin: 5px;
      border-radius: 30px;
      -webkit-transition-duration: 0.4s; /* Safari */
      transition-duration: 0.2s;
      cursor: pointer;
      text-align: city center;
      font-family: 'Lato', sans-serif;
      font-weight: bold;
      outline:none;
      border: none;
      color:white;  }

      #container {
      max-width: 310px;
      margin: auto;
      margin-top: 100px;
      background-color: #BDB7AA;
      border: 2px solid #AAB0BD;
      border-radius: 15px;
      padding: 5px;
      padding-top: 20px;
      padding-bottom: 20px;
      margin-top: 20px; 
      }

      #off {
      background-color: black;
      color: white; }
      #off:hover {
      background-color: white;
      border: 2px solid black;
      color: black; }

      #red {  background-color: #FF191B;  }
      #red:hover {
      background-color: white;
      border: 2px solid #FF191B;
      color: black; }
      
      
      #grad1 {  background-image: linear-gradient(0deg, rgba(232,20,20,1) 0%, 		    rgba(0,255,112,1) 100%);  }
      #grad1:hover {
      background-color: white;
      border: 2px solid #FFFFFF;
      color: black; }
      
      #grad2{background-image:linear-gradient(0deg, rgba(34,193,195,1) 0%, rgba(253,187,45,1) 100%);}
      #grad2:hover {
      background-color: white;
      border: 2px solid #FFFFFF;
      color: black; }
      
      #grad3{background-image:linear-gradient(0deg, rgba(115,3,192,1) 0%, rgba(236,56,188,1) 100%);}
      #grad3:hover {
      background-color: white;
      border: 2px solid #FFFFFF;
      color: black; }

      #green {  background-color: #288F00;  }
      #green:hover {
      background-color: white;
      border: 2px solid #288F00;
      color: black; }

      #blue { background-color: #122B9C;  }
      #blue:hover {
      background-color: white;
      border: 2px solid #122B9C;
      color: black; }
      
      #rainbow {
      background-image: linear-gradient(to right, red,orange,yellow,green,blue,indigo,violet); 
      animation:slidebg 5s linear infinite;
      font-size: 10px;
      }

    .function {
      background-color: #5A6170;
      font-size: 10px;  }
    .function:hover {
      background-color: white;
      border: 2px solid #5A6170;
      color: black;
      font-size: 10px;  }
      
      @keyframes slidebg {
  to {
    background-position:10vw;
  }
}
    </style>  
  </head>
  <body>
  <div id="container">
    <a href="red"><button id="red">R</button></a>
    <a href="green"><button id="green">G</button></a>
    <a href="blue"><button id="blue">B</button></a>
    <a href="off"><button id="off"><i class="material-icons">lightbulb_outline</i></button></a>
    <div style="clear : both"></div>
    <p>
     <a href="rainbow"><button id="rainbow"></button></a>
    <a href="grad1"><button id="grad1"></button></a>
    <a href="grad2"><button id="grad2"></button></a>
    <a href="grad3"><button id="grad3"> </button></a>
    </p>
    <div style="clear : both"></div>
    <center>
    	<p>
       	<font face="LAto" size="4px" color="#00000">Wybierz kolor z palety:</font>
    	</p>
    	<form method="post" action="/form">
    	<input type="color" name="color" value="#FFFFFF">
    <div style="clear : both">
      <p></p>      
    	<input type="checkbox" id="gora" name="gora" checked>
      	<label for="gora">Góra</label> 
      	<input type="checkbox" id="dol" name="dol" checked>
      	<label for="dol">Dół</label> 
     </div>
    <div style="clear : both"></div>
    <button type="submit" button class="function">USTAW</button>
    </form>
    </center>
    <div style="clear : both"></div>
    <center>
    <p>
     <font face="LAto" size="4px" color="#00000">Wybierz jasność paska:</font>
    </p> 
    <form method="post" action="/bright">
    <div class="slidecontainer">
      <input type="range" name="bright" min="1" max="100" value="100" id="bright">
      <button type="submit" button class="function">USTAW</button>
      </div>
      </form>
    </center>
    </div>
  </body>
  </html> 
)=====";

void ConnectToWifi()
{
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected!");
  Serial.print("My IP is: ");
  Serial.println(WiFi.localIP());
}

void colorWipe(uint32_t color, int wait)
{
  rainbow_enable = false;
  for (int i = 0; i < strip.numPixels(); i++)
  {                                // For each pixel in strip...
    strip.setPixelColor(i, color); //  Set pixel's color (in RAM)
    strip.show();                  //  Update strip to match
    delay(wait);                   //  Pause for a moment
  }
}

//uaktywnamy kolor tylko od wybranego piksela (funkcja tylko dla nóg)
void colorWipe(uint32_t color, int pocz, int wait)
{
  //uint32_t zero = strip.Color(0, 0, 0);
  for (int i = pocz; i < (pocz + 23); i++)
  {                                // For each pixel in strip...
    strip.setPixelColor(i, color); //  Set pixel's color (in RAM)
    strip.show();                  //  Update strip to match
    delay(wait);                   //  Pause for a moment
  }
  /*for (int i = pocz; i < strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }*/
}

void rainbow()
{
  //for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
  if (firstPixelHue < 5 * 65536)
  {
    for (int i = 0; i < strip.numPixels(); i++)
    { // For each pixel in strip...
      pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    firstPixelHue += 256;
  }
  else
  {
    firstPixelHue = 0;
  }
  strip.show(); // Update strip with new contents
  //delay(wait);  // Pause for a moment
}

void handleMainPage()
{
  String p = MAIN_page;
  p.replace("@@color@@", setcolor);
  server.send(200, "text/html", p);
}

void handleColor()
{
  setcolor = server.arg("color");
  String gora = server.arg("gora");
  String dol = server.arg("dol");
  Serial.println(setcolor);
  String hexstring = setcolor;
  long color = strtol(&hexstring[1], NULL, 16);
  int r = color >> 16;
  int g = color >> 8 & 0xFF;
  int b = color & 0xFF;
  //mamy cały pasek
  if (gora.equals("on") && dol.equals("on"))
  {
    colorWipe(strip.Color(r, b, g), 50);
    Serial.println("gora i dol");
  }
  //mamy tylko górę
  else if (gora.equals("on"))
  {
    colorWipe(strip.Color(r, b, g), 0, 50);
    Serial.println("tylko gora");
  }
  //tylko dół
  else if (dol.equals("on"))
  {
    colorWipe(strip.Color(r, b, g), 23, 50);
    Serial.println("tylko dol");
  }
  //jak nie wybierzemy nic, funkcja przeładuje stronę

  server.send(200, "text/html", MAIN_page);
}

void handleMessage()
{
  server.send(302, "text/plain", "Updated-- Press Back Button");
}

void handleBrightness()
{
  String brightness = server.arg("bright");
  int bright = brightness.toInt();
  Serial.println("Brightness = " + brightness);
  bright = map(bright, 0, 100, 0, 255);
  Serial.print("Bright = ");
  Serial.println(bright);
  strip.setBrightness(bright);
  strip.show();
  server.send(200, "text/html", MAIN_page);
}

void handle404Exception()
{
  server.send(404, "text/plain", "Page not found!");
}

void setup()
{
  Serial.begin(115200);
  ConnectToWifi();
  server.on("/", handleMainPage);
  server.on("/form", handleColor);
  server.on("/message", handleMessage);
  server.on("/red", []()
            {
              server.send(200, "text/html", MAIN_page);
              colorWipe(strip.Color(255, 0, 0), 50);
            });
  server.on("/green", []()
            {
              server.send(200, "text/html", MAIN_page);
              colorWipe(strip.Color(0, 0, 255), 50);
            });
  server.on("/blue", []()
            {
              server.send(200, "text/html", MAIN_page);
              colorWipe(strip.Color(0, 255, 0), 50);
            });
  server.on("/grad1", []()
            {
              server.send(200, "text/html", MAIN_page);
              colorWipe(strip.Color(0, 112, 255), 0, 50); //góra na zielono
              colorWipe(strip.Color(255, 0, 0), 23, 50);  //dół na czerwono
            });
  server.on("/grad2", []()
            {
              server.send(200, "text/html", MAIN_page);
              colorWipe(strip.Color(253, 45, 187), 0, 50);  //góra na zielono
              colorWipe(strip.Color(34, 195, 193), 23, 50); //dół na czerwono
            });
  server.on("/grad3", []()
            {
              server.send(200, "text/html", MAIN_page);
              colorWipe(strip.Color(236, 188, 56), 0, 50); //góra na zielono
              colorWipe(strip.Color(115, 192, 3), 23, 50); //dół na czerwono
            });
  server.on("/off", []()
            {
              server.send(200, "text/html", MAIN_page);
              colorWipe(strip.Color(0, 0, 0), 1);
            });
  server.on("/rainbow", []()
            {
              server.send(200, "text/html", MAIN_page);
              rainbow_enable = true;
            });
  server.on("/bright", handleBrightness);
  server.onNotFound(handle404Exception);
  server.begin();
  Serial.println("Server started");

  strip.begin();            // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.setBrightness(255); // Set BRIGHTNESS to about 1/5 (max = 255)
  strip.show();             // Turn OFF all pixels ASAP
}

void loop()
{
  server.handleClient();
  if (rainbow_enable)
  {
    now = millis();
    if ((now - diff) > 50)
    {
      diff = now;
      rainbow();
    }
  }
}
