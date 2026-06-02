import { Controller } from '../controller.js';
import { DaemonClient } from '../daemon-client.js';
import { resolveDaemonPort } from '../daemon-port.js';

/**
 * Render the current bridge state to the terminal. Reads the full, un-truncated
 * approval from the controller (the BLE-budgeted state.a is for the device, not
 * the terminal). The running-session count is intentionally omitted in M0: it is
 * derived purely from SSE lifecycle events and drifts (sessions already running
 * before the bridge connects are not counted). It returns in M1 once the daemon
 * exposes a running-sessions snapshot. See docs/M0-followups.md.
 */
function render(controller: Controller): void {
  const state = controller.state();
  const lines = [
    `┌─ MiniMax Desktop Buddy (bridge M0) ─────────────────`,
    `│ daemon: ${state.c ? 'CONNECTED' : 'disconnected'}   pending approvals: ${state.p}`,
  ];
  const cur = controller.currentApproval();
  if (cur) {
    const position = state.p > 1 ? `  (1 of ${state.p})` : '';
    lines.push(`│`);
    lines.push(`│ ▶ APPROVAL #${cur.localId}  [${cur.toolName}]${position}`);
    if (cur.toolInput) lines.push(`│   input:   ${cur.toolInput}`);
    if (cur.toolDescription) lines.push(`│   desc:    ${cur.toolDescription}`);
    if (cur.reason) lines.push(`│   reason:  ${cur.reason}`);
    lines.push(`│   session: ${cur.sessionId}${cur.agentName ? `  agent: ${cur.agentName}` : ''}`);
    lines.push(`│`);
    lines.push(`│   keys:  a = allow once   A = allow always   d = deny`);
  } else {
    lines.push(`│ (no pending approvals)`);
  }
  lines.push(`└─────────────────────────────────────────────────`);
  lines.push(`  Ctrl-C to quit`);
  // Clear screen and repaint.
  process.stdout.write('\x1b[2J\x1b[H' + lines.join('\n') + '\n');
}

async function main(): Promise<void> {
  const port = resolveDaemonPort();
  let controller: Controller;

  const client = new DaemonClient({
    port,
    onEvent: (ev) => controller.ingest(ev),
    onConnectionChange: (connected) => {
      if (connected) void controller.onConnected();
      else controller.onDisconnected();
    },
  });

  // onState fires on every state change; re-render from the controller so we
  // always paint the full (un-truncated) approval, not the BLE-budgeted state.a.
  controller = new Controller(client, { onState: () => render(controller) });

  console.log(`Connecting to daemon on 127.0.0.1:${port} …`);
  client.start();

  // Keyboard handling: a/A/d act on the currently surfaced approval.
  const stdin = process.stdin;
  if (stdin.isTTY) stdin.setRawMode(true);
  stdin.resume();
  stdin.setEncoding('utf8');
  stdin.on('data', (key: string) => {
    if (key === '\x03') { // Ctrl-C
      client.stop();
      process.exit(0);
    }
    const cur = controller.currentApproval();
    if (!cur) return;
    if (key === 'a') void controller.decide(cur.localId, 'allowOnce');
    else if (key === 'A') void controller.decide(cur.localId, 'allowAlways');
    else if (key === 'd') void controller.decide(cur.localId, 'deny');
  });
}

void main();
