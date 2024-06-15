#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <WebSocketsServer_Generic.h>
#include "camera.h"
#include "img_converters.h"

#include <ESP32Servo.h>
Servo myservo;  
#define BUTTON_PIN 15 // Порт IO15 для кнопки

#define AP_SSID "promolchi"
#define AP_PASS "08052002"

WebSocketsServer ws(82, "", "hub");
WebServer server(80);

const char* loginPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Login</title>
</head>
<body>
  <h1>Login</h1>
  <form action="/login" method="post">
    <label for="username">Username:</label>
    <input type="text" id="username" name="username"><br><br>
    <label for="password">Password:</label>
    <input type="password" id="password" name="password"><br><br>
    <input type="submit" value="Submit">
  </form>
</body>
</html>
)rawliteral";

const char* homePage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Home Page</title>
  <style>
    .button {
      display: block;
      width: 300px;
      height: 60px;
      margin: 10px auto;
      font-size: 24px;
      text-align: center;
      line-height: 60px;
      background-color: #4CAF50;
      color: white;
      text-decoration: none;
    }
  </style>
</head>
<body>
  <h1><p align="center">Home Page</p></h1>
  <a href="/video_surveillance" class="button">Video surveillance of RC</a>
  <a href="/entrance_access" class="button">Access to the entrance</a>
  <a href="/object_access" class="button">Access to RC facilities</a>
  <a href="/electricity_supply" class="button">Electricity supply</a>
  <a href="/smart_parking" class="button">Smart parking</a>
</body>
</html>
)rawliteral";

const char* smartParkingPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Smart Parking</title>
  <style>
    .button {
      display: block;
      width: 300px;
      height: 60px;
      margin: 10px auto;
      font-size: 24px;
      text-align: center;
      line-height: 60px;
      background-color: #4CAF50;
      color: white;
      text-decoration: none;
    }
  </style>
</head>
<body>
  <p align="center"><h1><p align="center">Smart Parking</p></h1></p>
  <a href="/booking" class="button">Parking space reservation</a>
  <a href="/video" class="button">Video surveillance</a>
  <a href="/search" class="button">Search for a parking space</a>
</body>
</html>
)rawliteral";

const char* Entrance_access_Page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Access to the entrance</title>
  <style>
    .button {
      display: block;
      width: 300px;
      height: 60px;
      margin: 10px auto;
      font-size: 24px;
      text-align: center;
      line-height: 60px;
      background-color: #4CAF50;
      color: white;
      text-decoration: none;
    }
  </style>
</head>
<body>
  <h1><p align="center">Access to the entrance</p></h1>
  <label for="`Entrance_number"><p align="center">Entrance number:</label>
  <input type="text" id="Entrance_number" name="Entrance_number"><br><br>
  <a href="/entrance_access" class="button">Grant access</a>
</body>
</html>
)rawliteral";

const char* Object_access_Page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Access to RC facilities</title>
  <style>
    .button {
      display: block;
      width: 300px;
      height: 60px;
      margin: 10px auto;
      font-size: 24px;
      text-align: center;
      line-height: 60px;
      background-color: #4CAF50;
      color: white;
      text-decoration: none;
    }
  </style>
</head>
<body>
  <h1><p align="center">Access to RC facilities</p></h1>
  <label for="`Name_object"><p align="center">Name of the object:</label>
  <input type="text" id="Name of the object" name="`Name of the object"><br><br>
  <a href="/object_access" class="button">Grant access</a>
</body>
</html>
)rawliteral";
                        
const char* Electricity_supplyPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Electricity supply</title>
  <style>
  .button {
  display: block;
  width: 300px;
  height: 60px;
  margin: 10px auto;
  font-size: 24px;
  text-align: center;
  line-height: 60px;
  background-color: #4CAF50;
  color: white;
  text-decoration: none;
    }
   </style>
  </head>
  <body>
   <h1><p align="center">Electricity supply</p></h1>
    <label for="`Name_object"><p align="center">Name of the object:</label>
    <input type="text" id="Name of the object" name="`Name of the object"><br><br>
      <a href="/electricity_supply" class="button">Review</a>
    </body>
    </html>
)rawliteral";

const char* booking_Page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Access to RC facilities</title>
  <style>
    .button {
      display: block;
      width: 300px;
      height: 60px;
      margin: 10px auto;
      font-size: 24px;
      text-align: center;
      line-height: 60px;
      background-color: #4CAF50;
      color: white;
      text-decoration: none;
    }
  </style>
</head>
<body>
  <h1><p align="center">Parking space reservation</p></h1>
  <label for="`Enter_number"><p align="center">Enter the parking space number:</h1></label>
  <input type="text" id="Enter the parking space number" name="Enter the parking space number"><br><br>
  <a href="/Reserve_servo" class="button">Reserve</a>
  <a href="/Grant_access_servo" class="button">Grant access</a>
</body>
</html>
)rawliteral";


const char* buttonPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Button State</title>
</head>
<body>
  <h1><p align="center">Parking space reservation</p></h1>
  <label for="`Enter_number"><p align="center">Enter the parking space number:</h1></label>
  <input type="text" id="Enter the parking space number" name="Enter the parking space number"><br><br>
  <p align="center">The place is:<span id="buttonState">Unknown</span></p>
  <script>
    const buttonStateSpan = document.getElementById('buttonState');
    function fetchButtonState() {
      fetch('/button_state')
        .then(response => response.text())
        .then(state => {
          buttonStateSpan.innerText = state;
        });
    }
    setInterval(fetchButtonState, 1000); // Оновлювати кожну секунду
  </script>
</body>
</html>
)rawliteral";


const char* surveillanceRCPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Camera Unavailable</title>
  <style>
  .button {
      display: block;
      width: 300px;
      height: 60px;
      margin: 10px auto;
      font-size: 24px;
      text-align: center;
      line-height: 60px;
      background-color: #4CAF50;
      color: white;
      text-decoration: none;
    }
    .container {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
    }
    .box {
      width: 200px;
      height: 200px;
      border: 1px solid #000;
      display: flex;
      align-items: center;
      justify-content: center;
      margin: 20px;
      text-align: center;
      box-sizing: border-box;
    }
    .box-container {
      width: 1000px;
      display: flex;
      flex-wrap: wrap;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="box-container">
      <div class="box">The camera is not available</div>
      <div class="box">The camera is not available</div>
      <div class="box">The camera is not available</div>
      <div class="box">The camera is not available</div>
      <div class="box">The camera is not available</div>
      <div class="box">The camera is not available</div>
      <div class="box">The camera is not available</div>
      <div class="box">The camera is not available</div>
    </div>
  </div>
   <a href="/video_surveillance" class="button">Archive</a>
  
</body>
</html>

)rawliteral";


const char* streamPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Video surveillance</title>
    <style>
    .button {
      display: block;
      width: 300px;
      height: 60px;
      margin: 10px auto;
      font-size: 24px;
      text-align: center;
      line-height: 60px;
      background-color: #4CAF50;
      color: white;
      text-decoration: none;
    }
  </style>
</head>
<body>
  <h1><p align="center">Video surveillance</p></h1>
   <label for="`Enter_number"><p align="center">Enter the parking space number:</h1></label>
   <input type="text" id="Enter the parking space number" name="Enter the parking space number"><br><br>
   <a href="/video" class="button">Video surveillance</a>
   <a href="/video" class="button">Archive</a>
      
  <p align="center"><img id="stream" src=""></p>
  <script>
    const streamImg = document.getElementById('stream');
    const ws = new WebSocket(`ws://${location.hostname}:82/`);
    ws.binaryType = 'arraybuffer';
    ws.onmessage = (event) => {
      const blob = new Blob([event.data], { type: 'image/jpeg' });
      const url = URL.createObjectURL(blob);
      streamImg.src = url;
    };
  </script>
</body>
</html>
)rawliteral";

bool isAuthenticated = false;

void handleRoot() {
    if (!isAuthenticated) {
        server.send(200, "text/html", loginPage);
    } else {
        server.send(200, "text/html", homePage);
    }
}

void handleLogin() {
    if (server.method() == HTTP_POST) {
        String username = server.arg("username");
        String password = server.arg("password");

        if (username == "admin" && password == "admin") { // Заміни на свої логін і пароль
            isAuthenticated = true;
            server.send(200, "text/html", homePage);
        } else {
            server.send(401, "text/html", "Unauthorized");
        }
    } else {
        server.send(405, "text/html", "Method Not Allowed");
    }
}

void handleSmartParking() {
    server.send(200, "text/html", smartParkingPage);
}

void handleEntrance_access() {
    server.send(200, "text/html",Entrance_access_Page);
}

void handleObject_access() {
    server.send(200, "text/html",Object_access_Page);
}

void handlebooking() {
    server.send(200, "text/html",booking_Page);
}

void Serovo_doun() {
    server.send(200, "text/html",booking_Page);
    myservo.write(0);
}

void Serovo_up() {
    server.send(200, "text/html",booking_Page);
    myservo.write(90);
}

void handleButtonStatePage() {
    server.send(200, "text/html", buttonPage);
}

void handlesurveillanceRC() {
    server.send(200, "text/html", surveillanceRCPage);
}


void handleElectricity_supply() {
    server.send(200, "text/html", Electricity_supplyPage);
}

void handleButtonState() {
    int buttonState = digitalRead(BUTTON_PIN);
    server.send(200, "text/plain", buttonState == HIGH ? " free" : " busy");
}


void handleVideoSurveillance() {
    server.send(200, "text/html", streamPage);
    handleStartStream();// Вмикання стріму з камери тут
}

void setup() {
    Serial.begin(115200);
    delay(200);
    cam_init(FRAMESIZE_VGA, PIXFORMAT_JPEG, 10);
    
    myservo.attach(13);

    WiFi.mode(WIFI_STA);
    WiFi.begin(AP_SSID, AP_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println(WiFi.localIP());

    ws.begin();

    server.on("/", handleRoot);
    server.on("/login", HTTP_POST, handleLogin);
    server.on("/smart_parking", handleSmartParking);
    server.on("/entrance_access", handleEntrance_access);
    server.on("/object_access", handleObject_access);
    server.on("/booking", handlebooking);
    server.on("/Reserve_servo", Serovo_doun);
    server.on("/Grant_access_servo", Serovo_up);
    server.on("/search", handleButtonStatePage);
    server.on("/button_state", handleButtonState);
    server.on("/electricity_supply", handleElectricity_supply);

    server.on("/video_surveillance", handlesurveillanceRC);


    server.on("/video", handleVideoSurveillance);
    server.begin();

}

void loop() {
    server.handleClient();
    ws.loop();

    if (!ws.connectedClients()) return;

    camera_fb_t *fbj = nullptr;
    fbj = esp_camera_fb_get();
    if (fbj) {
        uint32_t len = fbj->width * fbj->height * 2;
        uint8_t *buf = (uint8_t *)ps_malloc(len);

        if (buf) {
            bool ok = jpg2rgb565(fbj->buf, fbj->len, buf, JPG_SCALE_NONE);
            if (ok) {
                for (uint32_t i = 0; i < len; i += 2) {
                    uint8_t b = buf[i];
                    buf[i] = buf[i + 1];
                    buf[i + 1] = b;
                }

                if (ws.connectedClients()) {
                    size_t jpg_buf_len = 0;
                    uint8_t *jpg_buf = nullptr;
                    ok = fmt2jpg(buf, len, fbj->width, fbj->height, PIXFORMAT_RGB565, 80, &jpg_buf, &jpg_buf_len);
                    if (ok) ws.broadcastBIN(jpg_buf, jpg_buf_len);
                    if (jpg_buf) free(jpg_buf);
                }
            }
            free(buf);
        }
        esp_camera_fb_return(fbj);
    }
    delay(30);
}

void handleStartStream() {
    // Підготовка та запуск стріму з камери
    camera_fb_t *fbj = nullptr;
    fbj = esp_camera_fb_get();
    if (fbj) {
        uint32_t len = fbj->width * fbj->height * 2;
        uint8_t *buf = (uint8_t *)ps_malloc(len);

        if (buf) {
            bool ok = jpg2rgb565(fbj->buf, fbj->len, buf, JPG_SCALE_NONE);
            if (ok) {
                for (uint32_t i = 0; i < len; i += 2) {
                    uint8_t b = buf[i];
                    buf[i] = buf[i + 1];
                    buf[i + 1] = b;
                }

                if (ws.connectedClients()) {
                    size_t jpg_buf_len = 0;
                    uint8_t *jpg_buf = nullptr;
                    ok = fmt2jpg(buf, len, fbj->width, fbj->height, PIXFORMAT_RGB565, 80, &jpg_buf, &jpg_buf_len);
                    if (ok) ws.broadcastBIN(jpg_buf, jpg_buf_len);
                    if (jpg_buf) free(jpg_buf);
                }
            }
            free(buf);
        }
        esp_camera_fb_return(fbj);
    }
    server.send(200, "text/plain", "Stream started");
}
