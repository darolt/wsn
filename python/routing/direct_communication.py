import logging, sys
import config as cf

from python.routing.routing_protocol import *

class DC(RoutingProtocol):
  def pre_communication(self, network):
    """Setup all the point-to-point connections for the direct communica-
    tion scenario. In this scenario, the setup is executed only once, and
    all nodes send information directly to the base station.
    """
    logging.info('Direct Communication: Setup phase')
    for node in network:
      node.next_hop = cf.BSID

  def broadcast(self, network):
    pass

