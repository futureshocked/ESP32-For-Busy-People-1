/*  06.021 - Single Seven Segment Display Control for ESP32-S3 (Minimal Version)
 * 
 * Course: IoT Development with ESP32-S3
 * Target Hardware: ESP32-S3-DevKitC-1 v1.1
 * Module: ESP32-S3-WROOM-1-N16R16V
 * 
 * This simple sketch shows you how to drive a single common cathode seven 
 * segment display using the ESP32-S3's digital pins. The program displays 
 * numbers from 0 to 9, plus the decimal point.
 * 
 * HARDWARE INFORMATION:
 * =====================
 * ESP32-S3-DevKitC-1 with ESP32-S3-WROOM-1-N16R16V module
 * - 16 MB Flash Memory, 16 MB PSRAM
 * - Native USB support on GPIO19/20
 * - Updated GPIO pins for ESP32-S3 compatibility
 * 
 * CIRCUIT DIAGRAM:
 * ================
 *     ESP32-S3           Resistor    Seven-Segment Display
 *     ────────           ────────    ──────────────────────
 *     GPIO1  ─────────── [220Ω] ──── A segment (top)
 *     GPIO2  ─────────── [220Ω] ──── B segment (top right)
 *     GPIO19 ─────────── [220Ω] ──── C segment (bottom right)
 *     GPIO18 ─────────── [220Ω] ──── D segment (bottom)
 *     GPIO5  ─────────── [220Ω] ──── E segment (bottom left)
 *     GPIO6  ─────────── [220Ω] ──── F segment (top left)
 *     GPIO14 ─────────── [220Ω] ──── G segment (middle)
 *     GPIO4  ─────────── [220Ω] ──── Dp segment (decimal point)
 *     GND    ─────────────────────── Common cathode
 * 
 * Seven-Segment Layout:
 *         A
 *       ┌───┐
 *    F  │   │  B
 *       │ G │
 *       ├───┤
 *    E  │   │  C
 *       │   │
 *       └───┘  ● Dp
 *         D
 * 
 * COMPONENTS:
 * ===========
 * - 1x ESP32-S3-DevKitC-1 v1.1
 * - 1x Common cathode seven-segment display
 * - 8x 220Ω resistors (current limiting)
 * - 1x Breadboard
 * - 10x Jumper wires
 * 
 * GPIO ASSIGNMENTS:
 * =================
 * A=GPIO1, B=GPIO2, C=GPIO19, D=GPIO18
 * E=GPIO5, F=GPIO6, G=GPIO14, Dp=GPIO4
 * 
 * TECHNICAL NOTES:
 * ================
 * - Updated from original ESP32 pins (GPIO33, GPIO32, GPIO27 not available on ESP32-S3)
 * - Common cathode display: segments turn ON when GPIO is HIGH (3.3V)
 * - 220Ω resistors limit current to safe levels (~15mA per segment)
 * - Bit patterns: .GFEDCBA (bit 7 = Dp, bit 0 = A segment)
 * 
 * IDE REQUIREMENTS:
 * =================
 * - Arduino IDE 2.0+
 * - ESP32 Arduino Core v3.0+
 * - Board: "ESP32S3 Dev Module"
 * 
 * Created: April 1 2019 by Peter Dalmaris
 * Updated: December 2024 for ESP32-S3 compatibility
 */

// ====================================================================
// CONFIGURATION
// ====================================================================

const byte CHAR_COUNT = 11;    // Number of characters to display (0-9 and decimal point)

// Seven-segment display patterns for digits 0-9 and decimal point
// Each byte represents which segments to turn on
// Bit order: .GFEDCBA (Dp, G, F, E, D, C, B, A)
const byte symbols[CHAR_COUNT] = {
    0b00111111,  // 0: A,B,C,D,E,F segments
    0b00000110,  // 1: B,C segments  
    0b01011011,  // 2: A,B,G,E,D segments
    0b01001111,  // 3: A,B,G,C,D segments
    0b01100110,  // 4: F,G,B,C segments
    0b01101101,  // 5: A,F,G,C,D segments
    0b01111101,  // 6: A,F,G,E,D,C segments
    0b00000111,  // 7: A,B,C segments
    0b01111111,  // 8: All segments except Dp
    0b01101111,  // 9: A,B,C,D,F,G segments
    0b10000000   // .: Decimal point only
};

// GPIO pin assignments for ESP32-S3 (updated from original ESP32)
// Pin order: A, B, C, D, E, F, G, Dp
byte segmentPins[] = {1, 2, 19, 18, 5, 6, 14, 4};

// ====================================================================
// SETUP FUNCTION
// ====================================================================

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);
    
    // Wait a moment for serial to initialize
    delay(1000);
    
    Serial.println("ESP32-S3 Seven Segment Display");
    Serial.println("Initializing GPIO pins...");
    
    // Set all segment pins as outputs
    for (int i = 0; i < 8; i++) {
        pinMode(segmentPins[i], OUTPUT);
        digitalWrite(segmentPins[i], LOW);  // Start with all segments off
        Serial.print("GPIO");
        Serial.print(segmentPins[i]);
        Serial.println(" initialized");
    }
    
    Serial.println("Setup complete. Starting display sequence...");
    Serial.println();
    
    // Optional: Test display by showing a specific digit
    // Uncomment the line below to test with digit "8" (all segments on)
    // writeLeds(symbols[8]);  // Display "8" for testing
}

// ====================================================================
// MAIN LOOP
// ====================================================================

void loop() {
    // Display each character from 0 to 9, then decimal point
    for (int i = 0; i < CHAR_COUNT; i++) {
        // Show what we're displaying
        if (i < 10) {
            Serial.print("Displaying digit: ");
            Serial.println(i);
        } else {
            Serial.println("Displaying decimal point");
        }
        
        // Send the pattern to the display
        writeLeds(symbols[i]);
        
        // Wait 1 second before showing next character
        delay(1000);
    }
    
    Serial.println("--- Cycle complete ---");
    Serial.println();
}

// ====================================================================
// DISPLAY CONTROL FUNCTION
// ====================================================================

/**
 * Write a bit pattern to the seven-segment display
 * This function takes a byte where each bit controls one segment
 * 
 * @param pattern: 8-bit value where each bit controls a segment
 *                 bit 0 = A, bit 1 = B, bit 2 = C, bit 3 = D
 *                 bit 4 = E, bit 5 = F, bit 6 = G, bit 7 = Dp
 */
void writeLeds(byte pattern) {
    // Go through each of the 8 segments
    for (int i = 0; i < 8; i++) {
        // Read the bit for this segment from the pattern
        // bitRead(value, bit) returns 1 or 0 for the specified bit
        int segmentState = bitRead(pattern, i);
        
        // Set the GPIO pin HIGH (on) or LOW (off) based on the bit
        digitalWrite(segmentPins[i], segmentState);
    }
}
