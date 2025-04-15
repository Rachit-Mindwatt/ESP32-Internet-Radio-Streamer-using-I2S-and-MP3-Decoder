#include <WiFi.h>
#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

// WiFi credentials
const char* ssid = "Robust_2";
const char* password = "Mudra@fi22#";

// MP3 stream URL
const char* streamURL = "http://stream.radioparadise.com/mp3-128"; //http://ice1.somafm.com/groovesalad

// Audio objects
AudioGeneratorMP3 *mp3;
AudioFileSourceICYStream *file;
AudioFileSourceBuffer *buff;
AudioOutputI2S *out;

void setup() {
  Serial.begin(115200);
  delay(500);

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Optional: Max CPU for decoding
  setCpuFrequencyMhz(240);

  // Set up I2S output (BCLK, LRC, DIN)
  out = new AudioOutputI2S();
  out->SetPinout(15, 16, 17);
  out->SetGain(0.9);
  out->SetChannels(1);       // Mono output for MAX98357A
  out->SetRate(44100);       // Ensure standard sample rate

  // Set up audio stream with larger buffer
  file = new AudioFileSourceICYStream(streamURL);
  buff = new AudioFileSourceBuffer(file, 128 * 1024); // 128KB buffer
  buff->seek(0, SEEK_SET);

  mp3 = new AudioGeneratorMP3();
  mp3->begin(buff, out);
}

void loop() {
  // Check WiFi status
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Reconnecting...");
    WiFi.reconnect();
    delay(1000);
    return;
  }

  // MP3 decoding loop
  if (mp3->isRunning()) {
    if (!mp3->loop()) {
      Serial.println("MP3 decoder stopped. Restarting...");
      mp3->stop();
    }
  } else {
    // Restart streaming if it stopped
    delete mp3;
    delete buff;
    delete file;

    file = new AudioFileSourceICYStream(streamURL);
    buff = new AudioFileSourceBuffer(file, 128 * 1024);
    buff->seek(0, SEEK_SET);

    mp3 = new AudioGeneratorMP3();
    mp3->begin(buff, out);
  }

  // Let background tasks run
  delay(1);
}
