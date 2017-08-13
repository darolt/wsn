
"""Utility class used to store local traces."""
class Tracer(dict):
  def __init__(self):
    alive_nodes_label      = 'Number of alive nodes'
    rounds_label           = 'Rounds'
    energies_label         = 'Energy (J)'
    coverage_label         = 'Coverate rate'
    overlapping_label      = 'Overlapping rate'
    nb_sleeping_label      = '% of sleeping nodes'
    initial_learning_label = 'Initial learning'
    final_learning_label   = 'Final learning'

    # every tuple has a y-axis label, x-axis label, list with values,
    # boolean that indicates if it is plotable and if is printable
    self['alive_nodes']     = (alive_nodes_label,      rounds_label, [], 1, 0)
    self['energies']        = (energies_label,         rounds_label, [], 1, 0)

    self['coverage']        = (coverage_label,         rounds_label, [], 0, 1)
    self['overlapping']     = (overlapping_label,      rounds_label, [], 0, 1)
    self['nb_sleeping']     = (nb_sleeping_label,      rounds_label, [], 0, 1)
    self['initial_fitness'] = (initial_learning_label, rounds_label, [], 0, 1)
    self['final_fitness']   = (final_learning_label,   rounds_label, [], 0, 1)

