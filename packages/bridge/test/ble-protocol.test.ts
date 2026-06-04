import { describe, it, expect } from 'vitest';
import { BLE_UUIDS, BLE_DEVICE_NAME, encodeState, decodeEvent, eventToDecision } from '../src/ble-protocol.js';
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

  it('encodeState byte-caps oversized approval fields to stay within MTU', () => {
    const state: BuddyState = {
      v: 1, c: 1, r: 0, p: 1,
      a: { id: 1, t: 'x'.repeat(100), d: 'y'.repeat(300), s: 'z'.repeat(100) },
    };
    const buf = encodeState(state);
    expect(buf.length).toBeLessThanOrEqual(180);
    // still valid JSON with the expected shape
    const parsed = JSON.parse(buf.toString('utf8'));
    expect(parsed.a.id).toBe(1);
    expect(typeof parsed.a.d).toBe('string');
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
    expect(decodeEvent(Buffer.from('{"ev":"approve","id":1.5}', 'utf8'))).toBeNull();   // float id
    expect(decodeEvent(Buffer.from('{"ev":"approve","id":"7"}', 'utf8'))).toBeNull();   // string id
    expect(decodeEvent(Buffer.from('{"ev":"approve","id":0}', 'utf8'))).toBeNull();     // out of 1..255
    expect(decodeEvent(Buffer.from('{"ev":"approve","id":300}', 'utf8'))).toBeNull();   // out of 1..255
  });

  it('encodeState carries the error flag e when set', () => {
    const state: BuddyState = { v: 1, c: 1, r: 1, p: 0, a: null, e: 1 };
    const parsed = JSON.parse(encodeState(state).toString('utf8'));
    expect(parsed.e).toBe(1);
  });

  it('encodeState omits e when not set (keeps payload small)', () => {
    const state: BuddyState = { v: 1, c: 1, r: 0, p: 0, a: null };
    const parsed = JSON.parse(encodeState(state).toString('utf8'));
    expect(parsed.e).toBeUndefined();
  });

  it('eventToDecision maps ev → daemon Decision', () => {
    expect(eventToDecision('approve')).toBe('allowOnce');
    expect(eventToDecision('always')).toBe('allowAlways');
    expect(eventToDecision('deny')).toBe('deny');
  });
});
