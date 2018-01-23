from config import *
import numpy as np
from energy_source import *
from utils import *

class Node(object):
  def __init__(self, id, parent = None):
    self.pos_x = np.random.uniform(0, AREA_WIDTH)
    self.pos_y = np.random.uniform(0, AREA_LENGTH)

    if id == BSID:
      self.energy_source = PluggedIn(self)
    else:
      self.energy_source = Battery(self)

    self.id = id
    self._next_hop = BSID
    self.alive = 1
    self.tx_queue_size = 0
    self.nodes_handler = parent
    self.distance_to_endpoint = 0
    self.amount_sensed = 0
    self.amount_transmitted = 0
    self.amount_received = 0
    self.membership = BSID
    # aggregation function determines the cost of forwarding messages
    # (in number of bits)
    self.aggregation_function = lambda x: 0
    # PSO-related attributes
    self.nb_neighbors = -1
    self.neighbors = []
    self._is_sleeping = 0
    self.sleep_prob = 0.0
    self.time_of_death = INFINITY

  @property
  def next_hop(self):
    return self._next_hop

  @next_hop.setter
  def next_hop(self, value):
    self._next_hop = value
    distance = calculate_distance(self, self.nodes_handler[value])
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
  def update_sleep_prob(self, nb_neighbors):
    """Update the sleep probability. This method supposes that the
    endpoint is the cluster head
    """
    # cluster heads should not go to sleep. Neither dead nodes.
    if self.next_hop == BSID:
      self.sleep_prob = 0.0
      return

    if nb_neighbors == 0:
      term1 = 0
    else:
      term1 = PSO_E*(nb_neighbors-1)/nb_neighbors
    if self.distance_to_endpoint == 0:
      term2 = 0
    else:
      term2 = PSO_F*(self.distance_to_endpoint-1)/self.distance_to_endpoint
    #self.sleep_prob = term1 + term2
    self.sleep_prob = 0.5

  def reactivate(self):
    """Reactivate nodes for next simulation."""
    self.alive = 1
    self.tx_queue_size = 0

  def is_head(self):
    return 1 if self.next_hop == BSID and self.id != BSID else 0

  def is_ordinary(self):
    return 1 if self.next_hop != BSID and self.id != BSID else 0

  @_only_active_nodes
  def _aggregate(self, msg_length):
    logging.debug("node %d aggregating." % (self.id))
    # number of bits to be sent increase while forwarding messages
    self.tx_queue_size += self.aggregation_function(msg_length)

    # energy model for aggregation
    energy = E_DA * msg_length
    self.energy_source.consume(energy)

  @_only_active_nodes
  def transmit(self, msg_length=None, destination=None):
    logging.debug("node %d transmitting." % (self.id))
    if not msg_length:
      msg_length = self.tx_queue_size
    msg_length += HEADER_LENGTH

    if not destination:
      destination = self.nodes_handler[self.next_hop]
      distance = self.distance_to_endpoint
    else:
      distance = calculate_distance(self, destination)

    # transmitter energy model
    energy = E_ELEC
    if distance > THRESHOLD_DIST:
      energy += E_MP * (distance**4)
    else:
      energy += E_FS * (distance**2)
    energy *= msg_length

    # automatically call other endpoint receive
    destination._receive(msg_length)
    # after the message is sent, queue is emptied 
    self.tx_queue_size = 0
    self.amount_transmitted += msg_length

    self.energy_source.consume(energy)

  @_only_active_nodes
  def _receive(self, msg_length):
    logging.debug("node %d receiving." % (self.id))
    self._aggregate(msg_length - HEADER_LENGTH)

    self.amount_received += msg_length

    # energy model for receiver
    energy = E_ELEC * msg_length
    self.energy_source.consume(energy)

  @_only_active_nodes
  def sense(self):
    self.tx_queue_size = MSG_LENGTH
    self.amount_sensed += MSG_LENGTH

  def battery_depletion(self):
    self.alive = 0
    self.sleep_prob = 0.0
    self.time_of_death = self.nodes_handler.round

