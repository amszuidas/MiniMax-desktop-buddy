#pragma once
#ifndef UNIT_TEST
#include <NimBLEDevice.h>
#include "ble_protocol.h"

// NimBLE peripheral 封装:广播 + State 写回调 + Event 通知。
namespace buddy_ble {

class BlePeripheral {
 public:
  void begin() {
    NimBLEDevice::init(kDeviceName);
    server_ = NimBLEDevice::createServer();
    NimBLEService* svc = server_->createService(kServiceUuid);

    stateChar_ = svc->createCharacteristic(kStateUuid, NIMBLE_PROPERTY::WRITE);
    stateChar_->setCallbacks(&stateCb_);

    eventChar_ = svc->createCharacteristic(kEventUuid, NIMBLE_PROPERTY::NOTIFY);

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
    eventChar_->notify();
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
