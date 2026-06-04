import { describe, it, expect, vi } from 'vitest';
import { BleControllerAdapter } from '../src/ble-controller-adapter.js';
import type { BuddyState } from '../src/types.js';

function sampleState(over: Partial<BuddyState> = {}): BuddyState {
  return { v: 1, c: 1, r: 0, p: 0, a: null, ...over };
}

describe('BleControllerAdapter', () => {
  it('writes encoded state to the link whenever controller emits state', () => {
    const writes: Buffer[] = [];
    const link = {
      writeState: (b: Buffer) => { writes.push(b); },
      onEvent: vi.fn(),
      start: vi.fn(),
      stop: vi.fn(),
    };
    const decide = vi.fn();
    const adapter = new BleControllerAdapter(link as never, { decide });

    adapter.handleState(sampleState({ p: 1, a: { id: 5, t: 'read', d: '/etc/x', s: 'sess' } }));
    expect(writes.length).toBe(1);
    expect(JSON.parse(writes[0]!.toString('utf8'))).toEqual(
      { v: 1, c: 1, r: 0, p: 1, a: { id: 5, t: 'read', d: '/etc/x', s: 'sess' } },
    );
  });

  it('on a device approve event calls decide(id, allowOnce)', async () => {
    const decide = vi.fn().mockResolvedValue(undefined);
    let emit!: (buf: Buffer) => void;
    const link = {
      writeState: vi.fn(),
      onEvent: (cb: (buf: Buffer) => void) => { emit = cb; },
      start: vi.fn(),
      stop: vi.fn(),
    };
    const adapter = new BleControllerAdapter(link as never, { decide });
    adapter.bind();
    emit(Buffer.from('{"ev":"approve","id":9}', 'utf8'));
    await Promise.resolve();
    expect(decide).toHaveBeenCalledWith(9, 'allowOnce');
  });

  it('maps always→allowAlways and deny→deny', async () => {
    const decide = vi.fn().mockResolvedValue(undefined);
    let emit!: (buf: Buffer) => void;
    const link = { writeState: vi.fn(), onEvent: (cb: (b: Buffer) => void) => { emit = cb; }, start: vi.fn(), stop: vi.fn() };
    const adapter = new BleControllerAdapter(link as never, { decide });
    adapter.bind();
    emit(Buffer.from('{"ev":"always","id":2}', 'utf8'));
    emit(Buffer.from('{"ev":"deny","id":4}', 'utf8'));
    await Promise.resolve();
    expect(decide).toHaveBeenCalledWith(2, 'allowAlways');
    expect(decide).toHaveBeenCalledWith(4, 'deny');
  });

  it('ignores malformed device events (no decide call)', async () => {
    const decide = vi.fn();
    let emit!: (buf: Buffer) => void;
    const link = { writeState: vi.fn(), onEvent: (cb: (b: Buffer) => void) => { emit = cb; }, start: vi.fn(), stop: vi.fn() };
    const adapter = new BleControllerAdapter(link as never, { decide });
    adapter.bind();
    emit(Buffer.from('garbage', 'utf8'));
    await Promise.resolve();
    expect(decide).not.toHaveBeenCalled();
  });
});
