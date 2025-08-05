#include <WiFi.h>
#include <WebServer.h>

// Pin definitions
#define TRIG_PIN    5
#define ECHO_PIN    18

// Distance thresholds (in cm)
#define SAFE_DISTANCE     100
#define WARNING_DISTANCE  50

// Wi-Fi credentials
const char* ssid = "Infinix HOT 12i";
const char* password = "rfaith123";

// Web server on port 80
WebServer server(80);

// Global message to send to browser
String alertMessage = "Waiting for measurement...";

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  // Define route for dashboard
  server.on("/", handleDashboard);
  server.on("/message", handleMessage);

  server.begin();
}

void loop() {
  server.handleClient();

  long duration;
  int distance;

  // Trigger ultrasonic pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read echo pulse
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;

  Serial.println("Distance: " + String(distance) + " cm");

  // Update alert message
  if (distance > SAFE_DISTANCE) {
    alertMessage = "Safe distance: " + String(distance) + " centimeters";
  } else if (distance > WARNING_DISTANCE) {
    alertMessage = "Caution: Object at " + String(distance) + " centimeters";
  } else {
    alertMessage = "Lookout! Obstacle is very close: " + String(distance) + " centimeters";
  }

  delay(1000);
}

// Serve the dashboard HTML
void handleDashboard() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>ESP32 Ultrasonic TTS</title>
      <style>
        body { font-family: sans-serif; text-align: center; padding: 2rem; background: #f0f4f8; }
        h1 { color: #0078D7; }
        #message { font-size: 1.5rem; margin-top: 2rem; color: #333; }
      </style>
    </head>
    <body>
      <h1>Smart Cane Assistant</h1>
      <div id="message">Loading...</div>

      <script>
        async function fetchMessage() {
          const res = await fetch('/message');
          const text = await res.text();
          document.getElementById('message').textContent = text;

          const utterance = new SpeechSynthesisUtterance(text);
          utterance.lang = 'en-US';
          speechSynthesis.speak(utterance);
        }

        setInterval(fetchMessage, 2000); // Fetch every 2 seconds
      </script>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

// Serve the latest message
void handleMessage() {
  server.send(200, "text/plain", alertMessage);
}