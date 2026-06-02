import { readFileSync } from 'node:fs';
import { homedir } from 'node:os';
import { join } from 'node:path';

export const DEFAULT_DAEMON_PORT = 5321;

/**
 * Resolve the daemon's data directory. The MiniMax Code daemon writes its
 * runtime files (daemon.port, daemon.pid, config.yaml) under `~/.mavis`
 * (its DEFAULT_DATA_DIR; on desktop installs this is a symlink to the
 * brand-specific dir). We mirror that default and expose BUDDY_DATA_DIR as
 * an explicit override for non-default profiles (e.g. a worktree daemon
 * whose data dir is `~/.mavis-<profile>`).
 */
export function resolveDataDir(): string {
  if (process.env.BUDDY_DATA_DIR) return process.env.BUDDY_DATA_DIR;
  return join(homedir(), '.mavis');
}

/**
 * Resolve the daemon TCP port. Priority:
 *   1. BUDDY_DAEMON_PORT env var (if a valid port number)
 *   2. `<dataDir>/daemon.port` file written by a running daemon
 *   3. DEFAULT_DAEMON_PORT (5321)
 */
export function resolveDaemonPort(opts?: { dataDir?: string }): number {
  const envPort = parsePort(process.env.BUDDY_DAEMON_PORT);
  if (envPort !== null) return envPort;

  const dataDir = opts?.dataDir ?? resolveDataDir();
  try {
    const filePort = parsePort(readFileSync(join(dataDir, 'daemon.port'), 'utf8'));
    if (filePort !== null) return filePort;
  } catch {
    // file missing/unreadable — fall through to default
  }
  return DEFAULT_DAEMON_PORT;
}

function parsePort(raw: string | undefined | null): number | null {
  if (raw == null) return null;
  const n = Number.parseInt(raw.trim(), 10);
  return Number.isInteger(n) && n > 0 && n < 65536 ? n : null;
}
