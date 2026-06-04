import { encodeState, decodeEvent, eventToDecision } from './ble-protocol.js';
import type { BuddyState, Decision } from './types.js';

/** The subset of the BLE link the adapter depends on (for testability). */
export interface BleLinkLike {
  start(): void;
  stop(): void;
  writeState(payload: Buffer): void;
  onEvent(cb: (payload: Buffer) => void): void;
}

export interface BleAdapterOptions {
  /** Resolve a device-local approval id to a daemon decision. */
  decide: (localId: number, decision: Decision) => Promise<void> | void;
}

/**
 * Bridges the M0 Controller to the BLE transport:
 *  - controller state → encodeState → link.writeState
 *  - device event → decodeEvent → options.decide(localId, decision)
 * Pure policy; the actual BLE I/O lives behind BleLinkLike.
 */
export class BleControllerAdapter {
  constructor(private readonly link: BleLinkLike, private readonly opts: BleAdapterOptions) {}

  /** Wire device → daemon events. Call once after construction. */
  bind(): void {
    this.link.onEvent((payload) => {
      const ev = decodeEvent(payload);
      if (!ev) return;
      void this.opts.decide(ev.id, eventToDecision(ev.ev));
    });
  }

  /** Push a controller state snapshot to the device. */
  handleState(state: BuddyState): void {
    this.link.writeState(encodeState(state));
  }
}
