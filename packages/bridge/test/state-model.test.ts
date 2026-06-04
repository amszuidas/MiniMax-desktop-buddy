import { describe, it, expect } from 'vitest';
import { StateModel } from '../src/state-model.js';
import type { PendingApproval } from '../src/types.js';

function pending(over: Partial<PendingApproval> = {}): PendingApproval {
  return {
    requestId: 'perm_a',
    sessionId: 'mvs_1',
    toolName: 'bash',
    ruleContents: ['bash(rm:*)'],
    toolInput: 'rm -rf build/',
    reason: 'destructive',
    ...over,
  };
}

describe('StateModel', () => {
  it('starts disconnected and empty', () => {
    const m = new StateModel();
    expect(m.getState()).toEqual({ v: 1, c: 0, r: 0, p: 0, a: null });
  });

  it('marks connected and counts running sessions via lifecycle events', () => {
    const m = new StateModel();
    m.setConnected(true);
    m.applyEvent({ type: 'session.start', timestamp: 1, source: 's', payload: { sessionId: 'mvs_1' } });
    m.applyEvent({ type: 'session.start', timestamp: 2, source: 's', payload: { sessionId: 'mvs_2' } });
    expect(m.getState().r).toBe(2);
    m.applyEvent({ type: 'session.finish', timestamp: 3, source: 's', payload: { sessionId: 'mvs_1' } });
    expect(m.getState().r).toBe(1);
  });

  it('is idempotent: duplicate session.start does not double-count', () => {
    const m = new StateModel();
    m.applyEvent({ type: 'session.start', timestamp: 1, source: 's', payload: { sessionId: 'mvs_1' } });
    m.applyEvent({ type: 'session.start', timestamp: 2, source: 's', payload: { sessionId: 'mvs_1' } });
    expect(m.getState().r).toBe(1);
  });

  it('reconcile() replaces the pending set and assigns local ids', () => {
    const m = new StateModel();
    m.setConnected(true);
    m.reconcile([pending({ requestId: 'perm_a' }), pending({ requestId: 'perm_b', toolInput: 'ls' })]);
    const s = m.getState();
    expect(s.p).toBe(2);
    expect(s.a).not.toBeNull();
    expect(s.a!.id).toBe(1); // first surfaced approval gets local id 1
    expect(m.requestIdFor(1)).toBe('perm_a');
    expect(m.requestIdFor(2)).toBe('perm_b');
  });

  it('applyEvent(permission.ask) adds a new pending approval with a fresh local id', () => {
    const m = new StateModel();
    m.setConnected(true);
    m.applyEvent({
      type: 'permission.ask',
      timestamp: 1,
      source: 'session-bridge',
      payload: { requestId: 'perm_x', sessionId: 'mvs_1', toolName: 'write', ruleContents: [], toolInput: 'foo.txt' },
    });
    expect(m.getState().p).toBe(1);
    expect(m.requestIdFor(1)).toBe('perm_x');
  });

  it('resolve(requestId) removes it and surfaces the next pending approval', () => {
    const m = new StateModel();
    m.setConnected(true);
    m.reconcile([pending({ requestId: 'perm_a' }), pending({ requestId: 'perm_b' })]);
    m.resolve('perm_a');
    const s = m.getState();
    expect(s.p).toBe(1);
    expect(s.a!.id).toBe(2); // perm_b, keeps its original local id
    expect(m.requestIdFor(2)).toBe('perm_b');
  });

  it('truncates long descriptions/labels in the surfaced approval', () => {
    const m = new StateModel();
    m.setConnected(true);
    const longInput = 'x'.repeat(100);
    m.reconcile([pending({ requestId: 'perm_a', toolInput: longInput, sessionId: 'a'.repeat(40) })]);
    const a = m.getState().a!;
    expect(a.d.length).toBeLessThanOrEqual(40);
    expect(a.s.length).toBeLessThanOrEqual(16);
  });

  it('local ids do not collide after reconcile churn', () => {
    const m = new StateModel();
    m.reconcile([pending({ requestId: 'perm_a' })]); // id 1
    m.resolve('perm_a');
    m.applyEvent({
      type: 'permission.ask', timestamp: 9, source: 's',
      payload: { requestId: 'perm_c', sessionId: 'mvs_1', toolName: 'bash', ruleContents: [] },
    });
    // After perm_a is gone, the next id must not reuse a live mapping incorrectly.
    expect(m.requestIdFor(m.getState().a!.id)).toBe('perm_c');
  });

  it('reconcile() drops approvals no longer present and frees their local id', () => {
    const m = new StateModel();
    m.setConnected(true);
    m.reconcile([pending({ requestId: 'perm_a' }), pending({ requestId: 'perm_b' })]);
    expect(m.getState().p).toBe(2);
    // perm_a disappears from the authoritative snapshot
    m.reconcile([pending({ requestId: 'perm_b' })]);
    const s = m.getState();
    expect(s.p).toBe(1);
    expect(m.requestIdFor(1)).toBeUndefined(); // perm_a's local id 1 is freed
    expect(m.requestIdFor(2)).toBe('perm_b');  // perm_b keeps its original id
  });

  it('currentApproval() returns the full, UN-truncated surfaced approval', () => {
    const m = new StateModel();
    m.setConnected(true);
    const longInput = '/Users/me/projects/agent-archon/apps/electron/.env.zh.development';
    const longSession = 'mvs_23afbfde8d3e4098bb531531d107b41a';
    m.reconcile([pending({ requestId: 'perm_a', toolInput: longInput, sessionId: longSession })]);
    const cur = m.currentApproval();
    expect(cur).not.toBeNull();
    expect(cur!.localId).toBe(1);
    expect(cur!.toolInput).toBe(longInput); // NOT truncated (unlike getState().a.d)
    expect(cur!.sessionId).toBe(longSession); // full session id
    expect(cur!.toolName).toBe('bash');
    // getState().a stays truncated for the BLE payload budget
    expect(m.getState().a!.d.length).toBeLessThanOrEqual(40);
  });

  it('currentApproval() returns null when there are no pending approvals', () => {
    const m = new StateModel();
    m.setConnected(true);
    expect(m.currentApproval()).toBeNull();
  });

  it('session.error sets a transient error flag e=1 then clears after the window', () => {
    const m = new StateModel();
    m.setConnected(true);
    m.applyEvent({ type: 'session.start', timestamp: 1, source: 's', payload: { sessionId: 'mvs_1' } });
    m.applyEvent({ type: 'session.error', timestamp: 1000, source: 's', payload: { sessionId: 'mvs_1', error: 'boom' } });
    expect(m.getState(1200).e).toBe(1);
    expect(m.getState(1200).r).toBe(0);
    expect(m.getState(1000 + 3000 + 1).e).toBeUndefined();
  });

  it('getState(now) returns no error flag when none was set', () => {
    const m = new StateModel();
    m.setConnected(true);
    expect(m.getState(500).e).toBeUndefined();
  });

  it('no-arg getState() never sets e, even after a session.error (backward compat)', () => {
    const m = new StateModel();
    m.setConnected(true);
    m.applyEvent({ type: 'session.error', timestamp: 1000, source: 's', payload: { sessionId: 'mvs_1', error: 'x' } });
    expect(m.getState().e).toBeUndefined();  // no-arg callers never see the transient flag
  });
});
