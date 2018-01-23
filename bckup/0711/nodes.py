from config import *
import logging
from node import *
from grid import *
import matplotlib.pyplot as plt
from utils import *

class Nodes(list):
  """This class stores a list with all network nodes plus the base sta-
  tion. Its methods ensure the network behavior.
  """
  def __init__(self, init_nodes=None):
    logging.debug('Instantiating nodes...')
    if init_nodes:
      self.extend(init_nodes)
    else:
      nodes = [Node(i, self) for i in range(0,NB_NODES)]
      self.extend(nodes)
      # last node in nodes is the base station
      base_station = Node(BSID, self)
      base_station.pos_x = BS_POS_X
      base_station.pos_y = BS_POS_Y
      self.append(base_station)
    
    self._dict = {}
    for node in self:
      self._dict[node.id] = node

    self._perform_two_level_comm = 0
    self.round = 0

    self.initial_energy = self.get_remaining_energy()

  def reset(self):
    """Set nodes to initial state so the same placement of nodes can be
    used by different techniques.
    """
    for node in self:
      node.energy_source.recharge()
      node.reactivate()

    self.round = 0

  def set_perform_two_level_comm(self, value):
    """Setter."""
    self._perform_two_level_comm = value

  def run_round(self, round):
    """Run one round. Every node captures using its sensor. Then this
    information is forwarded through the intermediary nodes to the base
    station.
    """
    self.round = round
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
    #ordinary_nodes = self.get_ordinary_nodes()
    #heads = self.get_ch_nodes()
    #msg = str("%d ordinary nodes, %d heads." % (len(ordinary_nodes), len(heads)))
    #logging.debug("Hierarchical communication: %s" % (msg))

    alive_nodes = self.get_alive_nodes()
    if self._perform_two_level_comm:
      self._two_level_comm(alive_nodes)
    else:
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

  def _two_level_comm(self, alive_nodes):
    """This method performs communication supposing that there are only
    ordinary nodes and cluster heads, this method is less generic than
    its recursive version, but it is faster.
    """
    for node in alive_nodes:
      if node.is_ordinary():
        node.transmit()
    for node in alive_nodes:
      if node.is_head():
        node.transmit()

  def get_alive_nodes(self):
    """Return nodes that have positive remaining energy."""
    return [node for node in self[0:-1] if node.alive]

  def get_active_nodes(self):
    """Return nodes that have positive remaining energy and that are
    awake."""
    is_active = lambda x: x.alive and not x.is_sleeping
    return [node for node in self[0:-1] if is_active(node)]

  def get_ordinary_nodes(self):
    return [node for node in self if node.is_ordinary() and node.alive]

  def get_ch_nodes(self, only_alives=1):
    input_set = self.get_alive_nodes() if only_alives else self
    return [node for node in input_set if node.is_head()]

  def get_sensor_nodes(self):
    """Return all nodes except base station."""
    return [node for node in self[0:-1]]

  def someone_alive(self):
    """Finds if there is at least one node alive. It excludes the base station,
       which is supposed to be always alive."""
    for node in self[0:-1]:
      if node.alive == 1:
        return 1
    return 0

  def count_alive_nodes(self):
    return sum(x.alive for x in self[:-1])

  def get_BS(self):
    # intention: make code clearer for non-Python readers
    return self[-1]

  def get_node(self, id):
    """By default, we assume that the id is equal to the node's posi-
    tion in the list, but that may not be always the case.
    """
    return self._dict[id]

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

  def get_remaining_energy(self, ignore_nodes=None):
    """Returns the sum of the remaining energies at all nodes."""
    set = self.get_alive_nodes()
    if len(set) == 0:
      return 0
    if ignore_nodes:
      set = [node for node in set if node not in ignore_nodes]
    transform = lambda x: x.energy_source.energy
    energies = [transform(x) for x in set]
    return sum(x for x in energies)

  def set_aggregation_function(self, function):
    """Sets the function that determines the cost of aggregation."""
    for node in self:
      node.aggregation_function = function
    
  def split_in_clusters(self, nb_clusters=LEACH_NB_CLUSTERS):
    """Split this nodes object into other nodes objects that contain only
    information about a single cluster."""
    # TODO: add BS as last element to keep compatibility between node 
    # and cluster
    clusters = []
    for cluster_idx in range(0, nb_clusters):
      nodes = self.get_nodes_by_membership(cluster_idx)
      cluster = Nodes(init_nodes=nodes)
      cluster.append(self.get_BS())
      clusters.append(cluster)
    return clusters

  def _calculate_nb_neighbors(self, target_node, coverage_radius):
    """Calculate the number of neighbors given the sensor coverage
    radius.
    """
    # if number of neighbors was calculated at least once
    # skips calculating the distance
    if target_node.nb_neighbors != -1:
      # only check if there are dead nodes
      all_neighbors = target_node.neighbors
      nb_dead_neighbors = sum(1 for x in all_neighbors if not x.alive)
      target_node.neighbors[:] = [x for x in all_neighbors if x.alive]
      return target_node.nb_neighbors - nb_dead_neighbors

    nb_neighbors = 0
    for node in self.get_alive_nodes():
      if node != target_node:
        distance = calculate_distance(target_node, node)
        if distance <= coverage_radius:
          nb_neighbors += 1
          target_node.neighbors.append(node) 
    return nb_neighbors

  def update_sleep_prob(self):
    for node in self.get_alive_nodes():
      nb_neighbors = self._calculate_nb_neighbors(node, COVERAGE_RADIUS)
      node.nb_neighbors = nb_neighbors
      node.update_sleep_prob(nb_neighbors)

  def plot_time_of_death(self):
    x = [node.pos_x for node in self[0:-1]]
    y = [node.pos_y for node in self[0:-1]]
    z = [node.time_of_death for node in self[0:-1]]
    plot_time_of_death(x, y, z)
