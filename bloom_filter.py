import mmh3
import numpy as np
import zlib
from math import ceil,log

class BloomFilter:

  def __init__(self, num_values, error_rate):
    self.num_values = num_values
    self.error_rate = error_rate
    self.array_size = 8 * int(ceil(-num_values * log(error_rate) / log(2)**2.0 * 0.125))
    self.num_hashes = int(self.array_size / self.num_values * log(2)) + 1
    self.bf = np.zeros(self.array_size, dtype=np.uint8)

  def add_value(self, value):
    m, k = self.array_size, self.num_hashes
    for i in range(k):
      self.bf[mmh3.hash(value, i) % m] = 1

  def has_value(self, value):
    m, k = self.array_size, self.num_hashes
    return all(self.bf[mmh3.hash(value, i) % m] != 0 for i in range(k))

  def dumps(self):
    bytes = np.packbits(self.bf).tostring()
    return zlib.compress(bytes)

  def loads(self, encoding):
    bytes = zlib.decompress(encoding)
    self.bf = np.unpackbits(np.fromstring(bytes, dtype=np.uint8))
    assert self.array_size == len(self.bf)


def test_bloom_filter():
  bf = BloomFilter(4, 1.0 / 2**8)
  bf.add_value('value_1')
  bf.add_value('value_2')
  bf.add_value('value_3')
  bf.add_value('value_4')
  print 'Bloom filter has value_1:', bf.has_value('value_1')
  print 'Bloom filter has value_2:', bf.has_value('value_2')
  print 'Bloom filter has value_3:', bf.has_value('value_3')
  print 'Bloom filter has value_4:', bf.has_value('value_4')
  print 'Bloom filter has value_5:', bf.has_value('value_5')
  code = bf.dumps()
  print '4 value bloom filter has size %d bytes' % len(code)
  bf = BloomFilter(4, 1.0 / 2**8)
  bf.loads(code)
  print 'Bloom filter has value_1:', bf.has_value('value_1')
  print 'Bloom filter has value_2:', bf.has_value('value_2')
  print 'Bloom filter has value_3:', bf.has_value('value_3')
  print 'Bloom filter has value_4:', bf.has_value('value_4')
  print 'Bloom filter has value_5:', bf.has_value('value_5')

  # Create a fully loaded bloom filter.
  bf = BloomFilter(1000, 1.0 / 2**8)
  for i in range(1000):
    bf.add_value('value_%d' % i)
  print "All values are present:", all(
      bf.has_value('value_%d' % i) for i in range(1000))
  print "Test false positive:", sum(
      1 if bf.has_value('value_%d' % i) else 0 for i in range(1000, 200000))
  code = bf.dumps()
  print '1000 value bloom filter has size %d bytes' % len(code)

  # Test the size of a typical bloom filter load.
  bf = BloomFilter(100, 1.0 / 2**8)
  for i in range(100):
    bf.add_value('value_%d' % i)
  code = bf.dumps()
  print '100 value bloom filter has size %d bytes' % len(code)


if __name__ == '__main__':
  test_bloom_filter()
