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
    self.prev_round_as_ch = -10000
    self.amount_sensed = 0
    self.amount_transmitted = 0
    self.amount_received = 0
    self.membership = BSID
    self.zero_cost_aggregation = 0

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
    
  def aggregate(self, msg_length):
    if self.id == BSID or not self.alive:
      return # base station do not aggregate
    logging.debug("node %d aggregating." % (self.id))
    # dissipate energy while aggregating
    energy = E_DA * msg_length
    self.energy_source.consume(energy)

    # number of bits to be sent increase while forwarding messages
    if not self.zero_cost_aggregation:
      self.tx_queue_size += msg_length

  def transmit(self, msg_length=None, destination=None):
    if not self.alive:
      return
    logging.debug("node %d transmitting." % (self.id))
    if not msg_length:
      msg_length = self.tx_queue_size
    msg_length += HEADER_LENGTH

    if not destination:
      destination = self.nodes_handler[self.next_hop]

    self.update_distance_to_endpoint(destination)
    energy = E_ELEC
    if self.distance_to_endpoint > THRESHOLD_DIST:
      energy += E_MP * (self.distance_to_endpoint**4)
    else:
      energy += E_FS * (self.distance_to_endpoint**2)
    energy *= msg_length

    self.energy_source.consume(energy)
      
    destination.receive(msg_length)
    # after the message is sent, queue is emptied 
    self.tx_queue_size = 0
    self.amount_transmitted += msg_length

  def receive(self, msg_length):
    if not self.alive:
      return
    logging.debug("node %d receiving." % (self.id))
    energy = E_ELEC * msg_length
    self.energy_source.consume(energy)
    self.aggregate(msg_length - HEADER_LENGTH)

    self.amount_received += msg_length

  def sense(self):
    self.tx_queue_size = MSG_LENGTH
    self.amount_sensed += MSG_LENGTH

