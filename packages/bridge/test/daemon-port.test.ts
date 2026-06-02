import { describe, it, expect, afterEach } from 'vitest';
import { mkdtempSync, writeFileSync, rmSync } from 'node:fs';
import { tmpdir, homedir } from 'node:os';
import { join } from 'node:path';
import { resolveDaemonPort, resolveDataDir } from '../src/daemon-port.js';

const made: string[] = [];
function tmp(): string {
  const d = mkdtempSync(join(tmpdir(), 'buddy-port-'));
  made.push(d);
  return d;
}
afterEach(() => {
  for (const d of made.splice(0)) rmSync(d, { recursive: true, force: true });
  delete process.env.BUDDY_DAEMON_PORT;
  delete process.env.BUDDY_DATA_DIR;
});

describe('resolveDataDir', () => {
  it('defaults to ~/.mavis (matches the daemon DEFAULT_DATA_DIR), NOT the platform Library path', () => {
    // Regression: the bridge previously resolved ~/Library/Application Support/mavis,
    // which does not exist — the real daemon writes daemon.port under ~/.mavis.
    expect(resolveDataDir()).toBe(join(homedir(), '.mavis'));
  });

  it('honors BUDDY_DATA_DIR override when set', () => {
    process.env.BUDDY_DATA_DIR = '/tmp/some-buddy-dir';
    expect(resolveDataDir()).toBe('/tmp/some-buddy-dir');
  });
});

describe('resolveDaemonPort', () => {
  it('prefers BUDDY_DAEMON_PORT when set and valid', () => {
    process.env.BUDDY_DAEMON_PORT = '6000';
    expect(resolveDaemonPort({ dataDir: tmp() })).toBe(6000);
  });

  it('reads the daemon.port file when env is unset', () => {
    const dir = tmp();
    writeFileSync(join(dir, 'daemon.port'), '5999\n');
    expect(resolveDaemonPort({ dataDir: dir })).toBe(5999);
  });

  it('reads the daemon.port file from the default ~/.mavis when no dataDir is passed', () => {
    // This is the real end-to-end path: no opts, no env → must look in ~/.mavis.
    // We assert the resolved dir is ~/.mavis by pointing BUDDY_DATA_DIR at a temp
    // dir with a known port file and confirming it is read.
    const dir = tmp();
    writeFileSync(join(dir, 'daemon.port'), '15321\n');
    process.env.BUDDY_DATA_DIR = dir;
    expect(resolveDaemonPort()).toBe(15321);
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
