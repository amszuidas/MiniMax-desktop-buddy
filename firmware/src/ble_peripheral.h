#pragma once
#ifndef UNIT_TEST
#include <NimBLEDevice.h>
#include "ble_protocol.h"

// NimBLE peripheral 封装:广播 + State 写回调(加密)+ Event indication(加密链路)。
namespace buddy_ble {

class BlePeripheral {
 public:
  void begin() {
    NimBLEDevice::init(kDeviceName);
    // Just Works bonding(无屏设备:无输入无输出);加密在 _ENC 特征上强制。
    NimBLEDevice::setSecurityAuth(true, false, true);  // bonding, no MITM, secure connections
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
    // 显式抬高 MTU,容纳 ~173B State 写 + 审批事件(默认 23 会截断)。
    NimBLEDevice::setMTU(247);
    server_ = NimBLEDevice::createServer();
    NimBLEService* svc = server_->createService(kServiceUuid);

    stateChar_ = svc->createCharacteristic(
        kStateUuid, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC);
    stateChar_->setCallbacks(&stateCb_);

    // Event = 设备→Mac 的审批事件,用 INDICATE(带确认,审批不丢)。
    // 加密说明:NimBLE 无 INDICATE_ENC;indication 的加密继承自 bonded link
    //（Mac 先 WRITE_ENC 写 State 触发配对 → 链路已加密后 indication 才流动)。
    // READ_ENC 仅护直接 ATT Read,不强制 indication 订阅方加密——真板需验证
    // 未绑定客户端不能订到明文事件(见 M3-followups / Task9)。
    eventChar_ = svc->createCharacteristic(
        kEventUuid, NIMBLE_PROPERTY::INDICATE | NIMBLE_PROPERTY::READ_ENC);

    svc->start();
    NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
    adv->addServiceUUID(kServiceUuid);
    adv->setName(kDeviceName);
    adv->start();
  }

  /** 最近一次 Mac 写入的状态(若从未收到,ok=false 默认未连接)。
   *  注意:非线程安全——onWrite 在 NimBLE 任务写 latest,本方法在主循环读,
   *  struct 拷贝非原子。M2 单读者(宠物显示)+ 低频写下风险极低、瞬时毛刺无害。
   *  若将来加第二个读者或高频写,需加 mutex/spinlock。 */
  ParsedState latestState() const { return stateCb_.latest; }

  /** 是否有 central 连着。 */
  bool isConnected() const { return server_ && server_->getConnectedCount() > 0; }

  /** 发送一个审批事件给 Mac。 */
  void sendEvent(const char* ev, int id) {
    if (!eventChar_) return;
    char buf[48];
    size_t n = buildEvent(ev, id, buf, sizeof(buf));
    eventChar_->setValue(reinterpret_cast<uint8_t*>(buf), n);
    eventChar_->indicate();
  }

 private:
  class StateCallbacks : public NimBLECharacteristicCallbacks {
   public:
    ParsedState latest;
    void onWrite(NimBLECharacteristic* c) override {
      std::string v = c->getValue();
      ParsedState p = parseState(v.c_str(), v.size());
      if (p.ok) latest = p;
    }
  };

  NimBLEServer* server_ = nullptr;
  NimBLECharacteristic* stateChar_ = nullptr;
  NimBLECharacteristic* eventChar_ = nullptr;
  StateCallbacks stateCb_;
};

}  // namespace buddy_ble
#endif
