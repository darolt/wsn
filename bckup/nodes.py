from config import *
import logging
from node import *

class Nodes(list):
  """This class stores a list with all network nodes plus the base sta-
  tion. Its methods ensure the network behavior.
  """
  def __init__(self):
    logging.debug('Instantiating nodes...')
    nodes = [Node(i, self) for i in range(0,NB_NODES)]
    self.extend(nodes)
    # last node in nodes is the base station
    base_station = Node(BSID, self)
    base_station.pos_x = BS_POS_X
    base_station.pos_y = BS_POS_Y
    self.append(base_station)

  def run_round(self):
    """Run one round. Every node captures using its sensor. Then this
    information is forwarded through the intermediary nodes to the base
    station.
    """
    for i in range(0, MAX_TX_PER_ROUND):
      self._sensing_phase()
      self._communication_phase()

  def _sensing_phase(self):
    """Every alive node captures information using its sensor."""
    for node in self.get_alive_nodes():
      node.sense()

  def _communication_phase(self):
    """Each node transmits respecting its hierarchy: leaves start the 
    communication, then cluster heads forward the messages, until all
    messages reach the base station. This method works for any hierar-
    chy (even for LEACH).
    """
    ordinary_nodes = self.get_ordinary_nodes()
    heads = self.get_ch_nodes()
    msg = str("%d ordinary nodes, %d heads." % (len(ordinary_nodes), len(heads)))
    logging.debug("Hierarchical communication: %s" % (msg))

    alive_nodes = self.get_alive_nodes()
    self._recursive_comm(alive_nodes)

  def _recursive_comm(self, alive_nodes):
    """Hierarchical communication using recursivity. This method suppo-
    ses that there is no cycle in the network (network is a tree).
    Otherwise, expect infinite loop.
    """
    # TODO store communication order and reuse it instead of calling
    # this function always
    next_alive_nodes = alive_nodes[:]
    for node in alive_nodes:
      #check if other nodes must send info to this node
      depends_on_other_node = 0
      for other_node in alive_nodes:
        #if other_node == node:
        #  continue
        if other_node.next_hop == node.id:
          depends_on_other_node = 1
          break

      if not depends_on_other_node:
        node.transmit()
        next_alive_nodes = [n for n in next_alive_nodes if n != node]

    if len(next_alive_nodes) == 0:
      return
    else:
      self._recursive_comm(next_alive_nodes)

  def get_alive_nodes(self):
    return [node for node in self[0:-1] if node.alive]

  def get_ordinary_nodes(self):
    return [node for node in self if node.is_ordinary_node() and node.alive]

  def get_ch_nodes(self, only_alives=1):
    input_set = self.get_alive_nodes() if only_alives else self
    return [node for node in input_set if node.is_cluster_head()]

  def someone_alive(self):
    """Finds if there is at least one node alive. It excludes the base station,
       which is supposed to be always alive."""
    for node in self[0:-1]:
      if node.alive == 1:
        return 1
    return 0

  def count_alive_nodes(self):
    count = 0
    for node in self:
      if node.alive == 1:
        count += 1
    return count

  def get_BS(self):
    # intention: make code clearer for non-Python readers
    return self[-1]

  def notify_position(self):
    """Every node transmit its position directly to the base station."""
    for node in self.get_alive_nodes():
      node.transmit(msg_length=MSG_LENGTH, destination=self.get_BS())

  def broadcast_next_hop(self):
    """Base station informs nodes about their next hop."""
    base_station = self.get_BS()
    for node in self.get_alive_nodes():
      base_station.transmit(msg_length=MSG_LENGTH, destination=node)

  def get_nodes_by_membership(self, membership, only_alives=1):
    """Returns all nodes that belong to this membership/cluster."""
    input_set = self.get_alive_nodes() if only_alives else self
    condition = lambda node: node.membership == membership
    return [node for node in input_set if condition(node)]

  def get_remaining_energy(self):
    """Returns the sum of the remaining energies at all nodes."""
    set = self.get_alive_nodes()
    if len(set) == 0:
      return 0
    transform = lambda x: x.energy_source.energy
    return reduce(lambda x, y: x+y, [transform(x) for x in set])

  def set_zero_cost_aggregation(self, value):
    for node in self:
      node.zero_cost_aggregation = value
    
