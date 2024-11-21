# TELEPHONE RECHARGE MACHINE API

## Status Codes

1. “est” → Error Status
2. “ety” → Error Type
3. “mty” → Message Type
4. “mid” → Machine ID
5. “bl” → Card Balance
6. “amt” → Amount
7. “st” → Status

## Success Message Types

1. 1 → machine id read success
2. 2 → RFID card balance read success
3. 3 → RFID card recharge success

## Error Types

1. 1 → JSON decode error
2. 2 → RFID card not found error
3. 3 → failed to read the RFID card ID
4. 4 → Invalid RFID card type
5. 5 → failed to authenticate the RFID card
6. 6 → failed to read the RFID card memory
7. 7 → failed to write the RFID card memory
8. 8 → invalid option

## 1. Getting Machine ID

Request Format

```json
{ "st": 1 }
```

Response format

If no error occurres

```json
{ "est": 0, "mty": 1, "mid": "vs24rm01" }
```

if error occurres

```json
{ "est": 1, "ety": 1 }
```

**.**

**.**

**.**

## 2. Get RFID Card Balance

Request Format

```json
{ "st": 2 }
```

Response format

If no error occurres

```json
{ "est": 0, "mty": 2, "bl": 900 }
```

## 3. Recharge RFID Card

```json
{ "st": 3, "amt": 800 }
```

Response format

If no error occurres

```json
{ "est": 0, "mty": 3 }
```
