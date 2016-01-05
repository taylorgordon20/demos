import zlib


class ReadStateCoder:

  def add_event(self, event_time, read_state):
    pass

  

  @staticmethod
  def encode(self, event_times):
    et_iter = reversed(sorted(event_times))


def generate_event_times(num, duration=7 * 86400 * 1000):
  import time
  new_ms = int(time.time() * 1000)
  old_ms = new_ms - duration
  return [random.randint(old_ms, new_ms) for i in range(num)]


def test_event_times_coder():
  # Try encoding 1000 event times from over a week
  event_times = generate_event_times(1000)
  code = EventTimesCoder.encode(event_times)
  print 'Length of 1000 events from week encoding:', len(code)

  # Verify that the encoding did not lose any information.
  decoded_times = EventTimesCoder.decode(code)
  event_times.sort()
  decoded_times.sort()
  for i, event_time in 
  


if __name__ == '__main__':
  test_event_times_coder()
