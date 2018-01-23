"""Utility class used to store local traces."""
class Tracer(dict):
  alive_nodes_label = 'Number of alive nodes'
  rounds_label      = 'Rounds'
  energies_label    = 'Energy (J)'

  def __init__(self):
    self['alive_nodes'] = (self.alive_nodes_label, self.rounds_label, [])
#    self.energies    = (energies_label, rounds_label, [])
