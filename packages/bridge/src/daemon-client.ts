import { request } from 'undici';
import { SSEParser } from './sse-parse.js';
import type { DaemonEvent, Decision, PendingApproval } from './types.js';

export interface DaemonClientOptions {
  port: number;
  host?: string; // default 127.0.0.1
  /** called for every parsed daemon event (including heartbeat) */
  onEvent: (ev: DaemonEvent) => void;
  /** called when the SSE connection opens or closes */
  onConnectionChange: (connected: boolean) => void;
}

/**
 * Thin client for the MiniMax Code daemon. Owns the SSE subscription
 * (auto-reconnecting) and the two HTTP calls M0 needs: list pending
 * approvals and batch-reply. All paths are under /mavis/api on 127.0.0.1.
 */
export class DaemonClient {
  private readonly base: string;
  private readonly opts: DaemonClientOptions;
  private stopped = false;
  private controller: AbortController | null = null;

  constructor(opts: DaemonClientOptions) {
    this.opts = opts;
    this.base = `http://${opts.host ?? '127.0.0.1'}:${opts.port}/mavis/api`;
  }

  /** Begin (and maintain) the SSE subscription. Resolves immediately; runs until stop(). */
  start(): void {
    void this.runLoop();
  }

  stop(): void {
    this.stopped = true;
    this.controller?.abort();
  }

  async listPending(): Promise<PendingApproval[]> {
    const res = await request(`${this.base}/permission/requests`, { method: 'GET' });
    if (res.statusCode !== 200) {
      res.body.destroy();
      throw new Error(`listPending: HTTP ${res.statusCode}`);
    }
    const body = (await res.body.json()) as { requests: PendingApproval[] };
    return body.requests ?? [];
  }

  async batchReply(requestIds: string[], decision: Decision): Promise<{ processed: string[]; skipped: string[] }> {
    const res = await request(`${this.base}/permission/batch-reply`, {
      method: 'POST',
      headers: { 'content-type': 'application/json' },
      body: JSON.stringify({ requestIds, decision }),
    });
    if (res.statusCode !== 200) {
      res.body.destroy();
      throw new Error(`batchReply: HTTP ${res.statusCode}`);
    }
    const body = (await res.body.json()) as { processed: string[]; skipped: string[] };
    return { processed: body.processed ?? [], skipped: body.skipped ?? [] };
  }

  /** Count running sessions across all agents (status.type === 'started'). */
  async getRunningCount(): Promise<number> {
    const fetcher: JsonFetcher = async (path: string) => {
      const res = await request(`${this.base}${path}`, { method: 'GET' });
      if (res.statusCode !== 200) {
        res.body.destroy();
        throw new Error(`GET ${path}: HTTP ${res.statusCode}`);
      }
      return res.body.json();
    };
    return countRunningSessions(fetcher);
  }

  private async runLoop(): Promise<void> {
    while (!this.stopped) {
      this.controller = new AbortController();
      try {
        const res = await request(`${this.base}/events`, {
          method: 'GET',
          headers: { accept: 'text/event-stream' },
          signal: this.controller.signal,
          // disable undici's body timeout for a long-lived stream
          bodyTimeout: 0,
          headersTimeout: 0,
        });
        if (res.statusCode !== 200) {
          res.body.destroy();
          throw new Error(`events: HTTP ${res.statusCode}`);
        }
        this.opts.onConnectionChange(true);
        const parser = new SSEParser();
        res.body.setEncoding('utf8');
        for await (const chunk of res.body) {
          for (const frame of parser.push(chunk as string)) {
            try {
              this.opts.onEvent(JSON.parse(frame.data) as DaemonEvent);
            } catch {
              // ignore malformed frame
            }
          }
        }
      } catch {
        // connection error or abort — fall through to reconnect
      }
      this.opts.onConnectionChange(false);
      if (this.stopped) break;
      await delay(2000); // backoff before reconnect
    }
  }
}

function delay(ms: number): Promise<void> {
  return new Promise((r) => setTimeout(r, ms));
}

/** Minimal JSON fetcher over the daemon API (path is relative to /mavis/api). */
export type JsonFetcher = (path: string) => Promise<unknown>;

/**
 * Count sessions whose status.type === 'started' across all agents, using an
 * injectable fetcher (pure logic; the real HTTP fetcher is provided by DaemonClient).
 * Resilient to malformed payloads — anything unexpected contributes 0.
 */
export async function countRunningSessions(fetcher: JsonFetcher): Promise<number> {
  const agentsResp = (await fetcher('/agent')) as { agents?: Array<{ name?: unknown }> };
  const agents = Array.isArray(agentsResp?.agents) ? agentsResp.agents : [];
  const counts = await Promise.all(
    agents.map(async (a) => {
      if (typeof a?.name !== 'string') return 0;
      try {
        const sresp = (await fetcher(`/agent/${encodeURIComponent(a.name)}/session`)) as {
          sessions?: Array<{ status?: { type?: unknown } }>;
        };
        const sessions = Array.isArray(sresp?.sessions) ? sresp.sessions : [];
        return sessions.filter((s) => s?.status?.type === 'started').length;
      } catch {
        return 0; // one agent's failure shouldn't break the whole count
      }
    }),
  );
  return counts.reduce((sum, n) => sum + n, 0);
}
