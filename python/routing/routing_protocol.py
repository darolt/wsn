import config as cf

"""This class defines the interface that should be used when defining a new
routing protocol.
"""
class RoutingProtocol(object):
  def pre_communication(self, network):
    """This method is called before round 0."""
    if cf.NOTIFY_POSITION:
      network.notify_position()

  def setup_phase(self, network, round_nb=None):
    """This method is called before every round. It only redirects to
    protected methods."""
    if round_nb == 0:
      self._initial_setup(network)
    else:
      self._setup_phase(network)

  def _initial_setup(self, network):
    """By default, this codes only calls _setup_phase."""
    self._setup_phase(network)

  def _setup_phase(self, network):
    """Should set next hop and cluster heads for all clusters."""
    pass

  def broadcast(self, network):
    network.broadcast_next_hop()

