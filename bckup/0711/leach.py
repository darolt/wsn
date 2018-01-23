import logging, sys
from nodes import *
from config import *

def setup_phase_leach(nodes):
  """The base station decides which nodes are cluster heads in this
  round, depending on a probability. Then it broadcasts this information
  to all nodes.
  Reference:
    W. Heinzelman, A. Chandrakasan, and H. Balakrishnan, Energy-
    efficient communication protocols for wireless sensor networks, In
    Proceedings of the 33rd Annual Hawaii International Conference on
    System Sciences (HICSS), Hawaii, USA, January 2000.
  """
  logging.info('LEACH: setup phase.')
  # decide which nodes are cluster heads
  prob_ch = float(LEACH_NB_CLUSTERS)/float(NB_NODES)
  heads = []
  alive_nodes = nodes.get_alive_nodes()
  logging.info('LEACH: deciding which nodes are cluster heads.')
  idx = 0
  while len(heads) != LEACH_NB_CLUSTERS:
    node = alive_nodes[idx]
    u_random = np.random.uniform(0, 1)
    # node will be a cluster head
    if u_random < prob_ch:
      node.next_hop = BSID
      heads.append(node)

    idx = idx+1 if idx < len(alive_nodes)-1 else 0

  # ordinary nodes choose nearest cluster heads
  logging.info('LEACH: ordinary nodes choose nearest nearest cluster head')
  for node in alive_nodes:
    if node in heads: # node is cluster head
      continue
    nearest_head = heads[0]
    # find the nearest cluster head
    for head in heads[1:]:
      if calculate_distance(node, nearest_head) > calculate_distance(node, head):
        nearest_head = head

    node.next_hop = nearest_head.id

def LEACH_round(nodes, round_nb, local_traces=None, ret=None):
  setup_phase_leach(nodes)
  nodes.broadcast_next_hop()
  nodes.run_round(round_nb)

