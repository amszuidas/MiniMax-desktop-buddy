#pragma once
#ifndef UNIT_TEST
#include <ArduinoJson.h>
#include "pet_state.h"

// 与 packages/bridge/src/ble-protocol.ts 逐字段对应的契约。两端必须一致。
namespace buddy_ble {

// 固定 128-bit UUID(与 TS 端 BLE_UUIDS 相同)
constexpr const char* kServiceUuid = "6e9c0001-b5a3-4f6e-a4d2-1c8f7e2a0b01";
constexpr const char* kStateUuid   = "6e9c0002-b5a3-4f6e-a4d2-1c8f7e2a0b01";
constexpr const char* kEventUuid   = "6e9c0003-b5a3-4f6e-a4d2-1c8f7e2a0b01";
constexpr const char* kDeviceName  = "MmxBuddy";

// 解析后的状态(供 main 喂状态机 + 显示当前审批)
struct ParsedState {
  bool ok = false;          // 解析是否成功
  buddy::PetInputs inputs;  // connected/running/pending
  bool hasApproval = false;
  int approvalId = 0;       // localId
  char toolName[32] = {0};  // 容纳 Mac 端 ≤24 字节
  char desc[64] = {0};      // 容纳 Mac 端 ≤60 字节
};

// 解析 State 包(Mac→设备)。失败时 ok=false。
inline ParsedState parseState(const char* json, size_t len) {
  ParsedState s;
  JsonDocument doc;
  if (deserializeJson(doc, json, len)) return s;  // 解析错误 → ok=false
  s.inputs.connected = (doc["c"] | 0) == 1;
  s.inputs.runningSessions = doc["r"] | 0;
  s.inputs.pendingApprovals = doc["p"] | 0;
  if (!doc["a"].isNull()) {
    s.hasApproval = true;
    s.approvalId = doc["a"]["id"] | 0;
    const char* t = doc["a"]["t"] | "";
    const char* d = doc["a"]["d"] | "";
    snprintf(s.toolName, sizeof(s.toolName), "%s", t);
    snprintf(s.desc, sizeof(s.desc), "%s", d);
  }
  s.ok = true;
  return s;
}

// 构建 Event 包(设备→Mac)。ev ∈ {"approve","always","deny"}。写入 out,返回长度。
inline size_t buildEvent(const char* ev, int id, char* out, size_t outSize) {
  JsonDocument doc;
  doc["ev"] = ev;
  doc["id"] = id;
  return serializeJson(doc, out, outSize);
}

}  // namespace buddy_ble
#endif
