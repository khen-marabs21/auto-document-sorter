#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Khen";
const char* password = "Akopaba21";

WebServer server(80);

#define RXD2 16
#define TXD2 17

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <title>Auto Document Sorter OCR</title>
  <script src="https://cdn.jsdelivr.net/npm/tesseract.js@5/dist/tesseract.min.js"></script>
  <style>
    * { box-sizing: border-box; }
    body { font-family: -apple-system, BlinkMacSystemFont, sans-serif; text-align: center; margin: 0; padding: 20px; background: #121212; color: #fff; }
    h2 { font-size: 20px; margin-bottom: 20px; }
    
    .btn-capture {
      display: block;
      width: 100%;
      max-width: 320px;
      margin: 20px auto;
      padding: 18px;
      font-size: 18px;
      font-weight: bold;
      color: #fff;
      background: #007aff;
      border: none;
      border-radius: 12px;
      cursor: pointer;
    }
    
    #fileInput { display: none; }
    
    #previewImg {
      max-width: 320px;
      max-height: 240px;
      margin: 10px auto;
      border-radius: 8px;
      display: none;
      border: 2px solid #444;
    }

    #statusBox {
      background: #222;
      margin: 15px auto;
      max-width: 320px;
      padding: 12px;
      border-radius: 8px;
      font-size: 15px;
      font-weight: bold;
      color: #ffcc00;
    }

    #ocrPreview {
      max-width: 320px;
      margin: 10px auto;
      background: #000;
      border: 1px solid #333;
      color: #00ff66;
      font-size: 13px;
      text-align: left;
      padding: 10px;
      border-radius: 6px;
      min-height: 50px;
      white-space: pre-wrap;
    }
    canvas { display: none; }
  </style>
</head>
<body>
  <h2>Document Sorter OCR</h2>

  <label for="fileInput" class="btn-capture">SNAP & SORT DOCUMENT</label>
  <input type="file" id="fileInput" accept="image/*" capture="environment">

  <img id="previewImg" alt="Captured Document">
  <div id="statusBox">Loading OCR Engine...</div>
  <pre id="ocrPreview">Detected text will appear here...</pre>
  <canvas id="procCanvas"></canvas>

  <script>
    const fileInput = document.getElementById('fileInput');
    const previewImg = document.getElementById('previewImg');
    const statusBox = document.getElementById('statusBox');
    const ocrPreview = document.getElementById('ocrPreview');
    const canvas = document.getElementById('procCanvas');
    const ctx = canvas.getContext('2d');

    let worker = null;

    async function initOCR() {
      worker = await Tesseract.createWorker('eng');
      statusBox.innerText = "READY: Tap button to snap document";
      statusBox.style.color = "#00ff66";
    }

    // Automatically runs the instant a photo is taken
    fileInput.addEventListener('change', async function(e) {
      const file = e.target.files[0];
      if (!file || !worker) return;

      statusBox.innerText = "Processing image...";
      statusBox.style.color = "#ffcc00";

      const reader = new FileReader();
      reader.onload = function(evt) {
        previewImg.src = evt.target.result;
        previewImg.style.display = 'block';

        const img = new Image();
        img.onload = async function() {
          // Crop the top 40% of the page where headings sit to remove noise
          const cropW = img.width;
          const cropH = img.height * 0.40;

          canvas.width = cropW;
          canvas.height = cropH;

          // Draw cropped top section
          ctx.drawImage(img, 0, 0, cropW, cropH, 0, 0, cropW, cropH);

          // Contrast Boost (High threshold black/white) to make text sharp
          const imgData = ctx.getImageData(0, 0, cropW, cropH);
          const data = imgData.data;
          for (let i = 0; i < data.length; i += 4) {
            let gray = data[i] * 0.299 + data[i + 1] * 0.587 + data[i + 2] * 0.114;
            let val = gray > 130 ? 255 : 0;
            data[i] = val;
            data[i + 1] = val;
            data[i + 2] = val;
          }
          ctx.putImageData(imgData, 0, 0);

          statusBox.innerText = "Analyzing text...";
          const { data: { text } } = await worker.recognize(canvas);
          const clean = text.toUpperCase().replace(/[^A-Z0-9\s]/g, " ");
          ocrPreview.innerText = clean.trim() || "[No text found]";

          // Keyword triggers with high tolerance
          const isORF = clean.includes("OFFICIAL REGISTRATION") || 
                        clean.includes("REGISTRATION") || 
                        (clean.includes("OFFICIAL") && clean.includes("REG"));

          const isSOG = clean.includes("GRADE SUMMARY") || 
                        clean.includes("SUMMARY OF GRADES") || 
                        clean.includes("Credit") ||
                        clean.includes("GRADE") || 
                        clean.includes("SUMMARY");

          if (isORF) {
            triggerESP("ORF", "OFFICIAL REGISTRATION (ORF)");
          } else if (isSOG) {
            triggerESP("SOG", "GRADE SUMMARY (SOG)");
          } else {
            statusBox.innerText = "No match found. Retake photo closer to header.";
            statusBox.style.color = "#ff4444";
          }
          
          // Clear input so the same file can be scanned again if needed
          fileInput.value = "";
        };
        img.src = evt.target.result;
      };
      reader.readAsDataURL(file);
    });

    function triggerESP(type, title) {
      statusBox.innerText = title + " MATCHED! Sorting...";
      statusBox.style.color = type === "ORF" ? "#00ff66" : "#00ccff";

      fetch('/' + type.toLowerCase())
        .then(() => {
          setTimeout(() => {
            statusBox.innerText = "READY: Tap button to snap document";
            statusBox.style.color = "#00ff66";
          }, 3500);
        })
        .catch(() => {
          statusBox.innerText = "Error contacting ESP32.";
        });
    }

    window.onload = initOCR;
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");
  Serial.print("Open this in Safari: http://");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });

  server.on("/orf", HTTP_GET, []() {
    Serial.println(">>> Command: ORF");
    Serial2.println("ORF");
    server.send(200, "text/plain", "OK");
  });

  server.on("/sog", HTTP_GET, []() {
    Serial.println(">>> Command: SOG");
    Serial2.println("SOG");
    server.send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  server.handleClient();
}