import type { BuddyState, Decision } from './types.js';

/** Fixed 128-bit UUIDs shared by both ends (generated once, permanent). */
export const BLE_UUIDS = {
  service: '6e9c0001-b5a3-4f6e-a4d2-1c8f7e2a0b01',
  state: '6e9c0002-b5a3-4f6e-a4d2-1c8f7e2a0b01',
  event: '6e9c0003-b5a3-4f6e-a4d2-1c8f7e2a0b01',
} as const;

/** Advertised device name the Mac bridge scans for. */
export const BLE_DEVICE_NAME = 'MmxBuddy';

/** Event kinds the device sends back over the Event characteristic. */
export type EventKind = 'approve' | 'always' | 'deny';

/** A decoded device → Mac event. */
export interface DeviceEvent {
  ev: EventKind;
  id: number;
}

/** Encode a BuddyState into compact JSON bytes for the State characteristic. */
export function encodeState(state: BuddyState): Buffer {
  // BuddyState is already compact (single-letter keys); send as-is.
  return Buffer.from(JSON.stringify(state), 'utf8');
}

/** Decode an Event characteristic payload; returns null if malformed. */
export function decodeEvent(buf: Buffer): DeviceEvent | null {
  let obj: unknown;
  try {
    obj = JSON.parse(buf.toString('utf8'));
  } catch {
    return null;
  }
  if (typeof obj !== 'object' || obj === null) return null;
  const o = obj as Record<string, unknown>;
  if (o.ev !== 'approve' && o.ev !== 'always' && o.ev !== 'deny') return null;
  if (typeof o.id !== 'number' || !Number.isInteger(o.id)) return null;
  return { ev: o.ev, id: o.id };
}

/** Map a device event kind to the daemon's Decision verdict. */
export function eventToDecision(ev: EventKind): Decision {
  switch (ev) {
    case 'approve': return 'allowOnce';
    case 'always': return 'allowAlways';
    case 'deny': return 'deny';
  }
}
