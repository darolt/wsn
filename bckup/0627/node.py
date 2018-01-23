from config import *
import numpy as np
from energy_source import *
from utils import *

class Node():
  def __init__(self, id, parent = None):
    self.pos_x = np.random.uniform(0, AREA_WIDTH)
    self.pos_y = np.random.uniform(0, AREA_LENGTH)

    if id == BSID:
      self.energy_source = PluggedIn(self)
    else:
      self.energy_source = Battery(self)
    self.id = id
    self.next_hop = BSID
    self.alive = 1
    self.tx_queue_size = 0
    self.nodes_handler = parent
    self.distance_to_endpoint = 0
    self.amount_sensed = 0
    self.amount_transmitted = 0
    self.amount_received = 0
    self.membership = BSID
    self.zero_cost_aggregation = 0
    # PSO-related attributes
    self.nb_neighbors = -1
    self.neighbors = []
    self.is_sleeping = 0
    self.sleep_prob = 0

  def calculate_sleep_prob(self, nb_neighbors):
    """Calculate the sleep probability. This method supposes that the
    endpoint is the cluster head
    """
    if self.next_hop == BSID:
      self.sleep_prob = 0
      return

    if nb_neighbors == 0:
      term1 = 0
    else:
      term1 = PSO_E*(nb_neighbors-1)/nb_neighbors
    if self.distance_to_endpoint == 0:
      term2 = 0
    else:
      term2 = PSO_F*(self.distance_to_endpoint-1)/self.distance_to_endpoint
    self.sleep_prob = term1 + term2

  def reactivate(self):
    """Reactivate nodes for next simulation."""
    self.alive = 1
    self.tx_queue_size = 0

  def is_cluster_head(self):
    return 1 if self.next_hop == BSID and self.id != BSID else 0

  def is_ordinary_node(self):
    return 1 if self.next_hop != BSID and self.id != BSID else 0

  def update_distance_to_endpoint(self, endpoint=None):
    """Update the distance to endpoint for energy consumption purposes.
    """
    if not endpoint:
      endpoint = self.nodes_handler[self.next_hop]
    distance = calculate_distance(self, endpoint)
    self.distance_to_endpoint = distance
    return distance

  def _only_active_nodes(func):
    """This is a decorator. It wraps all energy consuming methods to
    ensure that only active nodes execute this method. Also it automa-
    tically calls the battery. 
    """
    def wrapper(self, *args, **kwargs):
      if self.alive and not self.is_sleeping:
        energy = func(self, *args, **kwargs)
        self.energy_source.consume(energy)
        return 1
      else:
        return 0
    return wrapper
    
  @_only_active_nodes
  def _aggregate(self, msg_length):
    logging.debug("node %d aggregating." % (self.id))
    # number of bits to be sent increase while forwarding messages
    if not self.zero_cost_aggregation:
      self.tx_queue_size += msg_length

    # energy model for aggregation
    energy = E_DA * msg_length
    return energy

  @_only_active_nodes
  def transmit(self, msg_length=None, destination=None):
    logging.debug("node %d transmitting." % (self.id))
    if not msg_length:
      msg_length = self.tx_queue_size
    msg_length += HEADER_LENGTH

    if not destination:
      destination = self.nodes_handler[self.next_hop]

    self.update_distance_to_endpoint(destination)
    # transmitter energy model
    energy = E_ELEC
    if self.distance_to_endpoint > THRESHOLD_DIST:
      energy += E_MP * (self.distance_to_endpoint**4)
    else:
      energy += E_FS * (self.distance_to_endpoint**2)
    energy *= msg_length

    # automatically call other endpoint receive
    destination.receive(msg_length)
    # after the message is sent, queue is emptied 
    self.tx_queue_size = 0
    self.amount_transmitted += msg_length

    return energy

  @_only_active_nodes
  def receive(self, msg_length):
    logging.debug("node %d receiving." % (self.id))
    self._aggregate(msg_length - HEADER_LENGTH)

    self.amount_received += msg_length

    # energy model for receiver
    energy = E_ELEC * msg_length

    return energy

  @_only_active_nodes
  def sense(self):
    self.tx_queue_size = MSG_LENGTH
    self.amount_sensed += MSG_LENGTH
    return 0

