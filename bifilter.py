import itertools
import math
import mmh3
import numpy as np
import struct


class Bifilter:

  def __init__(self, includes=set(), excludes=set()):
    assert not includes & excludes, 'includes and excludes cannot intersect'
    assert len(includes) + len(excludes) < 2**16
    self.layers = []
    while len(includes) + len(excludes) > 0:
      s = len(self.layers)
      n = len(includes) + len(excludes)
      k = 8 * (1 + int(-0.0625 * n / math.log(1 - 1.0 / 2**0.5)))  # Byte padded

      # Compute the bucket mapping of each include and exclude element
      in_keys = [self.__bucket(el, s, k) for el in includes]
      ex_keys = [self.__bucket(el, s, k) for el in excludes]

      # Record which buckets get mapped include and exclude elements
      in_buckets = [False] * k
      ex_buckets = [False] * k
      for i in xrange(len(includes)):
        in_buckets[in_keys[i]] = True
      for i in xrange(len(excludes)):
        ex_buckets[ex_keys[i]] = True

      # Compute this layers bucket states
      layer = [
          2 if in_buckets[i] and ex_buckets[i] else int(ex_buckets[i])
          for i in xrange(k)
      ]

      # Recurse until nothing is ambiguous.
      includes = [el for i, el in enumerate(includes) if layer[in_keys[i]] == 2]
      excludes = [el for i, el in enumerate(excludes) if layer[ex_keys[i]] == 2]
      self.layers.append(layer)

    
  def __contains__(self, element):
    for i, layer in enumerate(self.layers):
      state = layer[self.__bucket(element, i, len(layer))]
      assert state in (0, 1, 2)
      if state == 0:
        return True
      elif state == 1:
        return False
    raise LookupError('Invalid state. Final layer should be unambiguous')

  @staticmethod
  def __bucket(element, level, range): 
    return mmh3.hash(element, level) % range

  def dumps(self):
    header = np.array([len(l) for l in self.layers], np.uint16).tostring()
    flat = np.fromiter(itertools.chain.from_iterable(self.layers), dtype=np.int)
    leaves = flat != 2
    branch_bits = np.packbits(leaves.astype(np.int))
    leaves_bits = np.packbits(flat[leaves])  # Might be padded
    return (
        struct.pack('H', len(header)) +
        header +
        branch_bits.tostring() +
        leaves_bits.tostring()
    )

  @staticmethod
  def loads(encoding):
    assert len(encoding) > 2
    header_size = struct.unpack('H', encoding[:2])[0]
    header = np.fromstring(encoding[2:2 + header_size], dtype=np.uint16)

    # Decode the bucket states
    bo, bs = 2 + header_size, int(np.sum(header)) / 8
    branch_bits = np.fromstring(encoding[bo:bo + bs], dtype=np.uint8)
    leaves_bits = np.fromstring(encoding[bo + bs:], dtype=np.uint8)
    branch_states = np.unpackbits(branch_bits)
    leaves_states = np.unpackbits(leaves_bits)

    # Build the filter.
    bifilter = Bifilter()
    branch_sum, leaves_sum = 0, 0
    for i in xrange(len(header)):
      layer = branch_states[branch_sum:branch_sum + header[i]]
      k = np.sum(layer == 1)  # Count of leaves
      layer[layer == 0] = 2  # Ambiguous states
      layer[layer == 1] = leaves_states[leaves_sum:leaves_sum + k]
      bifilter.layers.append([int(x) for x in layer])  # Must cast to int!
      branch_sum += len(layer)
      leaves_sum += k
    return bifilter


def test():
  import random, struct, time
  n = 1000
  split = 0.5
  candidates = [struct.pack('L', i) for i in range(n)]

  random.shuffle(candidates)
  includes = candidates[:int(split * n)]
  excludes = candidates[int(n * split):]

  start_time = time.time()
  bifilter = Bifilter(set(includes), set(excludes))
  total_time = time.time() - start_time
  print "Construction took: %s seconds" % total_time

  start_time = time.time()
  for el in includes:
    assert el in bifilter
  for el in excludes:
    assert el not in bifilter
  total_time = time.time() - start_time
  print "Querying Took: %s seconds" % total_time

  start_time = time.time()
  encoding = bifilter.dumps()
  total_time = time.time() - start_time
  print "Encoding took: %s seconds" % total_time
  print "Encoding size:", len(encoding)

  start_time = time.time()
  bifilter = Bifilter.loads(encoding)
  total_time = time.time() - start_time
  print "Decoding took: %s seconds" % total_time

  start_time = time.time()
  for el in includes:
    assert el in bifilter
  for el in excludes:
    assert el not in bifilter
  total_time = time.time() - start_time
  print "Querying Took: %s seconds" % total_time


if __name__ == '__main__':
  test()
