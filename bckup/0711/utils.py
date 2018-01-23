import matplotlib.pyplot as plt
import math
from config import *
import pandas as pd
import os
import time
from numpy import linspace, meshgrid
from matplotlib.mlab import griddata

RESULTS_PATH = './results/'

def calculate_nb_clusters(avg_distance_to_BS):
  """Calculate the optimal number of clusters for FCM."""
  term1 = math.sqrt(NB_NODES)/(math.sqrt(2*math.pi))
  term2 = THRESHOLD_DIST
  term3 = AREA_WIDTH/(avg_distance_to_BS**2)
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

def plot_traces(traces_dict):
  nb_columns = len(traces_dict.itervalues().next())
  fig, ax = plt.subplots(nrows=1, ncols=nb_columns)

  colors = ['b-', 'r-', 'k-', 'y-', 'g-', 'c-', 'm-']

  color_idx = 0
  for scenario, tracer in traces_dict.iteritems():
    subplot_idx = 1
    for trace_name, trace in tracer.iteritems():
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

def plot_clusters(nodes):
  colors = ['b', 'r', 'k', 'y', 'g', 'm', 'c']
  for cluster_id in range(0, LEACH_NB_CLUSTERS):
    cluster = nodes.get_nodes_by_membership(cluster_id)
    X = [node.pos_x for node in cluster]
    Y = [node.pos_y for node in cluster]
    plt.scatter(X, Y, color=colors[cluster_id%len(colors)])
  plt.show()

def plot_time_of_death(x, y, z):
  """Plot time of death as a colormap."""
  X, Y, Z = grid(x, y, z)
  c = plt.contourf(X, Y, Z)
  cbar = plt.colorbar(c)
  cbar.ax.set_ylabel('number of rounds until death')
  plt.show ()

def log_curves(curves):
  """Write results."""
  dir_path = RESULTS_PATH + time.strftime("%H%M_%d-%m-%Y") + '/'
  os.makedirs(dir_path)
  
  # write alive_nodes vs round number
  df = pd.DataFrame.from_dict(curves)
  df.to_csv(dir_path + 'alive_nodes.txt')

  # write nodes position and time of death

def grid(x, y, z, resX=100, resY=100):
    "Convert 3 column data to matplotlib grid"
    xi = linspace(min(x), max(x), resX)
    yi = linspace(min(y), max(y), resY)
    Z = griddata(x, y, z, xi, yi, interp='linear')
    X, Y = meshgrid(xi, yi)
    return X, Y, Z

