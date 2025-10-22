# BLE Protocol

## Protocol Overview

The JenLib BLE protocol follows a simple request-response pattern:

1. **Broker** initiates measurement sessions with StartBroadcast messages
2. **Sensor** responds with Reading messages containing measurements
3. **Broker** acknowledges receipt with Receipt messages

## Message Types

- `StartBroadcast` - Broker→Sensor: start a measurement session
- `Reading` - Sensor→Broker: a measurement reading
- `Receipt` - Broker→Sensor: receipt/ack for readings

## Protocol Limits

- Maximum payload size: 64 bytes
- Recommended reading interval: 1000ms
- Protocol version: 1.0

## Message Flow

```
Broker → Sensor: StartBroadcast
Sensor → Broker: Reading (repeated)
Broker → Sensor: Receipt
```
