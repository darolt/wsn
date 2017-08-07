
"""Aggregation cost functions. Determine the cost of cluster heads for-
   warding messages.
"""
def zero_cost_aggregation(msg_length):
  return 0

def total_cost_aggregation(msg_length):
  return msg_length

def linear_cost_aggregation(factor):
  """Defines a family of functions."""
  return lambda x: int(x*factor)

def log_cost_aggregation(msg_length):
  return int(math.log(msg_length))

