#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

#define WIFI_SSID "OPPO Reno6"
#define WIFI_PASSWORD "h9tuxegm"

// Define the API key
#define API_KEY "AIzaSyByRGfh8ycKeb59kXxKGd6n2RN3R3aJjtw"
/* If work with RTDB, define the RTDB URL */
#define DATABASE_URL "https://iotprj-b5c41-default-rtdb.firebaseio.com/"

/* Define the Firebase Data object */
FirebaseData fbdo;
/* Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;
/* Define the FirebaseConfig data for config data */
FirebaseConfig config;

// Delaying time
unsigned long dataMillis = 0;

// Declaring IR SENSOR 
const int IR_PIN = 34;

// Declaring NTC Temp Sensor
const int TEMP_PIN = 35;

// Declaring Mic Sensor
const int MIC_PIN = 25;

// Declaring Buzzer
const int BUZZER_PIN = 27;

// Declaring Servo
Servo myServo;
const int SERVO_PIN = 32;

// Declaring Keypad
const byte ROWS = 4; 
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {21, 19, 18, 5};
byte colPins[COLS] = {12, 13, 14, 15};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Declaring LCD with I2C address 0x27 (adjust if necessary)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address to 0x27 for a 16 chars and 2 line display

const String correctPassword = "1234"; 
String inputPassword = "";

void setup()
{
    pinMode(IR_PIN, INPUT);
    pinMode(TEMP_PIN, INPUT);
    pinMode(MIC_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    myServo.attach(SERVO_PIN);

    // Initialize I2C for LCD
    Wire.begin(21, 22);  // SDA = 21, SCL = 22
    lcd.init();
    lcd.backlight();
    
    // Display start message on LCD
    lcd.clear();
    lcd.print("Starting...");
    delay(2000);

    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    /* Assign the api key (required) */
    config.api_key = API_KEY;
    /* Assign the user sign in credentials */
    auth.user.email = "parth@email.com";
    auth.user.password = "parth123";
    config.database_url = DATABASE_URL;
    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);
    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback;
    /* Initialize the library with the Firebase authen and config */
    Firebase.begin(&config, &auth);
}

void loop() {
    if (millis() - dataMillis > 1000 && Firebase.ready()) {
        dataMillis = millis();
        
        // Read analog data from IR sensor
        int IR_reads = analogRead(IR_PIN);

        // Read analog data from NTC temperature sensor
        int tempValue = analogRead(TEMP_PIN);
        float voltage = tempValue * (3.3 / 4095.0); // Assuming a 3.3V reference voltage
        float temperature = (voltage - 0.5) * 100.0; // Convert voltage to temperature in Celsius (example formula)

        // Read analog data from Mic sensor
        int micValue = analogRead(MIC_PIN);

        // Send IR data to Firebase
        if (Firebase.RTDB.setInt(&fbdo, "IR/", IR_reads)) {
            Serial.println("IR data sent successfully: " + String(IR_reads));
        } else {
            Serial.println("Failed to send IR data: " + fbdo.errorReason());
        }

        // Send temperature data to Firebase
        if (Firebase.RTDB.setFloat(&fbdo, "Temperature/", temperature)) {
            Serial.println("Temperature data sent successfully: " + String(temperature));
        } else {
            Serial.println("Failed to send Temperature data: " + fbdo.errorReason());
        }

        // Send Mic sensor data to Firebase
        if (Firebase.RTDB.setInt(&fbdo, "Mic/", micValue)) {
            Serial.println("Mic data sent successfully: " + String(micValue));
        } else {
            Serial.println("Failed to send Mic data: " + fbdo.errorReason());
        }
    }

    // Keypad input
    char key = keypad.getKey();
    if (key != NO_KEY) {
        lcd.setCursor(inputPassword.length(), 1); // Set cursor position on LCD
        lcd.print(key); // Print pressed key on LCD

        if (key == '#') {
            if (inputPassword == correctPassword) {
                lcd.clear();
                lcd.print("Welcome");
                myServo.write(90); // Move servo to 90 degrees
                delay(2000); // Keep it for 2 seconds
                myServo.write(0); // Move servo back to 0 degrees
            } else {
                lcd.clear();
                lcd.print("Wrong Password");
                for (int i = 0; i < 2; i++) {
                    digitalWrite(BUZZER_PIN, HIGH);
                    delay(500);
                    digitalWrite(BUZZER_PIN, LOW);
                    delay(500);
                }
            }
            inputPassword = ""; // Clear password after checking
            lcd.clear();
            lcd.print("Enter Password:");
            lcd.setCursor(0, 1);
        } else {
            inputPassword += key;
        }
    }
}
