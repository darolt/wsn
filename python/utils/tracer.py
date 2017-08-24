import config as cf

"""Utility class used to store local traces."""
class Tracer(dict):
  def __init__(self):
    rounds_label           = 'Rounds'

    # every tuple has a y-axis label, x-axis label, list with values,
    # boolean that indicates if it is plotable and if is printable

    # lifetime/energy-related log
    self['alive_nodes']     = ('Number of alive nodes', rounds_label, [], 1, 0)
    if cf.TRACE_ENERGY:
      self['energies']        = ('Energy (J)'           , rounds_label, [], 1, 0)

    self['first_depletion'] = ('First depletion'       , rounds_label, [], 0, 0)
    self['30per_depletion'] = ('30 percent depletion'  , rounds_label, [], 0, 0)

    # coverage-related log
    self['coverage']        = ('Coverate rate'        , rounds_label, [], 0, 1)
    self['overlapping']     = ('Overlapping rate'     , rounds_label, [], 0, 1)
    self['nb_sleeping']     = ('% of sleeping nodes'  , rounds_label, [], 0, 1)

    # learning-related log
    self['initial_fitness'] = ('Initial learning'     , rounds_label, [], 0, 1)
    self['final_fitness']   = ('Final learning'       , rounds_label, [], 0, 1)

    self['term1_initial']   = ('term1 learning'       , rounds_label, [], 0, 1)
    self['term2_initial']   = ('term2 learning'       , rounds_label, [], 0, 1)
    self['term1_final']     = ('term1 final'          , rounds_label, [], 0, 1)
    self['term2_final']     = ('term2 final'          , rounds_label, [], 0, 1)

