/** Decision verdicts accepted by the daemon's batch-reply endpoint. */
export type Decision = 'allowAlways' | 'allowOnce' | 'deny';

/** A daemon SSE event envelope: { type, timestamp, source, payload }. */
export interface DaemonEvent {
  type: string;
  timestamp: number;
  source: string;
  payload: Record<string, unknown>;
}

/** A pending tool-call approval, as the daemon exposes it. */
export interface PendingApproval {
  requestId: string;
  sessionId: string;
  agentName?: string;
  toolName: string;
  ruleContents: string[];
  toolInput?: string;
  toolDescription?: string;
  reason?: string;
  createdAt?: number;
}

/** A pending approval annotated with the small local id the device will use. */
export interface LocalApproval extends PendingApproval {
  /** 1..255 stable-per-session local id; what the device shows and returns. */
  localId: number;
}

/** The compact state the bridge derives and (later) pushes to the device. */
export interface BuddyState {
  /** protocol version */
  v: 1;
  /** 1 = bridge↔daemon healthy, 0 = daemon unreachable */
  c: 0 | 1;
  /** number of running sessions */
  r: number;
  /** number of pending approvals */
  p: number;
  /** the approval currently surfaced to the user, or null */
  a: {
    id: number;
    t: string;
    d: string;
    s: string;
  } | null;
}
