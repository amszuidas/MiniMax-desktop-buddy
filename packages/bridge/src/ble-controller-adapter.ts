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
  private bound = false;
  constructor(private readonly link: BleLinkLike, private readonly opts: BleAdapterOptions) {}

  /** Wire device → daemon events. Idempotent: safe to call more than once. */
  bind(): void {
    if (this.bound) return;
    this.bound = true;
    this.link.onEvent((payload) => {
      const ev = decodeEvent(payload);
      if (!ev) return;
      // A device button press is fire-and-forget; a failed daemon decision
      // (network/daemon error) must NOT crash the bridge process via an
      // unhandled rejection. Log and move on.
      Promise.resolve(this.opts.decide(ev.id, eventToDecision(ev.ev))).catch((err: unknown) => {
        // eslint-disable-next-line no-console
        console.error('[BleAdapter] decide failed:', err);
      });
    });
  }

  /** Push a controller state snapshot to the device.
   *  Note (M2): no debounce — Controller only emits on state change, which is
   *  low-frequency. If bursts become an issue, coalesce writes here. */
  handleState(state: BuddyState): void {
    this.link.writeState(encodeState(state));
  }
}
