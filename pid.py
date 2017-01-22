import itertools
import matplotlib.pyplot as plt
import math
import numpy as np
import random
import time
from collections import Counter


class Throttle:

  def __init__(self, period):
    self.period = period
    self.last_call = 0

  def __call__(self, fn):
    def _wrapper(*args, **kwargs):
      if time.time() > self.last_call + self.period:
        fn(*args, **kwargs)
        self.last_call = time.time()
    return _wrapper


def sigmoid(x):
  return 1 / (1 + math.exp(-x))


def pick_group():
  distro = [
    ("group_1", 0.4),
    ("group_2", 0.7),
    ("group_3", 0.8),
    ("group_4", 1.0),
  ]
  p = random.random()
  for group, cdf in distro:
    if p <= cdf:
      return group
  return distro[-1][0]


class Post(object):

  def __init__(self):
    self.plike = 0.1 # np.random.beta(2, 18)
    self.group = pick_group()


def make_batches(n):
  def __batch_generator():
    batches = []
    for i in range(1000):
      batches.append([
        Post() for i in range(int(1000 * np.random.beta(2, 18)))
      ])
    while True:
      for batch in batches:
        yield batch
      random.shuffle(batches)

  return list(itertools.islice(__batch_generator(), n))


class Stats(object):

  def __init__(self, name):
    self.name = name
    self.group_likes = Counter()
    self.num_batches = 0

  def consume(self, batch):
    depth = int(len(batch) * np.random.beta(5, 10))
    for post in batch[:depth]:
      self.group_likes[post.group] += post.plike
    self.num_batches += 1

  def log(self):
    print "Stats (%s):" % self.name
    print "\tnum_batches => %d" % self.num_batches
    for key, cnt in sorted(self.group_likes.iteritems()):
      print "\t%s => %d" % (key, cnt)
    print

  def log_relative(self, other):
    print "Relative Stats (%s):" % self.name
    print "\tnum_batches => %d" % self.num_batches
    for key, cnt in sorted(self.group_likes.iteritems()):
      ratio = float(cnt) / other.group_likes[key]
      print "\t%s => %.3f" % (key, 100 * ratio - 100)
    print


class PolicyParam:

  def __init__(self, name, value):
    self.name = name
    self.value = value


class ConstBoostPolicy:

  def __init__(self):
    self.params = [
      PolicyParam("group_1", 0.0),
      PolicyParam("group_2", 0.0),
      PolicyParam("group_3", 0.0),
      PolicyParam("group_4", 0.0),
    ]

  def score(self, post):
    for param in self.params:
      if param.name == post.group:
        return sigmoid(param.value) * post.plike
    return param.value

  def log(self):
    print "Policy:"
    for param in self.params:
      print "\t%s = %.5f (%.5f)" % (param.name, param.value, sigmoid(param.value))
    print


def measure_stats(policy, batches, name="default"):
  stats = Stats(name)
  for batch in batches:
    batch.sort(key=lambda post: policy.score(post), reverse=True)
    stats.consume(batch)
  return stats


def measure_loss(policy, batches):
  w = {
    "group_1": 0.25,
    "group_2": 0.25,
    "group_3": 0.25,
    "group_4": 0.25,
  }
  stats = measure_stats(policy, batches)
  return sum(
    w[g] * math.log(1.0 + float(cnt) / stats.num_batches)
    for g, cnt in stats.group_likes.iteritems()
  )


class PolicyOptimizer(object):

  def __init__(self, default_policy):
    self.policy = default_policy

  def eval_policy(self, policy, num_samples=1, num_batches=1000):
    return measure_loss(policy, make_batches(num_batches))
    losses = np.array([
      measure_loss(policy, make_batches(num_batches)) for _ in range(num_samples)
    ])
    print "mean:", np.mean(losses)
    print "s.d.", np.std(losses)
    return np.mean(losses)

  def run(self, num_steps=100, learning_rate=0.00001, gradient_delta=0.001):
    for _ in range(num_steps):
      self.policy.log()
      base_loss = self.eval_policy(self.policy)
      print "loss", base_loss

      gradient = []
      for i, param in enumerate(self.policy.params):
        param_policy = ConstBoostPolicy()
        param_policy.params = [
          PolicyParam(p.name, p.value) for p in self.policy.params
        ]
        param_policy.params[i].value += gradient_delta
        loss = self.eval_policy(self.policy)
        print "gradient_loss:", loss
        gradient.append((loss - base_loss) / gradient_delta)

      for i, update in enumerate(gradient):
        self.policy.params[i].value += learning_rate * update

    return self.policy


def test_policy():
  batches = make_batches(10000)

  policy = ConstBoostPolicy()
  stats = measure_stats(policy, batches, "baseline")
  stats.log()

  policy.params[0].value = 0.5
  stats_1 = measure_stats(policy, batches, "test_1")
  stats_1.log_relative(stats)

  policy.params[1].value = 0.8
  stats_2 = measure_stats(policy, batches, "test_2")
  stats_2.log_relative(stats)


def test_optimizer():
  optimizer = PolicyOptimizer(ConstBoostPolicy())
  test_policy = optimizer.run()
  base_policy = ConstBoostPolicy()

  print "\nBase policy:"
  for param in base_policy.params:
    print "%s: %.2f" % (param.name, param.value)

  print "\nTest policy:"
  for param in test_policy.params:
    print "%s: %.2f" % (param.name, param.value)

  batches = make_batches(10000)
  base_stats = measure_stats(base_policy, batches)
  test_stats = measure_stats(test_policy, batches)
  base_stats.log()
  test_stats.log()
  test_stats.log_relative(base_stats)

  print "Base loss:", measure_loss(base_policy, batches)
  print "Test loss:", measure_loss(test_policy, batches)


def test_noisy_function():
  def fn(x):
    return 0.01 * x**2 + 1.5 * x

  X = np.linspace(-10.0, 10.0, 100)
  Y = np.array([fn(x) for x in X])
  plt.plot(X, Y)
  plt.savefig('noise_fn1.png')


if __name__ == "__main__":
  #test_policy()
  #test_optimizer()
  test_noisy_function()
