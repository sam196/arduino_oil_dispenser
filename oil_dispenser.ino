#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <EEPROM.h>

#define MOSFET_PIN 2
#define INTERRUPT_BUTTON 3
#define EEPROM_SALES_ADDR 0  // Store total sales at address 0
#define EEPROM_PASSWORD_ADDR 10 // Store password at address 10

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {7, 6, 5, 4};
byte colPins[COLS] = {11, 10, 9, 8};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String password = "1095";
String inputPassword = "";
String enteredAmount = "";
unsigned int totalSales = 0;

void setup() {
    pinMode(MOSFET_PIN, OUTPUT);
    pinMode(INTERRUPT_BUTTON, INPUT_PULLUP);
    lcd.init();
    lcd.backlight();
    scrollMessage("Samtronics Solutions");

    // Load saved password from EEPROM
    char savedPassword[5];
    for (int i = 0; i < 4; i++) {
        savedPassword[i] = EEPROM.read(EEPROM_PASSWORD_ADDR + i);
    }
    savedPassword[4] = '\0';
    
    // Validate stored password
    if (String(savedPassword) != "" && String(savedPassword).length() == 4) {
        password = String(savedPassword);
    } else {
        // If EEPROM has invalid data, store default password
        for (int i = 0; i < 4; i++) {
            EEPROM.write(EEPROM_PASSWORD_ADDR + i, password[i]);
        }
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter Password:");
    inputPassword = "";
    while (true) {
        char key = keypad.getKey();
        if (key) {
            inputPassword += key;
            lcd.setCursor(inputPassword.length() - 1, 1);
            lcd.print(key); // Display entered password character
            if (inputPassword.length() == 4) {
                if (inputPassword.equals(password)) {
                    lcd.clear();
                    lcd.print("Access Granted");
                    delay(1000);
                    lcd.clear();
                    break;
                } else {
                    lcd.clear();
                    lcd.print("Password Error");
                    inputPassword = "";
                    delay(1000);
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("Enter Password:");
                }
            }
        }
    }
}

void loop() {
    lcd.setCursor(0, 0);
    lcd.print("Enter Amount:");
    enteredAmount = "";
    
    while (true) {
        char key = keypad.getKey();
        if (key) {
            if (key == '#') break; // End entry when '#' is pressed
            enteredAmount += key;
            lcd.setCursor(enteredAmount.length(), 1);
            lcd.print(key);
        }
    }
    
    if (enteredAmount == "0000") {
        resetPassword();
        return;
    }

    lcd.clear();
    lcd.print("Press Btn to");
    lcd.setCursor(0, 1);
    lcd.print("Dispense");
    
    while (digitalRead(INTERRUPT_BUTTON) == HIGH);
    lcd.clear();
    lcd.print("Dispensing...");
    digitalWrite(MOSFET_PIN, HIGH);
    delay(enteredAmount.toInt() * 100); // Control dispensing time based on amount
    digitalWrite(MOSFET_PIN, LOW);
    lcd.clear();
    lcd.print("Done");
    delay(1000);

    updateSales(enteredAmount.toInt());
}

void scrollMessage(String message) {
    for (int i = 0; i < message.length() + 16; i++) {
        lcd.clear();
        lcd.setCursor(16 - i, 0);
        lcd.print(message.substring(max(0, i - 16), i));
        delay(300);
    }
}

void updateSales(int amount) {
    totalSales += amount;
    EEPROM.put(EEPROM_SALES_ADDR, totalSales);
}

void resetPassword() {
    lcd.clear();
    lcd.print("New Password:");
    String newPassword = "";
    while (true) {
        char key = keypad.getKey();
        if (key) {
            newPassword += key;
            lcd.setCursor(newPassword.length() - 1, 1);
            lcd.print(key); // Display entered password character
            if (newPassword.length() == 4) {
                password = newPassword;
                for (int i = 0; i < 4; i++) {
                    EEPROM.write(EEPROM_PASSWORD_ADDR + i, password[i]);
                }
                lcd.clear();
                lcd.print("Password Set");
                delay(1000);
                return;
            }
        }
    }
}
