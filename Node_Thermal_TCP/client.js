
const net = require("net");
const WebSocket = require("ws");
const express = require("express");
const http = require("http");
const path = require("path");

const FRAME_WIDTH = 80;
const FRAME_HEIGHT = 62;
const RAW_FRAME_SIZE = FRAME_WIDTH * FRAME_HEIGHT * 2; // 10240 bytes
const STRIP_HEAD = 160;
const STRIP_TAIL = 160;
const TCP_FRAME_SIZE = RAW_FRAME_SIZE + STRIP_HEAD + STRIP_TAIL; // 10560

let receiveBuffer = Buffer.alloc(0);
let client = null;
let reconnectTimeout = null;

const ESP32_HOST = "192.168.x.x"; // your ESP32 IP
const ESP32_PORT = 3333;

// Web UI
const app = express();
app.use(express.static(path.join(__dirname, "public")));
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

function broadcast(frame) {
  for (const client of wss.clients) {
    if (client.readyState === WebSocket.OPEN) {
      client.send(frame);
    }
  }
}

function connectToESP32(retryDelay = 3000) {
  if (client) {
    client.destroy();
    client = null;
  }

  client = new net.Socket();

  client.connect(ESP32_PORT, ESP32_HOST, () => {
    console.log(`✅ Connected to ESP32 at ${ESP32_HOST}:${ESP32_PORT}`);
    receiveBuffer = Buffer.alloc(0); // reset buffer
  });

  client.setTimeout(5000); // optional timeout

  client.on("timeout", () => {
    console.warn("⏱ TCP connection timed out");
    client.destroy(); // will trigger 'close'
  });

  client.on("data", (data) => {
    receiveBuffer = Buffer.concat([receiveBuffer, data]);

    while (receiveBuffer.length >= TCP_FRAME_SIZE) {
      const frame = receiveBuffer.slice(STRIP_HEAD, STRIP_HEAD + RAW_FRAME_SIZE);
      receiveBuffer = receiveBuffer.slice(TCP_FRAME_SIZE);
      broadcast(frame);
    }
  });

  client.on("error", (err) => {
    console.error("❌ TCP Client Error:", err.message);
    client.destroy(); // ensure 'close' fires
  });

  client.on("close", () => {
    console.log("🔌 ESP32 TCP connection closed");
    if (!reconnectTimeout) {
      reconnectTimeout = setTimeout(() => {
        reconnectTimeout = null;
        console.log("🔁 Attempting to reconnect to ESP32...");
        connectToESP32();
      }, retryDelay);
    }
  });
}

connectToESP32(); // initial connection

server.listen(8080, () => {
  console.log("🌐 WebSocket/HTTP server ready at http://localhost:8080");
});
