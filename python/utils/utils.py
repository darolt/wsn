import matplotlib.pyplot as plt
import math
import pandas as pd
import numpy as np
import os
import time
from numpy import linspace, meshgrid
from matplotlib.mlab import griddata

import config as cf
from python.network.network import *

RESULTS_PATH = './results/'

plt.rcParams.update({'font.size': 14})

def calculate_nb_clusters(avg_distance_to_BS):
  """Calculate the optimal number of clusters for FCM."""
  term1 = math.sqrt(cf.NB_NODES)/(math.sqrt(2*math.pi))
  term2 = cf.THRESHOLD_DIST
  term3 = cf.AREA_WIDTH/(avg_distance_to_BS**2)
  return int(term1*term2*term3)

def calculate_distance(node1, node2):
  """Calculate the Euclidean distance between two nodes."""
  x1 = node1.pos_x
  y1 = node1.pos_y
  x2 = node2.pos_x
  y2 = node2.pos_y
  return calculate_distance_point(x1, y1, x2, y2)

def calculate_distance_point(x1, y1, x2, y2):
  """Calculate the Euclidean distance between two points."""
  return math.sqrt((x1 - x2)**2 + (y1 - y2)**2)

def print_positions(nodes):
  # check positions
  for node in nodes:
    print("%d %d" %(node.pos_x, node.pos_y))

def plot_curves(curves):
  """Generic plotter of curves."""
  assert len(curves) <= 7, "More plots (%d) than colors." %len(curves)

  colors = ['b-', 'r-', 'k-', 'y-', 'g-', 'c-', 'm-']
  color_idx = 0
  for scenario, curve in curves.iteritems():
    X = range(0, len(curve))
    plt.plot(X, curve, colors[color_idx], label=scenario)
    color_idx += 1

def print_coverage_info(traces):
  for scenario_name, tracer in traces.iteritems():
    for trace_name, trace in tracer.iteritems():
      if not trace[4]:
        continue
      values  = np.array(trace[2])
      mean    = np.nanmean(values)
      stdev   = np.nanstd(values)
      args = (scenario_name, trace_name, mean, stdev)
      print("%s: %s avg (std): %f (%f)" % args)

def plot_traces(traces):
  first_tracer = traces.itervalues().next()
  nb_columns   = len([1 for k, v in first_tracer.iteritems() if v[3]])
  fig, ax      = plt.subplots(nrows=1, ncols=nb_columns)

  colors = ['b-', 'r-', 'k-', 'y-', 'g-', 'c-', 'm-']

  color_idx = 0
  for scenario, tracer in traces.iteritems():
    subplot_idx = 1
    for trace_name, trace in tracer.iteritems():
      if not trace[3]:
        continue
      ax = plt.subplot(1, nb_columns, subplot_idx)
      #ax.set_title(trace_name)
      X = range(0, len(trace[2]))
      plt.plot(X, trace[2], colors[color_idx%len(colors)], label=scenario)
      plt.xlabel(trace[1])
      plt.ylabel(trace[0])
      plt.legend()
      subplot_idx += 1
    color_idx += 1
      
  plt.show()

def plot_nodes_plane(nodes):
  X_ch = [node.pos_x for node in nodes if node.is_head()]
  Y_ch = [node.pos_y for node in nodes if node.is_head()]
  X_or = [node.pos_x for node in nodes if node.is_ordinary()]
  Y_or = [node.pos_y for node in nodes if node.is_ordinary()]
  X_de = [node.pos_x for node in nodes if not node.alive]
  Y_de = [node.pos_y for node in nodes if not node.alive]

  plt.scatter(X_ch, Y_ch, color='b')
  plt.scatter(X_or, Y_or, color='r')
  plt.scatter(X_de, Y_de, color='k')
  plt.show()

def plot_clusters(network):
  colors = ['b', 'k', 'y', 'g', 'm', 'c']
  # print clusters
  plt.figure()
  for cluster_id in range(0, cf.NB_CLUSTERS):
    cluster = network.get_nodes_by_membership(cluster_id)
    X = [node.pos_x for node in cluster if not node.is_head()]
    Y = [node.pos_y for node in cluster if not node.is_head()]
    color_ref = float(cluster_id)/cf.NB_CLUSTERS*0.6
    plt.scatter(X, Y, color=colors[cluster_id%len(colors)])

  x_border = [0.0 for y in range(0, int(cf.AREA_LENGTH))]
  y_border = [y   for y in range(0, int(cf.AREA_LENGTH))]
  x_border.extend([cf.AREA_WIDTH  for y in range(0, int(cf.AREA_LENGTH))])
  y_border.extend([y              for y in range(0, int(cf.AREA_LENGTH))])
  x_border.extend([x              for x in range(0, int(cf.AREA_WIDTH))])
  y_border.extend([0.0            for x in range(0, int(cf.AREA_WIDTH))])
  x_border.extend([x              for x in range(0, int(cf.AREA_WIDTH))])
  y_border.extend([cf.AREA_LENGTH for x in range(0, int(cf.AREA_WIDTH))])
  z_border = [0 for x in range(0, int(2*cf.AREA_LENGTH + 2*cf.AREA_WIDTH))]
  for cluster_id in range(0, cf.NB_CLUSTERS):
    X = [node.pos_x for node in network[0:-1]]
    Y = [node.pos_y for node in network[0:-1]]
    Z = [1 if node.membership==cluster_id else 0 for node in network[0:-1]]
    X, Y, Z = grid(X, Y, Z)
    plt.contour(X, Y, Z, 1, colors='0.5')

  # print heads
  heads = network.get_heads()
  X = [node.pos_x for node in heads]
  Y = [node.pos_y for node in heads]
  plt.scatter(X, Y, color='r', marker='^', s=80)
  plt.xlim(xmin=0)
  plt.ylim(ymin=0)
  plt.xlim(xmax=cf.AREA_WIDTH)
  plt.ylim(ymax=cf.AREA_LENGTH)
  plt.show()

def plot_time_of_death(network):
  """Plot time of death as a colormap."""
  x = [node.pos_x for node in network[0:-1]]
  y = [node.pos_y for node in network[0:-1]]
  z = [node.time_of_death for node in network[0:-1]]

  X, Y, Z = grid(x, y, z)
  c = plt.contourf(X, Y, Z)
  cbar = plt.colorbar(c)
  cbar.ax.set_ylabel('number of rounds until death')
  plt.show()

def log_curves(curves):
  """Write results."""
  dir_path = RESULTS_PATH + time.strftime("%H:%M:%S_%d-%m-%Y") + '/'
  os.makedirs(dir_path)
  
  # write alive_nodes vs round number
  df = pd.DataFrame.from_dict(curves)
  df.to_csv(dir_path + 'alive_nodes.txt')

  # write nodes position and time of death

def log_coverages(pso_wrapper):
  dir_path = RESULTS_PATH + time.strftime("%H:%M:%S_%d-%m-%Y") + '/'
  os.makedirs(dir_path)
  df = pd.DataFrame.from_dict(pso_wrapper._cov_log)
  df.to_csv(dir_path + 'cov_log.txt')

def grid(x, y, z, resX=100, resY=100):
    "Convert 3 column data to matplotlib grid"
    xi = linspace(min(x), max(x), resX)
    yi = linspace(min(y), max(y), resY)
    Z = griddata(x, y, z, xi, yi, interp='linear')
    X, Y = meshgrid(xi, yi)
    return X, Y, Z

