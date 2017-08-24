
import skfuzzy
import numpy as np
import logging, sys

from python.routing.mte import *
from python.utils.utils import *
from python.network.node import *
from python.network.network import Network
from python.routing.routing_protocol import *
from python.sleep_scheduling.sleep_scheduler import *
import config as cf

"""Every node communicate its position to the base station. Then the 
BS uses FCM to define clusters and broadcast this information to the
network. Finally, a round is executed.
"""
class FCM(RoutingProtocol):

  #def _initial_setup(self, network):
  def _setup_phase(self, network):
    """The base station uses Fuzzy C-Means to clusterize the network. The
    optimal number of clusters is calculated. Then FCM is used to select
    the heads (centroids) for each cluster (only in the initial round).
    Then each cluster head chooses a new cluster head for the next round.
    Referece:
      D. C. Hoang, R. Kumar and S. K. Panda, "Fuzzy C-Means clustering 
      protocol for Wireless Sensor Networks," 2010 IEEE International 
      Symposium on Industrial Electronics, Bari, 2010, pp. 3477-3482.
    """
    logging.debug('FCM: setup phase')
  
    sensor_nodes = network.get_sensor_nodes()
    # calculate the average distance to the BS
    transform = lambda node: calculate_distance(node, network.get_BS())
    distances_to_BS = [transform(node) for node in sensor_nodes]
    avg_distance_to_BS = np.average(distances_to_BS)
    nb_clusters = calculate_nb_clusters(avg_distance_to_BS)
    # using a constant because calculating this value on-the-fly gives
    # different result than the paper
    nb_clusters = cf.NB_CLUSTERS
  
    # format data to shape expected by skfuzzy API
    data = [[node.pos_x, node.pos_y] for node in network[0:-1]]
    data = np.array(data).transpose()
    centroids, membership = skfuzzy.cluster.cmeans(data, nb_clusters,
                                                   cf.FUZZY_M, error=0.005,
                                                   maxiter=1000, 
                                                   init=None)[0:2]
    # assign node nearest to centroid as cluster head
    heads = []
    # also annotates centroids to network
    network.centroids = []
    for cluster_id, centroid in enumerate(centroids):
      tmp_centroid = Node(0)
      tmp_centroid.pos_x = centroid[0]
      tmp_centroid.pos_y = centroid[1]
      network.centroids.append(tmp_centroid)
      nearest_node = None
      shortest_distance = cf.INFINITY
      for node in network[0:-1]:
        distance = calculate_distance(node, tmp_centroid)
        if distance < shortest_distance:
          nearest_node      = node
          shortest_distance = distance
      nearest_node.next_hop   = cf.BSID
      nearest_node.membership = cluster_id
      heads.append(nearest_node)
  
    # assign ordinary network to cluster heads using fcm
    for i, node in enumerate(network[0:-1]):
      if node in heads: # node is already a cluster head
        continue
      cluster_id      = np.argmax(membership[:,i])
      node.membership = cluster_id
      head = [x for x in heads if x.membership == cluster_id][0]
      node.next_hop   = head.id

    self.head_rotation(network)
  
  #def _setup_phase(self, network):
  def head_rotation(self, network):
    logging.debug('FCM: head rotation')
    # head rotation
    # current cluster heads choose next cluster head with the most
    # residual energy and nearest to the cluster centroid
    for cluster_id in range(0, cf.NB_CLUSTERS):
      cluster = network.get_nodes_by_membership(cluster_id)
      # check if there is someone alive in this cluster
      if len(cluster) == 0:
        continue
  
      # someone is alive, find node with highest energy in the cluster
      # to be the next cluster head
      highest_energy = cf.MINUS_INFINITY
      next_head      = None
      for node in cluster:
        if node.energy_source.energy > highest_energy:
          highest_energy = node.energy_source.energy
          next_head      = node
  
      for node in cluster:
        node.next_hop = next_head.id
      next_head.next_hop = cf.BSID


# code temporary ommited
#def FCM_MTE_round(network, round_nb, local_traces=None, ret=None):
#  """Every node communicate its position to the base station. Then the 
#  BS uses FCM to define clusters and broadcast this information to the
#  network. Finally, a round is executed.
#  """
#  setup_phase_fcm(network, round_nb)
#  heads = Network(init_network=network.get_heads()+[network.get_BS()])
#  setup_phase_mte(heads)
#  network.broadcast_next_hop()
#  network.run_round(round_nb)


#def FCM_PSO_round(network, round_nb, local_traces=None, sleep_schedulers=None):
#  """Every node communicate its position to the base station. Then the 
#  BS uses FCM to define clusters and broadcast this information to the
#  network. Finally, a round is executed.
#  """
#  setup_phase_fcm(network, round_nb)
#  if round_nb == 0: # clusters do not change in FCM
#    clusters         = network.split_in_clusters()
#    sleep_schedulers = [SleepScheduler(cluster) for cluster in clusters]
#
#  for sleep_scheduler in sleep_schedulers:
#    sleep_scheduler.schedule()
#  network.run_round(round_nb)
#
#  return sleep_schedulers

