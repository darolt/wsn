# -*- coding: utf-8 -*-
from dijkstra import *
import logging, sys
from utils import *
from node import *
from nodes import Nodes

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
      nodes.get_node(id).next_hop = shortest_path[i+1]
      #nodes[id].next_hop = shortest_path[i+1]
      alive_nodes = [node for node in alive_nodes if node.id != id]
      done.append(id)

def MTE_round(nodes, round_nb, local_traces, ret=None):
  """Every node communicate its position to the base station. Then the 
  BS uses MTE to choose the routes and broadcasts this information to 
  the nodes. Finally, a round is executed.
  """
  if round_nb == 0 or local_traces['alive_nodes'][2][-1] != local_traces['alive_nodes'][2][-2]:
    setup_phase_mte(nodes)
    nodes.broadcast_next_hop()
  nodes.run_round(round_nb)
