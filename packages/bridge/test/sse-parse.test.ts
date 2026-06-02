import { describe, it, expect } from 'vitest';
import { SSEParser } from '../src/sse-parse.js';

describe('SSEParser', () => {
  it('parses a single complete frame', () => {
    const p = new SSEParser();
    const frames = p.push('event: heartbeat\ndata: {"type":"heartbeat"}\n\n');
    expect(frames).toEqual([{ event: 'heartbeat', data: '{"type":"heartbeat"}' }]);
  });

  it('buffers a frame split across chunks', () => {
    const p = new SSEParser();
    expect(p.push('event: permission.ask\nda')).toEqual([]);
    const frames = p.push('ta: {"x":1}\n\n');
    expect(frames).toEqual([{ event: 'permission.ask', data: '{"x":1}' }]);
  });

  it('parses two frames in one chunk and ignores id: lines', () => {
    const p = new SSEParser();
    const frames = p.push('id: 1\nevent: a\ndata: {}\n\nevent: b\ndata: {}\n\n');
    expect(frames).toEqual([
      { event: 'a', data: '{}' },
      { event: 'b', data: '{}' },
    ]);
  });

  it('defaults event name to "message" when only data: is present', () => {
    const p = new SSEParser();
    expect(p.push('data: {"k":1}\n\n')).toEqual([{ event: 'message', data: '{"k":1}' }]);
  });
});
