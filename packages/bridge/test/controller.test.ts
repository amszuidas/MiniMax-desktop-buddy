import { describe, it, expect, vi } from 'vitest';
import { Controller } from '../src/controller.js';
import type { DaemonEvent, Decision, PendingApproval } from '../src/types.js';

/** A fake client capturing batchReply calls and letting tests push events. */
function makeFakeClient(pending: PendingApproval[] = []) {
  const calls: Array<{ ids: string[]; decision: Decision }> = [];
  let emit: ((ev: DaemonEvent) => void) | null = null;
  let conn: ((c: boolean) => void) | null = null;
  return {
    calls,
    fire: (ev: DaemonEvent) => emit?.(ev),
    setConn: (c: boolean) => conn?.(c),
    client: {
      start: vi.fn(),
      stop: vi.fn(),
      listPending: vi.fn(async () => pending),
      batchReply: vi.fn(async (ids: string[], decision: Decision) => {
        calls.push({ ids, decision });
        return { processed: ids, skipped: [] };
      }),
      _bind: (onEvent: (ev: DaemonEvent) => void, onConn: (c: boolean) => void) => {
        emit = onEvent;
        conn = onConn;
      },
    },
  };
}

describe('Controller', () => {
  it('reconciles pending approvals on connect', async () => {
    const fake = makeFakeClient([
      { requestId: 'perm_a', sessionId: 'mvs_1', toolName: 'bash', ruleContents: [], toolInput: 'ls' },
    ]);
    const onState = vi.fn();
    const c = new Controller(fake.client as never, { onState });
    fake.client._bind = fake.client._bind; // satisfy lint
    await c.onConnected();
    expect(c.state().p).toBe(1);
    expect(c.state().c).toBe(1);
  });

  it('decide(localId, "allowOnce") maps to requestId and calls batchReply, then removes it', async () => {
    const fake = makeFakeClient([
      { requestId: 'perm_a', sessionId: 'mvs_1', toolName: 'bash', ruleContents: [], toolInput: 'ls' },
    ]);
    const c = new Controller(fake.client as never, { onState: vi.fn() });
    await c.onConnected();
    const localId = c.state().a!.id;
    await c.decide(localId, 'allowOnce');
    expect(fake.calls).toEqual([{ ids: ['perm_a'], decision: 'allowOnce' }]);
    expect(c.state().p).toBe(0);
    expect(c.state().a).toBeNull();
  });

  it('ingest(permission.ask) adds a pending approval and emits new state', async () => {
    const fake = makeFakeClient();
    const onState = vi.fn();
    const c = new Controller(fake.client as never, { onState });
    c.ingest({
      type: 'permission.ask', timestamp: 1, source: 's',
      payload: { requestId: 'perm_z', sessionId: 'mvs_1', toolName: 'write', ruleContents: [] },
    });
    expect(c.state().p).toBe(1);
    expect(onState).toHaveBeenCalled();
  });

  it('decide() on an unknown local id is a no-op (no batchReply)', async () => {
    const fake = makeFakeClient();
    const c = new Controller(fake.client as never, { onState: vi.fn() });
    await c.decide(99, 'deny');
    expect(fake.calls).toEqual([]);
  });
});
