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

if __name__ == '__main__':
  network  = Network()

  aggregation_function = zero_cost_aggregation
  #aggregation_function = linear_cost_aggregation(0.5)

  traces = {}

  scenario_names = {}
  for scenario in cf.scenarios:
    if type(scenario) is str:
      exec(scenario)
      continue

    routing_topology, optimization, aggregation = scenario

    if optimization:
      scenario_name = routing_topology+' + '+optimization
    else:
      scenario_name = routing_topology

    if scenario_name in scenario_names:
      scenario_names[scenario_name] += 1
    else:
      scenario_names[scenario_name] = 1
    scenario_name += " (" + str(scenario_names[scenario_name]) + ")"

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

    network.reset()

    #log_curves(trace_alive_network)
  if cf.TRACE_COVERAGE:
    print_coverage_info(traces)

  save2csv(traces)
  plot_traces(traces)

