import { describe, it, expect, afterEach } from 'vitest';
import { mkdtempSync, writeFileSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { resolveDaemonPort } from '../src/daemon-port.js';

const made: string[] = [];
function tmp(): string {
  const d = mkdtempSync(join(tmpdir(), 'buddy-port-'));
  made.push(d);
  return d;
}
afterEach(() => {
  for (const d of made.splice(0)) rmSync(d, { recursive: true, force: true });
  delete process.env.MAVIS_DAEMON_PORT;
});

describe('resolveDaemonPort', () => {
  it('prefers MAVIS_DAEMON_PORT when set and valid', () => {
    process.env.MAVIS_DAEMON_PORT = '6000';
    expect(resolveDaemonPort({ dataDir: tmp() })).toBe(6000);
  });

  it('reads the daemon.port file when env is unset', () => {
    const dir = tmp();
    writeFileSync(join(dir, 'daemon.port'), '5999\n');
    expect(resolveDaemonPort({ dataDir: dir })).toBe(5999);
  });

  it('falls back to 5321 when neither env nor file is present', () => {
    expect(resolveDaemonPort({ dataDir: tmp() })).toBe(5321);
  });

  it('falls back to 5321 when the file contains garbage', () => {
    const dir = tmp();
    writeFileSync(join(dir, 'daemon.port'), 'not-a-number');
    expect(resolveDaemonPort({ dataDir: dir })).toBe(5321);
  });
});
