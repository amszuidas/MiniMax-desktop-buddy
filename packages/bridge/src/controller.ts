import { StateModel } from './state-model.js';
import type { BuddyState, DaemonEvent, Decision, LocalApproval } from './types.js';

/** The subset of DaemonClient the controller depends on (for testability). */
export interface ClientLike {
  start(): void;
  stop(): void;
  listPending(): Promise<import('./types.js').PendingApproval[]>;
  batchReply(requestIds: string[], decision: Decision): Promise<{ processed: string[]; skipped: string[] }>;
  listRunningCount(): Promise<number>;
}

export interface ControllerOptions {
  /** called whenever derived state changes */
  onState: (state: BuddyState) => void;
}

/**
 * Wires the daemon client to the state model. Owns the policy:
 *  - on (re)connect, reconcile the full pending set
 *  - on each event, update the model and emit state
 *  - decide(localId, decision) resolves a real approval via batchReply
 */
export class Controller {
  private readonly model = new StateModel();
  constructor(private readonly client: ClientLike, private readonly opts: ControllerOptions) {}

  state(): BuddyState {
    return this.model.getState(Date.now());
  }

  /** The full, un-truncated currently-surfaced approval (for rich UIs), or null. */
  currentApproval(): LocalApproval | null {
    return this.model.currentApproval();
  }

  /** Call after the SSE connection opens (or reopens). */
  async onConnected(): Promise<void> {
    this.model.setConnected(true);
    try {
      this.model.reconcile(await this.client.listPending());
    } catch {
      // leave model as-is; a later event or retry will reconcile
    }
    await this.syncRunning();
    this.emit();
  }

  onDisconnected(): void {
    this.model.setConnected(false);
    this.emit();
  }

  /** Feed a single daemon event. */
  ingest(ev: DaemonEvent): void {
    if (ev.type === 'heartbeat') return;
    this.model.applyEvent(ev);
    this.emit();
  }

  /** Resolve the approval with this local id. No-op if the id is unknown. */
  async decide(localId: number, decision: Decision): Promise<void> {
    const requestId = this.model.requestIdFor(localId);
    if (!requestId) return;
    await this.client.batchReply([requestId], decision);
    this.model.resolve(requestId);
    this.emit();
  }

  /** Resync the running-session count from the daemon (covers sessions predating connect). */
  async syncRunning(): Promise<void> {
    try {
      this.model.setRunningCount(await this.client.listRunningCount());
      this.emit();
    } catch {
      // transient; next sync will retry
    }
  }

  private emit(): void {
    this.opts.onState(this.model.getState(Date.now()));
  }
}
