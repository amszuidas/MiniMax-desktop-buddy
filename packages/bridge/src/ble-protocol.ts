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

/** Max UTF-8 bytes for the whole State payload (BLE MTU budget). */
const MAX_STATE_BYTES = 180;
/** Per-field byte caps for the approval fields (defense-in-depth; upstream also truncates). */
const MAX_TOOL_BYTES = 24;
const MAX_DESC_BYTES = 60;
const MAX_SESS_BYTES = 24;

/** Truncate a string so its UTF-8 encoding is at most maxBytes, without splitting a multi-byte char. */
function truncateUtf8(s: string, maxBytes: number): string {
  const buf = Buffer.from(s, 'utf8');
  if (buf.length <= maxBytes) return s;
  // Walk back from maxBytes until we're not in the middle of a multi-byte sequence.
  let end = maxBytes;
  while (end > 0 && ((buf[end] ?? 0) & 0xc0) === 0x80) end--;
  return buf.toString('utf8', 0, end);
}

/** Encode a BuddyState into compact JSON bytes for the State characteristic.
 *  Approval string fields are byte-capped so the payload stays within the BLE
 *  MTU budget even if upstream truncation changes.
 *  Note: when a is null, JSON carries "a":null (not an absent key); the device
 *  firmware must treat a==null as "no approval". */
export function encodeState(state: BuddyState): Buffer {
  const safe: BuddyState = state.a
    ? {
        ...state,
        a: {
          id: state.a.id,
          t: truncateUtf8(state.a.t, MAX_TOOL_BYTES),
          d: truncateUtf8(state.a.d, MAX_DESC_BYTES),
          s: truncateUtf8(state.a.s, MAX_SESS_BYTES),
        },
      }
    : state;
  return Buffer.from(JSON.stringify(safe), 'utf8');
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
  if (typeof o.id !== 'number' || !Number.isInteger(o.id) || o.id < 1 || o.id > 255) return null;
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
