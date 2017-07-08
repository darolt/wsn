# -*- coding: utf-8 -*-
import math
import logging, sys
from config import *
from node import *
from nodes import Nodes
from utils import *
from dijkstra import *
import skfuzzy
import numpy as np
from pso import *

logging.basicConfig(stream=sys.stderr, level=logging.INFO)

def calculate_nb_clusters(avg_distance_to_BS):
  """Calculate the optimal number of clusters for FCM."""
  term1 = math.sqrt(NB_NODES)/(math.sqrt(2*math.pi))
  term2 = THRESHOLD_DIST
  term3 = AREA_WIDTH/(avg_distance_to_BS**2)
  return int(term1*term2*term3)

def setup_phase_mte(nodes):
  """The base station decides the next-hop for every node using
  Dijkstra's algorithm (shortest path). Then it broadcasts this infor-
  mation to all nodes. This function builds a graph with weights/cost
  related to each pair of nodes. The weights are not the Euclidean dis-
  nces, but rather a funcion of distances. If the distance is greater
  than THRESHOLD_DIST d^4 i used, otherwise d^2 is used. This comes
  from the energy model (see reference).
  Reference:
    M. Ettus. System Capacity, Latency, and Power Consumption in Multi-
    hop-routed SS-CDMA Wireless Networks. In Radio and Wireless Confe-
    rence (RAWCON 98), pages 55–58, Aug. 1998
  """
  logging.info('MTE: setup phase')

  # generate cost graph only for alive nodes (in dict form):
  # origin_id: {dest_id1: cost1, dest_id2: cost2, ...}, ...
  alive_nodes = nodes.get_alive_nodes()
  alive_nodes_and_BS = alive_nodes + [nodes.get_BS()]
  G = {}
  for node in alive_nodes_and_BS:
    G[node.id] = {}
    for other in alive_nodes_and_BS:
      if other == node:
        continue
      distance = calculate_distance(node, other)
      cost = distance**2 if distance < THRESHOLD_DIST else distance**4
      G[node.id][other.id] = cost

  # calculate shortest path and set next_hop accordingly
  done = []
  while len(alive_nodes) != 0:
    starting_node = alive_nodes[0]
    shortest_path = shortestPath(G, starting_node.id, BSID)
    for i, id in enumerate(shortest_path):
      if id == BSID or id in done:
        break
      nodes[id].next_hop = shortest_path[i+1]
      alive_nodes = [node for node in alive_nodes if node.id != id]
      done.append(id)

def run_mte(nodes):
  """Every node communicate its position to the base station. Then the 
  BS uses MTE to choose the routes and broadcasts this information to 
  the nodes. Finally, a round is executed.
  """
  logging.info('MTE: running scenario...')

  trace_alive_nodes = []
  nodes.notify_position()
  for round in range(0, MAX_ROUNDS):
    print_args = (round, nodes.get_remaining_energy())
    print("round %d: total remaining energy: %f" % print_args)
    if not nodes.someone_alive():
      break
    trace_alive_nodes.append(nodes.count_alive_nodes())
    # runs setup phase only if some node has died, otherwise the setup remains the same
    # this logic is used to improve simulation time
    if round == 0 or trace_alive_nodes[-1] != trace_alive_nodes[-2]:
      setup_phase_mte(nodes)
      nodes.broadcast_next_hop()
    nodes.run_round()

  return trace_alive_nodes

def setup_phase_dc(nodes):
  """Setup all the point-to-point connections for the direct communica-
  tion scenario. In this scenario, the setup is executed only once, and
  all nodes send information directly to the base station.
  """
  logging.info('Direct Communication: Setup phase')
  for node in nodes:
    node.next_hop = BSID

def run_direct_communication(nodes):
  """Run the direct communication scenario"""
  logging.info('Running direct communication scenario...')
  # set each node to send info to the base station
  setup_phase_dc(nodes[0:-1])

  nb_alive_nodes = []
  for round in range(0, MAX_ROUNDS):
    print_args = (round, nodes.get_remaining_energy())
    print("round %d: total remaining energy: %f" % print_args)
    if not nodes.someone_alive():
      break
    nb_alive_nodes.append(nodes.count_alive_nodes())
    nodes.run_round()

  return nb_alive_nodes

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

def run_leach(nodes):
  """Run the LEACH scenario."""
  logging.info('LEACH: running scenario...')
  trace_alive_nodes = []
  nodes.notify_position()
  for round in range(0, MAX_ROUNDS):
    print_args = (round, nodes.get_remaining_energy())
    print("round %d: total remaining energy: %f" % print_args)
    if not nodes.someone_alive():
      break
    trace_alive_nodes.append(nodes.count_alive_nodes())
    setup_phase_leach(nodes)
    nodes.broadcast_next_hop()
    nodes.run_round()

  return trace_alive_nodes

def setup_phase_fcm(nodes, round):
  """The base station uses Fuzzy C-Means to clusterize the network. The
  optimal number of clusters is calculated. Then FCM is used to select
  the heads (centroids) for each cluster (only in the initial round).
  Then each cluster head chooses a new cluster head for the next round.
  Referece:
    D. C. Hoang, R. Kumar and S. K. Panda, "Fuzzy C-Means clustering 
    protocol for Wireless Sensor Networks," 2010 IEEE International 
    Symposium on Industrial Electronics, Bari, 2010, pp. 3477-3482.
  """
  logging.info('FCM: setup phase')
  if round == 0:
    # calculate the average distance to the BS
    transform = lambda node: calculate_distance(node, nodes.get_BS())
    distances_to_BS = [transform(node) for node in nodes[0:-1]]
    avg_distance_to_BS = np.average(distances_to_BS)
    nb_clusters = calculate_nb_clusters(avg_distance_to_BS)
    # using a constant because calculating this value on-the-fly gives
    # different result than the paper
    nb_clusters = LEACH_NB_CLUSTERS

    # format data to shape expected by skfuzzy API
    data = [[node.pos_x, node.pos_y] for node in nodes[0:-1]]
    data = np.array(data).transpose()
    centroids, membership = skfuzzy.cluster.cmeans(data, nb_clusters,
                                                   FUZZY_M, error=0.005,
                                                   maxiter=1000, 
                                                   init=None)[0:2]
    # assign nearest node to centroid as cluster head
    tmp_centroid = Node(0)
    heads = []
    for cluster_id, centroid in enumerate(centroids):
      tmp_centroid.pos_x = centroid[0]
      tmp_centroid.pos_y = centroid[1]
      nearest_node = None
      shortest_distance = INFINITY
      for node in nodes[0:-1]:
        distance = calculate_distance(node, tmp_centroid)
        if distance < shortest_distance:
          nearest_node      = node
          shortest_distance = distance
      nearest_node.next_hop   = BSID
      nearest_node.membership = cluster_id
      heads.append(nearest_node)
      print("ch %d membership %d" % (nearest_node.id, nearest_node.membership))

    # assign ordinary nodes to cluster heads using fcm
    for i, node in enumerate(nodes[0:-1]):
      if node in heads: # node is already a cluster head
        continue
      cluster_id      = np.argmax(membership[:,i])
      node.membership = cluster_id
      head = [x for x in heads if x.membership == cluster_id][0]
      node.next_hop   = head.id
    #plot_clusters(nodes)

  else: # head rotation
    # current cluster heads choose next cluster head with the most
    # residual energy and nearest to the cluster centroid
    for cluster_id in range(0, LEACH_NB_CLUSTERS):
      cluster = nodes.get_nodes_by_membership(cluster_id)
      # check if there is someone alive in this cluster
      if len(cluster) == 0:
        continue

      # someone is alive, find node with highest energy in the cluster
      # to be the next cluster head
      highest_energy = MINUS_INFINITY
      next_head      = None
      for node in cluster:
        if node.energy_source.energy > highest_energy:
          highest_energy = node.energy_source.energy
          next_head      = node

      for node in cluster:
        node.next_hop = next_head.id
      next_head.next_hop = BSID

def run_fcm(nodes):
  """Every node communicate its position to the base station. Then the 
  BS uses FCM to define clusters and broadcast this information to the
  nodes. Finally, a round is executed.
  """
  logging.info('FCM: running scenario...')
  trace_alive_nodes = []
  nodes.notify_position()
  for round in range(0, MAX_ROUNDS):
    print_args = (round, nodes.get_remaining_energy())
    print("round %d: total remaining energy: %f" % print_args)
    if not nodes.someone_alive():
      break
    trace_alive_nodes.append(nodes.count_alive_nodes())
    setup_phase_fcm(nodes, round)
    nodes.broadcast_next_hop()
    nodes.run_round()

  return trace_alive_nodes

def run_fcm_pso(nodes):
  """Every node communicate its position to the base station. Then the 
  BS uses FCM to define clusters and broadcast this information to the
  nodes. Finally, a round is executed.
  """
  logging.info('FCM: running scenario...')
  trace_alive_nodes = []
  nodes.notify_position()
  for round in range(0, MAX_ROUNDS):
    print_args = (round, nodes.get_remaining_energy())
    print("round %d: total remaining energy: %f" % print_args)
    if not nodes.someone_alive():
      break
    trace_alive_nodes.append(nodes.count_alive_nodes())
    setup_phase_fcm(nodes, round)
    if round == 0: # clusters do not change in FCM
      clusters     = nodes.split_in_clusters()
      pso_wrappers = [PSOWrapper(cluster) for cluster in clusters]

    for idx, cluster in enumerate(clusters):
      pso_wrappers[idx].sleep_scheduling(cluster)
    nodes.broadcast_next_hop()
    nodes.run_round()

  return trace_alive_nodes

def reset_nodes(nodes):
  """Set nodes to initial state so the same placement of nodes can be
  used by different techniques.
  """
  for node in nodes:
    node.energy_source.recharge()
    node.reactivate()


if __name__ == '__main__':
  nodes  = Nodes()
  trace_alive_nodes = {}
  if RUN_DC:
    trace_alive_nodes['DC'] = run_direct_communication(nodes)
    reset_nodes(nodes)
  if RUN_MTE:
    # in the FCM paper, authors suppose that a forwarded message
    # in MTE is entirely sent to the next hop, meaning that there
    # is no aggregation/compression
    nodes.set_zero_cost_aggregation(0)
    trace_alive_nodes['mte'] = run_mte(nodes)
    reset_nodes(nodes)
  if RUN_LEACH:
    nodes.set_zero_cost_aggregation(1)
    trace_alive_nodes['leach'] = run_leach(nodes)
    reset_nodes(nodes)
  if RUN_FCM:
    nodes.set_zero_cost_aggregation(1)
    nodes.set_perform_two_level_comm(1)
    if RUN_PSO:
      trace_alive_nodes['pso'] = run_fcm_pso(nodes)
    else:
      trace_alive_nodes['pso'] = run_fcm(nodes)
    reset_nodes(nodes)

  log_curves(trace_alive_nodes)
  plot_curves(trace_alive_nodes)

