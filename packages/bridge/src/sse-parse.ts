/** One parsed SSE frame. `data` is the concatenated data lines (no trailing newline). */
export interface SSEFrame {
  event: string;
  data: string;
}

/**
 * Incremental Server-Sent-Events parser. Feed it raw text chunks via push();
 * it returns whatever complete frames (terminated by a blank line) are now
 * available, buffering any partial remainder for the next call.
 *
 * Only the `event:` and `data:` fields are interpreted; `id:` and comments
 * (lines starting with ':') are ignored, matching what the daemon emits.
 */
export class SSEParser {
  private buf = '';

  push(chunk: string): SSEFrame[] {
    this.buf += chunk;
    const frames: SSEFrame[] = [];
    let sep: number;
    // Frames are separated by a blank line ("\n\n").
    while ((sep = this.buf.indexOf('\n\n')) !== -1) {
      const raw = this.buf.slice(0, sep);
      this.buf = this.buf.slice(sep + 2);
      const frame = this.parseFrame(raw);
      if (frame) frames.push(frame);
    }
    return frames;
  }

  private parseFrame(raw: string): SSEFrame | null {
    let event = 'message';
    const dataLines: string[] = [];
    for (const line of raw.split('\n')) {
      if (line === '' || line.startsWith(':')) continue;
      const idx = line.indexOf(':');
      const field = idx === -1 ? line : line.slice(0, idx);
      // SSE spec: a single leading space after the colon is stripped.
      let value = idx === -1 ? '' : line.slice(idx + 1);
      if (value.startsWith(' ')) value = value.slice(1);
      if (field === 'event') event = value;
      else if (field === 'data') dataLines.push(value);
    }
    if (dataLines.length === 0) return null;
    return { event, data: dataLines.join('\n') };
  }
}
