import { Controller } from '../controller.js';
import { DaemonClient } from '../daemon-client.js';
import { resolveDaemonPort } from '../daemon-port.js';
import { BleLink } from '../ble-link.js';
import { BleControllerAdapter } from '../ble-controller-adapter.js';
import { BLE_DEVICE_NAME } from '../ble-protocol.js';
import type { BuddyState } from '../types.js';

async function main(): Promise<void> {
  const port = resolveDaemonPort();
  let controller: Controller;

  const ble = new BleLink();
  const adapter = new BleControllerAdapter(ble, {
    decide: (localId, decision) => controller.decide(localId, decision),
  });

  const client = new DaemonClient({
    port,
    onEvent: (ev) => controller.ingest(ev),
    onConnectionChange: (connected) => {
      if (connected) void controller.onConnected();
      else controller.onDisconnected();
    },
  });

  // controller state → BLE device (and a one-line terminal mirror)
  controller = new Controller(client, {
    onState: (state: BuddyState) => {
      adapter.handleState(state);
      process.stdout.write(
        `\r[daemon ${state.c ? 'OK' : '..'}] running=${state.r} pending=${state.p}` +
        (state.a ? `  ▶#${state.a.id} ${state.a.t} ${state.a.d}` : '            ') + '   \x1b[K',
      );
    },
  });

  adapter.bind();
  console.log(`Bridge(BLE) starting. daemon=127.0.0.1:${port}, scanning for ${BLE_DEVICE_NAME} …`);
  client.start();
  ble.start();

  process.on('SIGINT', () => { ble.stop(); client.stop(); process.exit(0); });
}

void main();
