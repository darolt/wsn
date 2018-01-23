import config as cf
import numpy as np
from python.network.energy_source import *
from python.utils.utils import *

class Node(object):
  def __init__(self, id, parent = None):
    self.pos_x = np.random.uniform(0, cf.AREA_WIDTH)
    self.pos_y = np.random.uniform(0, cf.AREA_LENGTH)

    if id == cf.BSID:
      self.energy_source = PluggedIn(self)
    else:
      self.energy_source = Battery(self)

    self.id = id
    self.network_handler = parent

    self.reactivate()

  def reactivate(self):
    """Reactivate nodes for next simulation."""
    self.alive = 1
    self.tx_queue_size = 0
    self._next_hop = cf.BSID
    self.distance_to_endpoint = 0
    self.amount_sensed = 0
    self.amount_transmitted = 0
    self.amount_received = 0
    self.membership = cf.BSID
    # aggregation function determines the cost of forwarding messages
    # (in number of bits)
    self.aggregation_function = lambda x: 0
    self.time_of_death = cf.INFINITY
    self._is_sleeping = 0
    self.sleep_prob = 0.0
    # for coverage purposes
    self.neighbors = []
    self.nb_neighbors = -1
    self.exclusive_radius = 0

  @property
  def next_hop(self):
    return self._next_hop

  @next_hop.setter
  def next_hop(self, value):
    self._next_hop = value
    distance = calculate_distance(self, self.network_handler[value])
    self.distance_to_endpoint = distance

  @property
  def is_sleeping(self):
    if self.is_head():
      self._is_sleeping = 0
    return self._is_sleeping

  @is_sleeping.setter
  def is_sleeping(self, value):
    """Cluster heads cannot be put to sleep."""
    self._is_sleeping = value if not self.is_head() else 0
    
  def _only_active_nodes(func):
    """This is a decorator. It wraps all energy consuming methods to
    ensure that only active nodes execute this method. Also it automa-
    tically calls the battery. 
    """
    def wrapper(self, *args, **kwargs):
      if self.alive and not self.is_sleeping:
        func(self, *args, **kwargs)
        return 1
      else:
        return 0
    return wrapper

  @_only_active_nodes
  def update_sleep_prob(self):
    """Update the sleep probability. This method supposes that the
    endpoint is the cluster head
    """
    # cluster heads should not go to sleep. Neither dead nodes.
    if self.next_hop == cf.BSID:
      self.sleep_prob = 0.0
    else:
      self.sleep_prob = 0.5

    return

  @_only_active_nodes
  def update_sleep_prob2(self, nb_neighbors):
    """Updates the sleep probability according to paper X."""
    if self.next_hop == cf.BSID:
      self.sleep_prob = 0.0
    if nb_neighbors == 0:
      term1 = 0
    else:
      term1 = PSO_E*(nb_neighbors-1)/nb_neighbors
    if self.distance_to_endpoint == 0:
      term2 = 0
    else:
      term2 = PSO_F*(self.distance_to_endpoint-1)/self.distance_to_endpoint
    self.sleep_prob = term1 + term2

  def is_head(self):
    if self.next_hop == cf.BSID and self.id != cf.BSID and self.alive:
      return 1
    return 0

  def is_ordinary(self):
    return 1 if self.next_hop != cf.BSID and self.id != cf.BSID else 0

  @_only_active_nodes
  def _aggregate(self, msg_length):
    logging.debug("node %d aggregating." % (self.id))
    # number of bits to be sent increase while forwarding messages
    aggregation_cost = self.aggregation_function(msg_length)
    self.tx_queue_size += aggregation_cost

    # energy model for aggregation
    energy = cf.E_DA * aggregation_cost
    self.energy_source.consume(energy)

  @_only_active_nodes
  def transmit(self, msg_length=None, destination=None):
    logging.debug("node %d transmitting." % (self.id))
    if not msg_length:
      msg_length = self.tx_queue_size
    msg_length += cf.HEADER_LENGTH

    if not destination:
      destination = self.network_handler[self.next_hop]
      distance = self.distance_to_endpoint
    else:
      distance = calculate_distance(self, destination)

    # transmitter energy model
    energy = cf.E_ELEC
    if distance > cf.THRESHOLD_DIST:
      energy += cf.E_MP * (distance**4)
    else:
      energy += cf.E_FS * (distance**2)
    energy *= msg_length

    # automatically call other endpoint receive
    destination.receive(msg_length)
    # after the message is sent, queue is emptied 
    self.tx_queue_size = 0
    self.amount_transmitted += msg_length

    self.energy_source.consume(energy)

  @_only_active_nodes
  def receive(self, msg_length):
    logging.debug("node %d receiving." % (self.id))
    self._aggregate(msg_length - cf.HEADER_LENGTH)

    self.amount_received += msg_length

    # energy model for receiver
    energy = cf.E_ELEC * msg_length
    self.energy_source.consume(energy)

  @_only_active_nodes
  def sense(self):
    self.tx_queue_size = cf.MSG_LENGTH
    self.amount_sensed += cf.MSG_LENGTH

  def battery_depletion(self):
    self.alive = 0
    self.sleep_prob = 0.0
    self.time_of_death = self.network_handler.round
    self.network_handler.deaths_this_round += 1

