# -*- coding: utf-8 -*-
import math
import logging, sys
from config import *
from nodes import Nodes
from direct_communication import *
from mte import *
from fcm import *
from leach import *
from tqdm import tqdm
from tracer import *

logging.basicConfig(stream=sys.stderr, level=logging.INFO)

"""Aggregation cost functions. Determine the cost of cluster heads for-
   warding messages.
"""
def zero_cost_aggregation(msg_length):
  return 0

def total_cost_aggregation(msg_length):
  return msg_length

def linear_cost_aggregation(factor):
  """Defines a family of functions."""
  return lambda x: int(x*factor)

def log_cost_aggregation(msg_length):
  return int(math.log(msg_length))

def default_init(nodes):
  # For most scenarios, nodes notify their positions to the BS.
  nodes.notify_position()

def run_scenario(scenario_name, nodes, round, initialization=default_init):
  """Wrapper for running other scenarios. Do the initialization,
  logging and finalization.
  """
  logging.info(scenario_name + ': running scenario...')
  local_traces = Tracer()
  if initialization:
    initialization(nodes)
  ret = 0
  for round_nb in range(0, MAX_ROUNDS):
    print_args = (round_nb, nodes.get_remaining_energy())
    print("round %d: total remaining energy: %f" % print_args)
    if not nodes.someone_alive():
      break
    local_traces['alive_nodes'][2].append(nodes.count_alive_nodes())
    #local_traces['energies'][2].append(nodes.get_remaining_energy())
    ret = round(nodes, round_nb, local_traces, ret)

  return local_traces

def do_nothing(value):
  pass

if __name__ == '__main__':
  nodes  = Nodes()
  # ex. traces: {'DC' : {'alive_nodes': [], 'energies': []},
  #              'MTE': {}...}

  aggregation_function = zero_cost_aggregation
  #aggregation_function = linear_cost_aggregation(0.5)

  traces = {}
  if RUN_DC:
    nodes.set_aggregation_function(aggregation_function)
    traces['DC'] = run_scenario('DC', nodes, DC_round, DC_init)
    nodes.plot_time_of_death()
    nodes.reset()
  if RUN_MTE:
    # in the FCM paper, authors suppose that a forwarded message
    # in MTE is entirely sent to the next hop, meaning that there
    # is no aggregation/compression
    nodes.set_aggregation_function(aggregation_function)
    traces['MTE'] = run_scenario('MTE', nodes, MTE_round, do_nothing)
    nodes.plot_time_of_death()
    nodes.reset()
  if RUN_LEACH:
    nodes.set_aggregation_function(aggregation_function)
    traces['LEACH'] = run_scenario('LEACH', nodes, LEACH_round, do_nothing)
    nodes.plot_time_of_death()
    nodes.reset()
  if RUN_FCM:
    nodes.set_perform_two_level_comm(1)
    nodes.set_aggregation_function(aggregation_function)
    traces['FCM'] = run_scenario('FCM', nodes, FCM_round, do_nothing)
    nodes.plot_time_of_death()
    nodes.reset()
  if RUN_PSO:
    nodes.set_perform_two_level_comm(1)
    nodes.set_aggregation_function(aggregation_function)
    traces['FCM+PSO'] = run_scenario('FCM+PSO', nodes, FCM_PSO_round, do_nothing)
    nodes.plot_time_of_death()
    nodes.reset()
  if RUN_FCM_MTE:
    nodes.set_perform_two_level_comm(0)
    nodes.set_aggregation_function(aggregation_function)
    traces['FCM+MTE'] = run_scenario('FCM+MTE', nodes, FCM_MTE_round, do_nothing)
    nodes.plot_time_of_death()
    nodes.reset()

  #log_curves(trace_alive_nodes)
  plot_traces(traces)

