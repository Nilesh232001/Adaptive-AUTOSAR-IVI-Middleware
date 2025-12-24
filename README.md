# Adaptive AUTOSAR IVI Middleware  
**C++17 | Linux (POSIX) | SOME/IP | Multi-Process | Event-Driven Architecture**

## 📌 Overview

This repository contains an **Adaptive AUTOSAR–style Infotainment Middleware** built on **Linux** using **C++17** and **SOME/IP**.  
The middleware provides an automotive-grade communication backbone for **Media**, **Navigation**, and **Climate** services, following the principles of **Service-Oriented Architecture (SOA)** and **event-driven communication** inspired by **AUTOSAR Adaptive Platform**.

The system is implemented as a **multi-process**, **multi-threaded** architecture supporting:

- Dynamic service discovery  
- Asynchronous SOME/IP messaging  
- Persistent state storage  
- POSIX-compliant synchronization  
- Fault-tolerant service handling  

This project is intended as a **learning, research, and demonstration platform** for automotive middleware development.

---

## 🏗️ System Architecture

```
+------------------------------+
|            HMI              |
|  (CLI / IVI Frontend App)   |
+---------------+--------------+
                |
          SOME/IP RPC + Events
                |
+---------------v--------------+
|      Service Manager         |
|  - Discovery                 |
|  - Availability Monitoring   |
|  - Lifecycle Control         |
+-------+-----------+----------+
        |           |
        |           |
+-------v--+   +----v-------+   +------------------+
| Media    |   | Climate    |   | Navigation       |
| Service  |   | Service    |   | Service          |
+----------+   +------------+   +------------------+
```

---

## 🚀 Features

### Core Middleware Features
- Adaptive AUTOSAR–style **Service-Oriented Architecture**
- **SOME/IP client-server and publish-subscribe communication**
- **Dynamic discovery** of services
- **Multi-process architecture** using separate executables
- **Event-based communication** (e.g., new track metadata, navigation updates)
- **POSIX threading** for concurrent operations
- **Fault tolerance** (automatic reconnection to restarted services)

### IVI Services Implemented
#### 🎵 Media Service  
- Playback controls (Play/Pause/Stop)  
- Volume controls  
- Track metadata broadcasting  
- Persistent media state  

#### 🗺️ Navigation Service  
- Destination setting  
- Route status updates (event-based)  
- GPS simulation  
- Persistent navigation state  

#### ❄️ Climate Control Service  
- Temperature + Fan speed controls  
- Mode switching (Auto/Manual)  
- Persistent climate configuration  

---

## 🧩 Communication Model (SOME/IP)

| Communication Type | Usage |
|-------------------|--------|
| **RPC (Request/Response)** | Commands like play(), setTemperature(), setDestination() |
| **Events (Publish/Subscribe)** | Navigation updates, media metadata changes |
| **Service Discovery** | Services announce availability dynamically |

---

## 🛠️ Technologies Used

| Category | Details |
|----------|---------|
| Language | **C++17** |
| OS | **Linux (POSIX)** |
| IPC | **SOME/IP** |
| Build System | **CMake** |
| Threading | `std::thread`, pthreads |
| Data Storage | JSON (nlohmann/json) |
| Architecture | Multi-process, SOA, event-driven |
| Networking | TCP/UDP (SOME/IP) |

---

## 📁 Project Structure

```
Adaptive-AUTOSAR-IVI/
├── cmake/
├── common/
│   ├── logging/
│   ├── someip/              # SOME/IP communication wrapper
│   ├── serialization/
│   └── persistence/
├── service_manager/
├── media_service/
├── navigation_service/
├── climate_service/
├── hmi_client/
├── config/
└── CMakeLists.txt
```

---

## ⚙️ Build Instructions

### 1️⃣ Prerequisites  
Install dependencies:

```
sudo apt update
sudo apt install g++ cmake libpthread-stubs0-dev
```

For SOME/IP support (e.g., vsomeip):

```
sudo apt install libvsomeip3 libvsomeip3-dev
```

---

### 2️⃣ Build the Project

```
git clone https://github.com/your-username/Adaptive-AUTOSAR-IVI.git
cd Adaptive-AUTOSAR-IVI
mkdir build && cd build
cmake ..
make -j$(nproc)
```

---

## ▶️ Running the System

### Step 1 — Start the Service Manager
```
./service_manager
```

### Step 2 — Start the IVI Services
```
./media_service
./navigation_service
./climate_service
```

### Step 3 — Start the HMI Client
```
./hmi_client
```

---

## 💾 Persistent State Management

Each service stores its configuration or last-known state in JSON files.

---

## 🧵 Concurrency Model

Each service launches multiple threads for:
- SOME/IP communication  
- Event publishing  
- State storage  

---

## 🧪 Testing & Validation

- Serialization tests  
- Multi-process integration tests  
- Fault injection (restart services)  

---

## 📈 Future Enhancements

- AUTOSAR Adaptive Manifest integration  
- Security extensions (TLS for SOME/IP)  
- Qt-based HMI  
- DDS communication backend  

---

## 📜 License

MIT License 

---

## 👤 Author

Nilesh Patil 
Automotive Embedded / Middleware Engineer