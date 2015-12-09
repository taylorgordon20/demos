import zlib


class ZlibTimestampCoder:

  def __init__(self):
    self.timestamps = []

  def __iter__(self):
    self.timestamps.sort()
    return iter(self.timestamps)

  def add_timestamp(self, timestamp):
    self.timestamps.append(timestamp)

  def clear(self):
    self.timestamps = []

  def dumps(self):
    self.timestamps.sort()
    prev = 0
    deltas = []
    for ts in self.timestamps:
      deltas.append(ts - prev)
      prev = ts
    bytes = ':'.join('%0x' % delta for delta in deltas)
    return zlib.compress(bytes, 9)

  def loads(self, encoding):
    self.timestamps = []
    bytes = zlib.decompress(encoding)
    prev = 0
    deltas = [int(s, 16) for s in bytes.split(':')]
    for delta in deltas:
      prev = delta + prev
      self.timestamps.append(prev)

def generate_timestamps(num, duration_ms=7 * 86400 * 1000):
  import random, time
  new_ms = int(time.time() * 1000)
  old_ms = new_ms - duration_ms
  return [random.randint(old_ms, new_ms) for i in range(num)]


def test_timestamp_coder():
  import base64
  TimestampCoder = ZlibTimestampCoder

  tc = TimestampCoder()
  tc.add_timestamp(1000)
  tc.add_timestamp(2200)
  tc.add_timestamp(5050)
  tc.add_timestamp(10101)
  tc.add_timestamp(99192)
  encoding = tc.dumps()
  print 'Encoded 5 timestamps. Size: %d bytes.' % len(encoding)
  tc.loads(encoding)
  print 'Decoded:', ', '.join(str(ts) for ts in tc)

  tc.clear()
  timestamps = generate_timestamps(50, 86400 * 1000)
  for ts in timestamps:
    tc.add_timestamp(ts)
  encoding = tc.dumps()
  print 'Encoded 50 timestamps over 1 day. Size: %d bytes.' % len(encoding)
  tc.loads(encoding)
  assert set(tc) == set(timestamps)

  tc.clear()
  timestamps = generate_timestamps(50, 7 * 86400 * 1000)
  for ts in timestamps:
    tc.add_timestamp(ts)
  encoding = tc.dumps()
  print 'Encoded 50 timestamps over 1 week. Size: %d bytes.' % len(encoding)
  tc.loads(encoding)
  assert set(tc) == set(timestamps)

  tc.clear()
  timestamps = generate_timestamps(100, 86400 * 1000)
  for ts in timestamps:
    tc.add_timestamp(ts)
  encoding = tc.dumps()
  print 'Encoded 100 timestamps over 1 day. Size: %d bytes.' % len(encoding)
  tc.loads(encoding)
  assert set(tc) == set(timestamps)

  tc.clear()
  timestamps = generate_timestamps(100, 7 * 86400 * 1000)
  for ts in timestamps:
    tc.add_timestamp(ts)
  encoding = tc.dumps()
  print 'Encoded 100 timestamps over 1 week. Size: %d bytes.' % len(encoding)
  tc.loads(encoding)
  assert set(tc) == set(timestamps)

  tc.clear()
  timestamps = generate_timestamps(500, 7 * 86400 * 1000)
  for ts in timestamps:
    tc.add_timestamp(ts)
  encoding = tc.dumps()
  print 'Encoded 500 timestamps over 1 week. Size: %d bytes.' % len(encoding)
  tc.loads(encoding)
  assert set(tc) == set(timestamps)

  tc.clear()
  timestamps = generate_timestamps(500, 12 * 7 * 86400 * 1000)
  for ts in timestamps:
    tc.add_timestamp(ts)
  encoding = tc.dumps()
  print 'Encoded 500 timestamps over 12 weeks. Size: %d bytes.' % len(encoding)
  tc.loads(encoding)
  assert set(tc) == set(timestamps)


if __name__ == '__main__':
  test_timestamp_coder()

