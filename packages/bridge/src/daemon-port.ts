import { readFileSync } from 'node:fs';
import { homedir } from 'node:os';
import { join } from 'node:path';

export const DEFAULT_DAEMON_PORT = 5321;

/** Resolve the daemon's data directory the same way the daemon does (macOS-focused for M0). */
export function resolveDataDir(): string {
  if (process.env.MAVIS_DATA_DIR) return process.env.MAVIS_DATA_DIR;
  // macOS default; Linux/Windows handled in a later milestone.
  return join(homedir(), 'Library', 'Application Support', 'mavis');
}

/**
 * Resolve the daemon TCP port. Priority:
 *   1. MAVIS_DAEMON_PORT env var (if a valid port number)
 *   2. `<dataDir>/daemon.port` file written by a running daemon
 *   3. DEFAULT_DAEMON_PORT (5321)
 */
export function resolveDaemonPort(opts?: { dataDir?: string }): number {
  const envPort = parsePort(process.env.MAVIS_DAEMON_PORT);
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
