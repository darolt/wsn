# -*- coding: utf-8 -*-
import logging, sys
from python.routing.dijkstra import *
from python.utils.utils import *
from python.network.node import *
from python.network.network import Network
from python.routing.routing_protocol import *
import config as cf

class MTE(RoutingProtocol):
  def _find_shortest_path(self, network):
    """The base station decides the next-hop for every node using
    Dijkstra's algorithm (shortest path). Then it broadcasts this infor-
    mation to all network. This function builds a graph with weights/cost
    related to each pair of network. The weights are not the Euclidean dis-
    nces, but rather a funcion of distances. If the distance is greater
    than THRESHOLD_DIST d^4 i used, otherwise d^2 is used. This comes
    from the energy model (see reference).
    Reference:
      M. Ettus. System Capacity, Latency, and Power Consumption in Multi-
      hop-routed SS-CDMA Wireless Networks. In Radio and Wireless Confe-
      rence (RAWCON 98), pages 55–58, Aug. 1998
    """
    logging.info('MTE: setup phase')
  
    # generate cost graph only for alive network (in dict form):
    # origin_id: {dest_id1: cost1, dest_id2: cost2, ...}, ...
    alive_nodes = network.get_alive_nodes()
    alive_nodes_and_BS = alive_nodes + [network.get_BS()]
    G = {}
    for node in alive_nodes_and_BS:
      G[node.id] = {}
      for other in alive_nodes_and_BS:
        if other == node:
          continue
        distance = calculate_distance(node, other)
        cost = distance**2 if distance < cf.THRESHOLD_DIST else distance**4
        G[node.id][other.id] = cost
  
    # calculate shortest path and set next_hop accordingly
    done = []
    while len(alive_nodes) != 0:
      starting_node = alive_nodes[0]
      shortest_path = shortestPath(G, starting_node.id, cf.BSID)
      for i, id in enumerate(shortest_path):
        if id == cf.BSID or id in done:
          break
        network.get_node(id).next_hop = shortest_path[i+1]
        #network[id].next_hop = shortest_path[i+1]
        alive_nodes = [node for node in alive_nodes if node.id != id]
        done.append(id)

  def _setup_phase(self, network):
    """Every node communicate its position to the base station. Then the 
    BS uses MTE to choose the routes and broadcasts this information to 
    the network. Finally, a round is executed.
    """
    if network.deaths_this_round != 0:
      self._find_shortest_path(network)
      network.broadcast_next_hop()

  def _initial_setup(self, network):
    network.perform_two_level_comm = 0
    self._find_shortest_path(network)
    network.broadcast_next_hop()
  
