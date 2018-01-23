import logging, sys
from config import *

def DC_init(nodes):
  """Setup all the point-to-point connections for the direct communica-
  tion scenario. In this scenario, the setup is executed only once, and
  all nodes send information directly to the base station.
  """
  logging.info('Direct Communication: Setup phase')
  for node in nodes:
    node.next_hop = BSID

def DC_round(nodes, round_nb, local_traces, ret=None):
  nodes.run_round(round_nb)

