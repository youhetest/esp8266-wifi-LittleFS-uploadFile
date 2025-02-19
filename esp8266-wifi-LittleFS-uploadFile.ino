#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

// 定义你的 WiFi 网络的 SSID 和密码
const char* WIFI_SSID = "you-wifi-SSID";
const char* WIFI_PASSWORD = "password";

// ESP8266WebServer server(80); // 创建 Web 服务器对象，监听 80 端口

AsyncWebServer server(80);

void setupLittleFS() {
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    Serial.println("Formatting LittleFS...");
    if (LittleFS.format()) {
      Serial.println("LittleFS formatted successfully");
    } else {
      Serial.println("LittleFS format failed");
    }
  }
}

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ");
  Serial.print(WIFI_SSID);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("WiFi connection failed");
  }
}

void handleFileUpload(AsyncWebServerRequest *request) {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>File Upload</title>
    </head>
    <body>
      <h1>File Upload</h1>
      <form method="POST" action="/upload" enctype="multipart/form-data">
        <input type="file" name="file">
        <input type="submit" value="Upload">
      </form>
    </body>
    </html>
  )rawliteral";
  request->send(200, "text/html", html);
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  static File uploadFile;

  if (!index) {
    Serial.printf("UploadStart: %s\n", filename.c_str());
    if (!filename.startsWith("/")) filename = "/" + filename;
    uploadFile = LittleFS.open(filename, "w");
    if (!uploadFile) {
      request->send(500, "text/plain", "Failed to open file for writing");
      return;
    }
  }
  if (uploadFile) {
    uploadFile.write(data, len);
  }
  if (final) {
    Serial.printf("UploadEnd: %s, %u bytes\n", filename.c_str(), index+len);
    uploadFile.close();
    request->send(200, "text/plain", "File uploaded successfully: " + filename);
    printLittleFSFiles();
  }
}

void printLittleFSFiles() {
  Serial.println("Files in LittleFS:");
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
    Serial.print("  ");
    Serial.println(dir.fileName());
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  setupLittleFS();
  connectWiFi();

  server.on("/", HTTP_GET, handleFileUpload);
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){}, handleUpload);  // Use lambda for empty handler
  server.begin();

  Serial.println("Async Web server started");
}


void loop() {
  // server.handleClient(); // 处理客户端请求
}
