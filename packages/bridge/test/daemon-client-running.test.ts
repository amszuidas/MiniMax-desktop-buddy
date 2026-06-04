import { describe, it, expect } from 'vitest';
import { countRunningSessions } from '../src/daemon-client.js';

describe('countRunningSessions', () => {
  it('counts sessions whose status.type is started across all agents', async () => {
    const fetcher = async (path: string): Promise<unknown> => {
      if (path === '/agent') return { agents: [{ name: 'main' }, { name: 'dev' }] };
      if (path === '/agent/main/session') {
        return { sessions: [
          { sessionId: 'a', status: { type: 'started' } },
          { sessionId: 'b', status: { type: 'finished' } },
        ] };
      }
      if (path === '/agent/dev/session') {
        return { sessions: [ { sessionId: 'c', status: { type: 'started' } } ] };
      }
      return {};
    };
    expect(await countRunningSessions(fetcher)).toBe(2);
  });

  it('returns 0 when no agents or no running sessions', async () => {
    const fetcher = async (path: string): Promise<unknown> => {
      if (path === '/agent') return { agents: [] };
      return {};
    };
    expect(await countRunningSessions(fetcher)).toBe(0);
  });

  it('is resilient to malformed agent/session payloads', async () => {
    const fetcher = async (path: string): Promise<unknown> => {
      if (path === '/agent') return { agents: [{ name: 'main' }, { notName: 1 }] };
      if (path === '/agent/main/session') return { sessions: [{ status: { type: 'started' } }, { foo: 1 }] };
      return { junk: true };
    };
    expect(await countRunningSessions(fetcher)).toBe(1);
  });
});
