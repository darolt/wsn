
import skfuzzy
import numpy as np
from mte import *
import logging, sys
from utils import *
from node import *
from nodes import Nodes
from pso_wrapper import *

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
    sensor_nodes = nodes.get_sensor_nodes()
    # calculate the average distance to the BS
    transform = lambda node: calculate_distance(node, nodes.get_BS())
    distances_to_BS = [transform(node) for node in sensor_nodes]
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

    # assign ordinary nodes to cluster heads using fcm
    for i, node in enumerate(nodes[0:-1]):
      if node in heads: # node is already a cluster head
        continue
      cluster_id      = np.argmax(membership[:,i])
      node.membership = cluster_id
      head = [x for x in heads if x.membership == cluster_id][0]
      node.next_hop   = head.id

    # uncomment next line if you want to see the cluster assignment
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

def FCM_MTE_round(nodes, round_nb, local_traces=None, ret=None):
  """Every node communicate its position to the base station. Then the 
  BS uses FCM to define clusters and broadcast this information to the
  nodes. Finally, a round is executed.
  """
  setup_phase_fcm(nodes, round_nb)
  heads = Nodes(init_nodes=nodes.get_ch_nodes()+[nodes.get_BS()])
  setup_phase_mte(heads)
  nodes.broadcast_next_hop()
  nodes.run_round(round_nb)

def FCM_round(nodes, round_nb, local_traces=None, ret=None):
  """Every node communicate its position to the base station. Then the 
  BS uses FCM to define clusters and broadcast this information to the
  nodes. Finally, a round is executed.
  """
  setup_phase_fcm(nodes, round_nb)
  nodes.broadcast_next_hop()
  nodes.run_round(round_nb)

def FCM_PSO_round(nodes, round_nb, local_traces=None, pso_wrappers=None):
  """Every node communicate its position to the base station. Then the 
  BS uses FCM to define clusters and broadcast this information to the
  nodes. Finally, a round is executed.
  """
  setup_phase_fcm(nodes, round_nb)
  if round_nb == 0: # clusters do not change in FCM
    clusters     = nodes.split_in_clusters()
    pso_wrappers = [PSOWrapper(cluster) for cluster in clusters]

  for pso_wrapper in pso_wrappers:
    pso_wrapper.sleep_scheduling()
  nodes.broadcast_next_hop()
  nodes.run_round(round_nb)

  return pso_wrappers

