import { Controller } from '../controller.js';
import { DaemonClient } from '../daemon-client.js';
import { resolveDaemonPort } from '../daemon-port.js';
import type { BuddyState } from '../types.js';

function render(state: BuddyState): void {
  const lines = [
    `┌─ MiniMax Desktop Buddy (bridge M0) ─────────────────`,
    `│ daemon: ${state.c ? 'CONNECTED' : 'disconnected'}   running sessions: ${state.r}   pending: ${state.p}`,
  ];
  if (state.a) {
    lines.push(`│ ▶ APPROVAL #${state.a.id}  [${state.a.t}]  ${state.a.d}   (session ${state.a.s})`);
    lines.push(`│   keys:  a = allow once   A = allow always   d = deny   n = next`);
  } else {
    lines.push(`│ (no pending approvals)`);
  }
  lines.push(`└─────────────────────────────────────────────────`);
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

  controller = new Controller(client, { onState: render });

  console.log(`Connecting to daemon on 127.0.0.1:${port} …`);
  client.start();

  // Keyboard handling: a/A/d/n act on the currently surfaced approval.
  const stdin = process.stdin;
  if (stdin.isTTY) stdin.setRawMode(true);
  stdin.resume();
  stdin.setEncoding('utf8');
  stdin.on('data', (key: string) => {
    const a = controller.state().a;
    if (key === '\x03') { // Ctrl-C
      client.stop();
      process.exit(0);
    }
    if (!a) return;
    if (key === 'a') void controller.decide(a.id, 'allowOnce');
    else if (key === 'A') void controller.decide(a.id, 'allowAlways');
    else if (key === 'd') void controller.decide(a.id, 'deny');
    // 'n' (next) is a device-side concern; with a single surfaced slot it is a no-op here.
  });
}

void main();
