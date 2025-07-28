
#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include <DHT.h>

#define WIFI_SSID "Galaxy M14 5G 17C4"
#define WIFI_PASSWORD "royarghya@1507"

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define AUTHOR_EMAIL "bubun15072006@gmail.com"
#define AUTHOR_PASSWORD "jjml yrnn owpd chpt"
#define RECIPIENT_EMAIL "roym20730@gmail.com"

#define DHTPIN 21
#define DHTTYPE DHT11
#define MQ2_PIN 35
#define RED_LED 13
#define GREEN_LED 25
#define BUZZER_PIN 4

DHT dht(DHTPIN, DHTTYPE);

// Email objects
SMTPSession smtp;
ESP_Mail_Session session;
SMTP_Message message;

bool emailSent = false;  // Flag to prevent repeat emails

void setup() {
  Serial.begin(115200);

  dht.begin();

  pinMode(MQ2_PIN, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int smoke = analogRead(MQ2_PIN);

  bool fireDetected = (temp > 31) || (smoke > 1200);
  bool warningTempOnly = ((temp > 30 && temp <= 31) && smoke > 1200);

  // LED & buzzer logic
  if (fireDetected) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER_PIN, HIGH);

    if (!emailSent) {
      sendEmail(temp, smoke);
      emailSent = true;
    }

  } else if (warningTempOnly) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER_PIN, LOW);  // Beeping logic could go here
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
    emailSent = false;  // Reset flag once fire condition clears
  }

  // Debug output
  Serial.print("Temp: "); Serial.print(temp); Serial.print(" °C, ");
  Serial.print("Humidity: "); Serial.print(hum); Serial.print(" %, ");
  Serial.print("Smoke: "); Serial.print(smoke); Serial.print(" --> ");

  if (fireDetected) {
    Serial.println("🔥 FIRE ALERT");
  } else if (warningTempOnly) {
    Serial.println("⚠️ Temp Warning");
  } else {
    Serial.println("✅ Safe");
  }

  delay(2000);
}

void sendEmail(float temp, int smoke) {
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  message.sender.name = "ESP32 Fire Alert";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "🔥 FIRE ALERT from ESP32";
  message.addRecipient("Owner", RECIPIENT_EMAIL);

  String htmlMsg = "<h2>🔥 FIRE DETECTED!</h2><p><b>Temperature:</b> " + String(temp) +
                   " °C<br><b>Smoke Level:</b> " + String(smoke) + "</p>";
  message.html.content = htmlMsg.c_str();
  message.html.content_type = "text/html";

  if (!smtp.connect(&session)) {
    Serial.println("Failed to connect to SMTP server");
    return;
  }

  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.print("Error sending Email: ");
    Serial.println(smtp.errorReason());
  } else {
    Serial.println("✅ Email sent successfully!");
  }

  smtp.closeSession();
}
