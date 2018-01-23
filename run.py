# -*- coding: utf-8 -*-
import logging, sys
import inspect

import config as cf
from python.network.network import Network
from python.utils.tracer import *
from python.routing.direct_communication import *
from python.routing.mte import *
from python.routing.leach import *
from python.routing.fcm import *
from python.network.aggregation_model import *

logging.basicConfig(stream=sys.stderr, level=logging.INFO)

def run_scenarios():
  network  = Network()

  traces = {}
  remaining_energies = []
  average_energies = []
  scenario_names = {}

  for scenario in cf.scenarios:
    if type(scenario) is str:
      exec(scenario)
      continue

    network.reset()

    routing_topology, optimization, aggregation, nickname = scenario

    if nickname:
      scenario_name = nickname
    else:
      if optimization:
        scenario_name = routing_topology+' + '+optimization
      else:
        scenario_name = routing_topology

    if scenario_name in scenario_names:
      scenario_names[scenario_name] += 1
      scenario_name += " (" + str(scenario_names[scenario_name]) + ")"
    else:
      scenario_names[scenario_name] = 1

    routing_protocol_class          = eval(routing_topology)
    network.routing_protocol        = routing_protocol_class()
    if optimization:
      sleep_scheduler_class         = eval(optimization)
      not_class_msg = 'optimization does not hold the name of a class'
      assert inspect.isclass(sleep_scheduler_class), not_class_msg
      network.sleep_scheduler_class = sleep_scheduler_class

    aggregation_function = aggregation + '_cost_aggregation'
    network.set_aggregation_function(eval(aggregation_function))

    logging.info(scenario_name + ': running scenario...')
    traces[scenario_name] = network.simulate()

    remaining_energies.append(600.0 - network.get_remaining_energy())
    average_energies.append(network.get_average_energy())

  if cf.TRACE_COVERAGE:
    print_coverage_info(traces)

  print('Remaining energies: ')
  print(remaining_energies)
  print('Average energies: ')
  print(average_energies)
  return remaining_energies, average_energies

def run_parameter_sweep():
  
  totals = {}
  avgs = {}

  for network_width in [400, 360, 320, 280, 240, 200, 160, 120, 80, 40]:
    totals[network_width] = {}
    avgs[network_width] = {}
    for elec_energy in [100e-9, 80e-9, 60e-9, 40e-9, 20e-9]:
      cf.AREA_WIDTH = network_width
      cf.AREA_LENGTH = network_width
      cf.BS_POS_X = network_width/2
      cf.BS_POS_Y = network_width/2
      cf.E_ELEC = elec_energy

      remaining_energies, average_energies = run_scenarios()
      totals[network_width][elec_energy] = remaining_energies
      avgs[network_width][elec_energy] = average_energies

  print(totals)
  print(avgs)  

if __name__ == '__main__':
  #run_parameter_sweep()
  run_scenarios()

