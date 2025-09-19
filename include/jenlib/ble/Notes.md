# Messaging between broker and sensors

## Overview
Communication between Broker and Sensor is handled over BLE using a custom GATT
service (distinct UUID per compatible version). The Sensor remains idle with a
default `SessionId` until the Broker initiates a session.

Start of session: The Broker writes a StartBroadcast message (containing
`SessionId` and target `DeviceId`) to the Sensor's Control characteristic. The
Sensor stores the `SessionId`, starts a millisecond offset timer, and begins
emitting Reading notifications at the configured cadence. Readings are buffered
locally until the Sensor receives a Receipt message acknowledging
`up_to_offset_ms`, after which acknowledged readings can be cleared.

Broker processing: The Broker receives Reading notifications, validates the
`DeviceId` (CRC-8) and associates data with active sessions (lookup DeviceId against SessionId table). It may batch and forward readings to a backend (e.g., HTTPS/JSON) on a schedule.

## Handling auth
Optionally, an authentication step (e.g., key exchange → JWT) may occur during
session setup. No secure enclave is assumed; the protocol design keeps space to
add this later without changing message formats.

## Communication layers
- Physical/data link: BLE
- Network: many-to-one (Sensors → Broker), one-to-one (Broker → Sensor)
- Transport: GATT (Control/Receipt via Write, Reading via Notify/Indicate)
- Session: set by Broker via StartBroadcast (`SessionId`)
- Presentation: binary messages (little-endian); `DeviceId` with CRC-8;
  temperature in centi-degrees Celsius; humidity in basis points; time offsets
  in milliseconds
- Application: message APIs in `Messages.h` and `Ble.h`; fixed buffer in
  `Payload.h`
  