import { describe, it, expect } from 'vitest';
import { BLE_UUIDS, BLE_DEVICE_NAME, encodeState, decodeEvent } from '../src/ble-protocol.js';
import type { BuddyState } from '../src/types.js';

describe('BLE protocol', () => {
  it('exposes fixed UUIDs and device name', () => {
    expect(BLE_UUIDS.service).toBe('6e9c0001-b5a3-4f6e-a4d2-1c8f7e2a0b01');
    expect(BLE_UUIDS.state).toBe('6e9c0002-b5a3-4f6e-a4d2-1c8f7e2a0b01');
    expect(BLE_UUIDS.event).toBe('6e9c0003-b5a3-4f6e-a4d2-1c8f7e2a0b01');
    expect(BLE_DEVICE_NAME).toBe('MmxBuddy');
  });

  it('encodeState produces compact JSON bytes under the MTU budget', () => {
    const state: BuddyState = {
      v: 1, c: 1, r: 2, p: 1,
      a: { id: 7, t: 'bash', d: 'rm -rf build/', s: 'ui-fix' },
    };
    const buf = encodeState(state);
    const parsed = JSON.parse(buf.toString('utf8'));
    expect(parsed).toEqual({ v: 1, c: 1, r: 2, p: 1, a: { id: 7, t: 'bash', d: 'rm -rf build/', s: 'ui-fix' } });
    expect(buf.length).toBeLessThanOrEqual(180);
  });

  it('encodeState omits a when there is no current approval', () => {
    const state: BuddyState = { v: 1, c: 1, r: 0, p: 0, a: null };
    const parsed = JSON.parse(encodeState(state).toString('utf8'));
    expect(parsed).toEqual({ v: 1, c: 1, r: 0, p: 0, a: null });
  });

  it('decodeEvent parses approve/always/deny with id', () => {
    expect(decodeEvent(Buffer.from('{"ev":"approve","id":7}', 'utf8'))).toEqual({ ev: 'approve', id: 7 });
    expect(decodeEvent(Buffer.from('{"ev":"always","id":3}', 'utf8'))).toEqual({ ev: 'always', id: 3 });
    expect(decodeEvent(Buffer.from('{"ev":"deny","id":1}', 'utf8'))).toEqual({ ev: 'deny', id: 1 });
  });

  it('decodeEvent returns null on malformed input', () => {
    expect(decodeEvent(Buffer.from('not json', 'utf8'))).toBeNull();
    expect(decodeEvent(Buffer.from('{"ev":"bogus","id":1}', 'utf8'))).toBeNull();
    expect(decodeEvent(Buffer.from('{"ev":"approve"}', 'utf8'))).toBeNull();
  });

  it('eventToDecision maps ev → daemon Decision', async () => {
    const { eventToDecision } = await import('../src/ble-protocol.js');
    expect(eventToDecision('approve')).toBe('allowOnce');
    expect(eventToDecision('always')).toBe('allowAlways');
    expect(eventToDecision('deny')).toBe('deny');
  });
});
