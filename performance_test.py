import mmh3
import numpy as np
import random
import struct
import time


def lookup_bucket(buckets, key):
  bucket = mmh3.hash(struct.pack('L', key)) % len(buckets)
  if buckets[bucket] == 0:
    return True
  elif buckets[bucket] == 1:
    return False


def test():
  n = 6000
  buckets = [random.randint(0, 1) for i in range(n)]
  buckets = np.array(buckets, np.int32)

  start_time = time.time()
  results = [] 
  for i in xrange(n):
    ret = lookup_bucket(buckets, i)
    assert ret == True or ret == False
    results.append(ret)
  end_time = time.time()
  print "Step 1 took: %.5f seconds" % (end_time - start_time)

  start_time = time.time()
  results = [lookup_bucket(buckets, i) for i in xrange(n)]
  end_time = time.time()
  print "Step 2 took: %.5f seconds" % (end_time - start_time)

  start_time = time.time()
  results = [buckets[mmh3.hash(struct.pack('L', i)) % len(buckets)] for i in xrange(n)]
  end_time = time.time()
  print "Step 3 took: %.5f seconds" % (end_time - start_time)


if __name__ == '__main__':
  test()
