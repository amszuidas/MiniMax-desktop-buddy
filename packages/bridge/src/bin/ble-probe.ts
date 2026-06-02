/**
 * BLE viability probe. Loads @abandonware/noble, waits for the adapter to reach
 * `poweredOn`, scans for ~5s, and prints how many peripherals were seen. This
 * does NOT talk to the device (no device exists yet) — it only proves noble
 * works on this Mac so Plan 3 (real GATT transport) can rely on it.
 *
 * Exit codes: 0 = poweredOn reached (success), 2 = noble missing, 3 = not poweredOn.
 */
async function main(): Promise<void> {
  let noble: typeof import('@abandonware/noble');
  try {
    noble = (await import('@abandonware/noble')).default ?? (await import('@abandonware/noble'));
  } catch (err) {
    console.error('noble failed to load. On macOS, grant Bluetooth permission to your terminal:');
    console.error('  System Settings → Privacy & Security → Bluetooth → add your terminal app.');
    console.error(String(err));
    process.exit(2);
  }

  let seen = 0;
  const timeout = setTimeout(() => {
    console.log(`Scan complete. Saw ${seen} BLE peripheral(s). noble is VIABLE on this Mac. ✅`);
    process.exit(0);
  }, 5000);

  noble.on('stateChange', (state: string) => {
    console.log(`noble state → ${state}`);
    if (state === 'poweredOn') {
      console.log('Adapter poweredOn; scanning 5s …');
      void noble.startScanningAsync([], true);
    } else if (state !== 'unknown' && state !== 'resetting') {
      clearTimeout(timeout);
      console.error(`Adapter not usable (state=${state}). Check Bluetooth is ON and permitted.`);
      process.exit(3);
    }
  });

  noble.on('discover', (p: { advertisement?: { localName?: string }; address?: string }) => {
    seen += 1;
    const name = p.advertisement?.localName || '(no name)';
    console.log(`  discovered: ${name} ${p.address ? `[${p.address}]` : ''}`);
  });
}

void main();
