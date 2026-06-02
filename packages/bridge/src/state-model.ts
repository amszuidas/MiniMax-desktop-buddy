import type { BuddyState, DaemonEvent, LocalApproval, PendingApproval } from './types.js';

const SESSION_END_TYPES = new Set(['session.finish', 'session.error', 'session.abort']);
const MAX_DESC = 40;
const MAX_LABEL = 16;

function truncate(s: string, max: number): string {
  return s.length <= max ? s : s.slice(0, max - 1) + '…';
}

/**
 * Pure, in-memory model of Buddy state. Consumers feed it daemon events and
 * pending-list snapshots; it derives the compact BuddyState and owns the
 * localId↔requestId mapping the device uses to refer to approvals.
 *
 * Local ids are monotonically increasing (1..255 wrapping) so a given approval
 * keeps a stable id for its lifetime; ids are freed when the approval resolves.
 */
export class StateModel {
  private connected = false;
  private running = new Set<string>();
  /** requestId → LocalApproval, insertion-ordered (Map preserves order). */
  private pending = new Map<string, PendingApproval & { localId: number }>();
  private byLocalId = new Map<number, string>();
  private nextLocalId = 1;

  setConnected(c: boolean): void {
    this.connected = c;
  }

  applyEvent(ev: DaemonEvent): void {
    const sid = typeof ev.payload.sessionId === 'string' ? ev.payload.sessionId : undefined;
    if (ev.type === 'session.start' && sid) {
      this.running.add(sid);
    } else if (SESSION_END_TYPES.has(ev.type) && sid) {
      this.running.delete(sid);
    } else if (ev.type === 'permission.ask') {
      this.addPending(this.eventToApproval(ev));
    }
  }

  /** Replace the entire pending set from an authoritative snapshot (startup / reconnect). */
  reconcile(list: PendingApproval[]): void {
    const keep = new Set(list.map((r) => r.requestId));
    // Drop approvals no longer pending.
    for (const reqId of [...this.pending.keys()]) {
      if (!keep.has(reqId)) this.removePending(reqId);
    }
    // Add new ones (preserve existing local ids for already-known approvals).
    for (const r of list) {
      if (!this.pending.has(r.requestId)) this.addPending(r);
    }
  }

  /** Remove a resolved approval (after a successful batch-reply). */
  resolve(requestId: string): void {
    this.removePending(requestId);
  }

  requestIdFor(localId: number): string | undefined {
    return this.byLocalId.get(localId);
  }

  /**
   * The full, un-truncated currently-surfaced approval (first pending), or null.
   * Unlike getState().a (which truncates for the BLE payload budget), this
   * returns complete fields for rich consumers like the terminal UI.
   */
  currentApproval(): LocalApproval | null {
    const first = this.pending.values().next();
    return first.done ? null : first.value;
  }

  getState(): BuddyState {
    const first = this.pending.values().next();
    const a = first.done
      ? null
      : {
          id: first.value.localId,
          t: first.value.toolName,
          d: truncate(first.value.toolInput ?? first.value.toolDescription ?? '', MAX_DESC),
          s: truncate(first.value.sessionId, MAX_LABEL),
        };
    return {
      v: 1,
      c: this.connected ? 1 : 0,
      r: this.running.size,
      p: this.pending.size,
      a,
    };
  }

  private eventToApproval(ev: DaemonEvent): PendingApproval {
    const p = ev.payload;
    return {
      requestId: String(p.requestId),
      sessionId: typeof p.sessionId === 'string' ? p.sessionId : '',
      agentName: typeof p.agentName === 'string' ? p.agentName : undefined,
      toolName: typeof p.toolName === 'string' ? p.toolName : 'tool',
      ruleContents: Array.isArray(p.ruleContents) ? (p.ruleContents as string[]) : [],
      toolInput: typeof p.toolInput === 'string' ? p.toolInput : undefined,
      toolDescription: typeof p.toolDescription === 'string' ? p.toolDescription : undefined,
      reason: typeof p.reason === 'string' ? p.reason : undefined,
    };
  }

  private addPending(r: PendingApproval): void {
    if (!r.requestId || this.pending.has(r.requestId)) return;
    const localId = this.allocLocalId();
    if (localId === null) return; // pool exhausted (>255 concurrent pending) — drop gracefully
    this.pending.set(r.requestId, { ...r, localId });
    this.byLocalId.set(localId, r.requestId);
  }

  /** Allocate the next free local id in 1..255, or null if all are in use. */
  private allocLocalId(): number | null {
    for (let i = 0; i < 255; i++) {
      const candidate = this.nextLocalId;
      this.nextLocalId = (this.nextLocalId % 255) + 1;
      if (!this.byLocalId.has(candidate)) return candidate;
    }
    return null;
  }

  private removePending(requestId: string): void {
    const existing = this.pending.get(requestId);
    if (!existing) return;
    this.pending.delete(requestId);
    this.byLocalId.delete(existing.localId);
  }
}
