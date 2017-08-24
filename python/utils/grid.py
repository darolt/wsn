import numpy as np
import logging

import config as cf
from python.utils.utils import *
from python.utils.region import *

def _adjust2grid(pos):
  """Reallocate the x or y value to the grid.
  Ex.: 5.454545 -> 5.45 (if GRID_PRECISION == 0.01)
  """
  return cf.GRID_PRECISION*int(pos/cf.GRID_PRECISION)

"""This module classes are used to calculate the network coverage area
and the network overlapping area. They are optimized in order to speed
up simulation time and therefore coding simplicity is sometimes compro-
mised.
"""

class Grid(object):
  """This class is used to calculate the network coverage area and the
  network overlapping area. This is done by using a pixel grid where
  each node coverage area is 'painted' on the grid. Each pixel is an
  infinitesimal point on the map. The area calculated by this method is
  an approximation that depends on the size of the grid. Therefore the 
  accuracy is configurable.
  """

  def __init__(self):
    # pixels is a dictionary that follows the pattern:
    # {pos_x0: {pos_y0: [node_id0, node_id1], pos_y1: []}, pos_x1:}
    # only painted pixels are added to grid to save memory
    self._pixels = {}

    # for nodes that have no neighbors we just store the area
    # it supposes that the number of neighbors attribute (Node) was
    # already calculated
    self._exclusive_regions = []

  def _paint_pixel(self, x, y, id):
    """Paint pixel if not painted yet, or add node id to painted node.
    """
    if x not in self._pixels: # add line if it does not exist
      self._pixels[x] = {}

    if y in self._pixels[x]:
      owners = self._pixels[x][y]
      # pixel is already painted. Annotate node id to it.
      owners.append(id)
      logging.debug("overlapping pixel %s %s" %(x, y))
    else:
      # paint new pixel
      self._pixels[x][y] = [id]
      logging.debug("painting pixel %s %s" %(x, y))

  def add_node(self, node, coverage_radius):
    """Paint the node on the grid. Assumes a circular radius. It is
    optimized to skip exclusive regions (i.e. regions that are covered
    by a single node.
    """
    logging.info("adding node %d to grid" % (node.id))
    # covers a rectangular area around the circle, but paints only area
    # inside the radius
    initial_x = _adjust2grid(node.pos_x - coverage_radius)
    initial_y = _adjust2grid(node.pos_y - coverage_radius)
    final_x   = _adjust2grid(node.pos_x + coverage_radius)
    final_y   = _adjust2grid(node.pos_y + coverage_radius)
    if initial_x < 0.0: 
      initial_x = 0.0
    if initial_y < 0.0:
      initial_y = 0.0
    if final_x > _adjust2grid(cf.AREA_WIDTH):
      final_x   = _adjust2grid(cf.AREA_WIDTH)
    if final_y > _adjust2grid(cf.AREA_LENGTH):
      final_y   = _adjust2grid(cf.AREA_LENGTH)

    for pixel_x in np.arange(initial_x, final_x, cf.GRID_PRECISION):
      for pixel_y in np.arange(initial_y, final_y, cf.GRID_PRECISION):
        distance = calculate_distance_point(pixel_x, pixel_y,
                                            node.pos_x, node.pos_y)
        if distance < coverage_radius:
          self._paint_pixel(str(_adjust2grid(pixel_x)),
                            str(_adjust2grid(pixel_y)), 
                            node.id)

