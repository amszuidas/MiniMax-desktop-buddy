import { BLE_UUIDS, BLE_DEVICE_NAME } from './ble-protocol.js';
import type { BleLinkLike } from './ble-controller-adapter.js';

// noble has no bundled types; load via dynamic import in start().
type NobleCharacteristic = {
  writeAsync(data: Buffer, withoutResponse: boolean): Promise<void>;
  subscribeAsync(): Promise<void>;
  on(event: 'data', cb: (data: Buffer) => void): void;
  uuid: string;
};

/** strip dashes — noble compares UUIDs lowercased without dashes. */
function nobleUuid(u: string): string {
  return u.replace(/-/g, '').toLowerCase();
}

/**
 * noble BLE central: scans for the MmxBuddy peripheral, connects, discovers the
 * State (write) and Event (notify) characteristics, and exposes writeState /
 * onEvent. Auto-reconnects on disconnect. I/O boundary — verified live, not unit-tested.
 */
export class BleLink implements BleLinkLike {
  private stateChar: NobleCharacteristic | null = null;
  private eventCb: ((payload: Buffer) => void) | null = null;
  private connectCb: (() => void) | null = null;
  private stopped = false;
  // eslint-disable-next-line @typescript-eslint/no-explicit-any -- noble has no types
  private noble: any = null;

  start(): void {
    void this.run();
  }

  stop(): void {
    this.stopped = true;
    try { this.noble?.stopScanning?.(); } catch { /* ignore */ }
  }

  writeState(payload: Buffer): void {
    // fire-and-forget; State characteristic is write-with-response
    void this.stateChar?.writeAsync(payload, false).catch(() => { /* link dropped; reconnect loop handles it */ });
  }

  onEvent(cb: (payload: Buffer) => void): void {
    this.eventCb = cb;
  }

  /** Called each time the device connects (initial or reconnect). */
  onConnect(cb: () => void): void {
    this.connectCb = cb;
  }

  private async run(): Promise<void> {
    const mod = await import('@abandonware/noble');
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    this.noble = (mod as any).default ?? mod;
    try { this.noble.reset?.(); } catch { /* reset unavailable on some platforms */ }

    this.noble.on('stateChange', (s: string) => {
      if (s === 'poweredOn' && !this.stopped) void this.noble.startScanningAsync([], false);
    });

    this.noble.on('discover', async (peripheral: any) => {
      if (peripheral.advertisement?.localName !== BLE_DEVICE_NAME) return;
      try {
        await this.noble.stopScanningAsync();
        await peripheral.connectAsync();
        const { characteristics } = await peripheral.discoverSomeServicesAndCharacteristicsAsync(
          [nobleUuid(BLE_UUIDS.service)],
          [nobleUuid(BLE_UUIDS.state), nobleUuid(BLE_UUIDS.event)],
        );
        for (const ch of characteristics as NobleCharacteristic[]) {
          if (ch.uuid === nobleUuid(BLE_UUIDS.state)) this.stateChar = ch;
          if (ch.uuid === nobleUuid(BLE_UUIDS.event)) {
            ch.on('data', (data: Buffer) => this.eventCb?.(data));
            await ch.subscribeAsync();
          }
        }
        peripheral.once('disconnect', () => {
          this.stateChar = null;
          if (!this.stopped) void this.noble.startScanningAsync([], false);
        });
        // eslint-disable-next-line no-console
        console.log('[ble] connected to MmxBuddy');
        this.connectCb?.();
      } catch (err) {
        // eslint-disable-next-line no-console
        console.error('[ble] connect failed, rescanning:', String(err));
        if (!this.stopped) void this.noble.startScanningAsync([], false);
      }
    });
  }
}
