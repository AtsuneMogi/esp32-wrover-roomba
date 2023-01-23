#include <Arduino.h>
#include <AsyncUDP.h>
#include <vector>
#include <WiFi.h>

#define btnUp 23
#define btnDown 22

#define sensorF 13
#define sensorB 14
#define sensorFL 25
#define sensorFR 26
#define sensorBL 32
#define sensorBR 33


std::vector<int> seg = {0, 21, 19, 18, 5, 4, 15, 2}; // dp, g, f, e, d, c, b, a
std::vector<std::vector<int>> numbers = {
    {0, 0, 1, 1, 1, 1, 1, 1}, // 0
    {0, 0, 0, 0, 0, 1, 1, 0}, // 1
    {0, 1, 0, 1, 1, 0, 1, 1}, // 2
    {0, 1, 0, 0, 1, 1, 1, 1}, // 3
    {0, 1, 1, 0, 0, 1, 1, 0}, // 4
    {0, 1, 1, 0, 1, 1, 0, 1}, // 5
    {0, 1, 1, 1, 1, 1, 0, 1}, // 6
    {0, 0, 0, 0, 0, 1, 1, 1}, // 7
    {0, 1, 1, 1, 1, 1, 1, 1}, // 8
    {0, 1, 1, 0, 1, 1, 1, 1}, // 9
    {1, 0, 0, 0, 0, 0, 0, 0}  // dp
};

char buf[1];        // direction command
bool brush = false; // brush flag
bool music;         // music flag

int v = 150; // velocity
// super mario
std::vector<int> oneUp = {140, 2, 6, 84, 5, 91, 5, 100, 5, 96, 5, 98, 5, 103, 5, 141, 2};

char ssid[] = "M5StickC-Controller";
char pass[] = "controller";
AsyncUDP udp; // udp instance
unsigned int port = 8888; // local port to listen on


void displayNumber(int n) {
    for (int i = 0; i < 8; i++)
        digitalWrite(seg[i], numbers[n][i]);
}


void displaySpeed() {
    if (v == 0) displayNumber(0);
    else if (v == 50) displayNumber(1);
    else if (v == 100) displayNumber(2);
    else if (v == 150) displayNumber(3);
    else if (v == 200) displayNumber(4);
    else displayNumber(5);
}

// devide integer to 8 bits
unsigned int hex_convert_to16(int a, int b) {
    return (unsigned int)(a << 8) | (int)(b);
}


unsigned int hex_convert_to8_high(int a) {
    return (unsigned int)(a >> 8) & 0x00FF;
}


unsigned int hex_convert_to8_low(int a) {
    return a ^ (hex_convert_to8_high(a) << 8);
}


void roomba_send_num(int num) { // devide into two 8-bit commands
    Serial1.write(hex_convert_to8_high(num));
    Serial1.write(hex_convert_to8_low(num));
}


void roomba_drive(int left, int right) { // go advance
    Serial1.write(byte(145));
    roomba_send_num(right); // velocity right
    roomba_send_num(left);  // velocity left
    delay(100);
}


void stop() {
    Serial1.write(137);
    roomba_send_num(0); // velocity 0mm/s
    roomba_send_num(0); // radius
    delay(100);
}


void roomba_drive_turn_counterclockwise(int num) {
    Serial1.write(137);
    roomba_send_num(num); // velocity 100mm/s
    roomba_send_num(1);   // radius
    delay(100);
}


void roomba_drive_turn_clockwise(int num) {
    Serial1.write(137);
    roomba_send_num(num); // velocity
    roomba_send_num(-1);  // radius
    delay(100);
}


void send_data(std::vector<int> &arr) {
    for (int i = 0; i < arr.size(); i++) {
        Serial1.write(arr[i]);
    }
}


void yobikomi() {
    std::vector<std::vector<int>> arr2 = {
        {140, 1, 6, 81, 14, 81, 28, 83, 14, 81, 14, 78, 14, 81, 28, 141, 1},
        {141, 1},
        {140, 1, 6, 74, 14, 74, 14, 74, 14, 76, 14, 78, 42, 74, 14, 141, 1},
        {140, 1, 3, 78, 42, 81, 14, 81, 56, 141, 1},
        {140, 1, 5, 74, 14, 74, 14, 74, 14, 76, 14, 78, 56, 141, 1},
        {141, 1},
        {140, 1, 6, 76, 14, 76, 14, 76, 14, 74, 14, 76, 28, 78, 28, 141, 1},
        {140, 1, 4, 81, 28, 79, 28, 78, 28, 76, 28, 141, 1},
        {140, 1, 6, 81, 14, 81, 28, 83, 14, 81, 14, 78, 14, 81, 28, 141, 1},
        {140, 1, 6, 81, 14, 81, 28, 83, 14, 81, 14, 78, 14, 76, 28, 141, 1},
        {140, 1, 1, 74, 60, 141, 1}};
    for (int i = 0; i < arr2.size(); i++) {
        std::vector<int> arr1 = arr2[i];
        send_data(arr1);
        delay(1800);
    }
}


void roomba_setup() {
    Serial1.begin(115200, SERIAL_8N1, 3, 1); // roomba, tx: 1, rx: 3
    Serial1.write(128);
    delay(50);
    Serial1.write(132);
    delay(50);
    send_data(oneUp);
}


void roomba_end() {
    Serial1.write(128);
    Serial1.write(132);
    Serial1.write(128);
    Serial1.write(173);
    Serial1.end();
}


void setup() {
    pinMode(btnUp, INPUT_PULLUP);
    pinMode(btnDown, INPUT_PULLUP);
    pinMode(sensorF, INPUT);
    pinMode(sensorB, INPUT);
    pinMode(sensorFL, INPUT);
    pinMode(sensorFR, INPUT);
    pinMode(sensorBL, INPUT);
    pinMode(sensorBR, INPUT);
    for (int i = 0; i < 8; i++) pinMode(seg[i], OUTPUT);

    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    delay(100);

    Serial.print("Connecting...");

    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.print(".");
        for (int i = 0; i < 11; i++) {
            displayNumber(i);
            delay(100);
        }
    }
    Serial.println("");

    roomba_setup();

    if (udp.listen(port)) {
        udp.onPacket([](AsyncUDPPacket packet) {
            buf[0] = (char)*(packet.data()); });
    }
}


void loop() {
    if (!digitalRead(btnDown) && 0 < v) { // min: 0
        if (!digitalRead(btnUp)) {
            roomba_end();
        }
        stop();
        v -= 50;
        music = false;
        delay(1000);
    } else if (!digitalRead(btnUp) && v < 250) { // max: 250
        if (!digitalRead(btnDown)) {
            roomba_end();
        }
        stop();
        v += 50;
        music = false;
        delay(1000);
    } else if (v == 0 && music == false) {
        delay(100);
        yobikomi();
        music = true;
    }

    switch (buf[0]) {
        case 'a':
            if (!digitalRead(sensorF) + !digitalRead(sensorFL) + !digitalRead(sensorFR)) {
                Serial.print("front");
                stop();
            } else {
                roomba_drive(v, v); // go advance
            }
            break;
        case 'A':
            if (!digitalRead(sensorF) + !digitalRead(sensorFL) + !digitalRead(sensorFR)) {
                stop();
            } else {
                roomba_drive(2 * v, 2 * v); // go advance
            }
            break;
        case 'b':
            if (!digitalRead(sensorB) + !digitalRead(sensorBL) + !digitalRead(sensorBR)) {
                stop();
            } else {
                roomba_drive(-v, -v);
            }
            break;
        case 'B':
            if (!digitalRead(sensorB) + !digitalRead(sensorBL) + !digitalRead(sensorBR)) {
                stop();
            } else {
                roomba_drive(-2 * v, -2 * v);
            }
            break;
        case 'c':
            if (!digitalRead(sensorF) + !digitalRead(sensorFL)) {
                stop();
            } else {
                roomba_drive(v / 5, v);
            }
            break;
        case 'C':
            if (!digitalRead(sensorF) + !digitalRead(sensorFL)) {
                stop();
            } else {
                roomba_drive(2 * v / 5, 2 * v);
            }
            break;
        case 'd':
            if (!digitalRead(sensorF) + !digitalRead(sensorFR)) {
                stop();
            } else {
                roomba_drive(v, v / 5);
            }
            break;
        case 'D':
            if (!digitalRead(sensorF) + !digitalRead(sensorFR)) {
                stop();
            } else {
                roomba_drive(2 * v, 2 * v / 5);
            }
            break;
        case 'e':
            if (!digitalRead(sensorB) + !digitalRead(sensorBL)) {
                stop();
            } else {
                roomba_drive(-v / 5, -v);
            }
            break;
        case 'E':
            if (!digitalRead(sensorB) + !digitalRead(sensorBL)) {
                stop();
            } else {
                roomba_drive(-2 * v / 5, -2 * v);
            }
            break;
        case 'f':
            if (!digitalRead(sensorB) + !digitalRead(sensorBR)) {
                stop();
            } else {
                roomba_drive(-v, -v / 5);
            }
            break;
        case 'F':
            if (!digitalRead(sensorB) + !digitalRead(sensorBR)) {
                stop();
            } else {
                roomba_drive(-2 * v, -2 * v / 5);
            }
            break;
        case 'g':
            roomba_drive_turn_counterclockwise(v); // turn counterclockwise
            break;
        case 'G':
            roomba_drive_turn_counterclockwise(2 * v); // turn counterclockwise
            break;
        case 'h':
            roomba_drive_turn_clockwise(v); // turn clockwise
            break;
        case 'H':
            roomba_drive_turn_clockwise(2 * v); // turn clockwise
            break;
        case 'I':
            send_data(oneUp);
            break;
        case 'J':
            if (!brush) {
                Serial1.write(138);
                Serial1.write(7);
                brush = true;
                delay(500);
            } else if (brush) {
                Serial1.write(138);
                Serial1.write(0);
                brush = false;
                delay(500);
            }
            break;
        case 'K':
            stop();
            break;
        default:
            stop();
            break;
    }
    displaySpeed();

    if (WiFi.status() != WL_CONNECTED) {
        roomba_end();
        while (WiFi.waitForConnectResult() != WL_CONNECTED) {
            Serial.print(".");
            for (int i = 0; i < 11; i++) {
                displayNumber(i);
                delay(100);
            }
        }
        roomba_setup();
        v = 150;
    }

    delay(100);
}

