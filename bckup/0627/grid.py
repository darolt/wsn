from utils import *
from config import *
import numpy as np
import logging

"""This module classes are used to calculate the network coverage area
and the network overlapping area. They are optimized in order to speed
up simulation time and therefore coding simplicity is sometimes compro-
mised.
"""

class Region():
  """A region represents all regions that have the same owners (are co-
  vered by the same nodes. Therefore it may represent disjoint regions
  as a single region.
  """
  def __init__(self, area, owners=set()):
    # total area of the region
    self.area   = area
    # nodes that cover this region
    self.owners = owners

class Regions(list):
  """A list of regions"""

  _area_single_pixel = GRID_PRECISION**2

  def __init__(self, grid):
    logging.info('Creating Regions instance.')
    self.extend(grid._exclusive_regions)
    self._grid2regions(grid._pixels)

    self.initial_coverage    = self.get_network_coverage()
    self.initial_overlapping = self.get_network_overlapping()

  def _grid2regions(self, pixels):
    """Convert a grid to regions."""
    logging.info('converting grid to regions.')
    for x, line in pixels.iteritems():
      for y, pixel in line.iteritems():
        owners = set(pixels[x][y])
        region = self._get_region(owners)
        if region:
          # increase area
          region.area += self._area_single_pixel
        else:
          # create region
          new_region = Region(self._area_single_pixel, owners)
          self.append(new_region)

    print(self)


  def _get_region(self, owners):
    """Return region if owners match otherwise return 0.

    Args:
      owners (list): List of node's ids
    """
    for region in self:
      if owners == region.owners:
        return region

    return 0

  def __str__(self):
    """Print all regions."""
    regions_str = ''
    for region in self:
      regions_str += "%s, %f \n" %(str(region.owners), region.area)
    return regions_str

  def get_network_coverage(self, ignore_nodes = []):
    """Sums the areas from every region that is not owned by the igno-
    red nodes.

    Args:
      ignore_nodes (list): List of node's ids to ignore
    """
    coverage = 0.0
    s = set(ignore_nodes)
    for region in self:
      if len(region.owners - s) > 0:
        coverage += region.area

    logging.debug("network coverage area %f." % (coverage))
    return coverage

  def get_network_overlapping(self, ignore_nodes=[]):
    """Sums the areas from every region that has overlapping, but igno-
    ring some nodes.

    Args:
      ignore_nodes (list): List of node's ids to ignore
    """
    overlapping = 0.0
    s = set(ignore_nodes)
    for region in self:
      if len(region.owners - s) > 1:
        overlapping += region.area

    logging.debug("network overlapping area %f." % (overlapping))
    return overlapping

  def get_both(self, ignore_nodes=[]):
    """Get both network coverage and overlapping areas."""
    coverage = 0.0
    overlapping = 0.0
    s = set(ignore_nodes)
    for region in self:
      if len(region.owners - s) > 0:
        coverage += region.area
        if len(region.owners - s) > 1:
          overlapping += region.area

    return coverage, overlapping

class Grid():
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

  @staticmethod
  def _add_offset(pos):
    """Reallocate the x or y value to the grid.
    Ex.: 5.454545 -> 5.45 (if GRID_PRECISION == 0.01)
    """
    return GRID_PRECISION*int(pos/GRID_PRECISION)
    #return int(pos/GRID_PRECISION)

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
    if node.nb_neighbors == 0:
      new_region = Region(math.pi*coverage_radius**2, set([node.id]))
      self._exclusive_regions.append(new_region)
      return
    else:
      # find nearest neighbor
      shortest_distance = INFINITY
      for neighbor in node.neighbors:
        distance = calculate_distance(node, neighbor)
        if distance < shortest_distance:
          shortest_distance = distance
      # create exclusive area
      exclusive_radius = shortest_distance - coverage_radius
      if exclusive_radius < 0:
        exclusive_radius = 0
      new_region = Region(math.pi*exclusive_radius**2, set([node.id]))
      self._exclusive_regions.append(new_region)

    # covers a rectangular area around the circle, but paints only area
    # inside the radius
    # need to offset x and y, otherwise
    initial_x = self._add_offset(node.pos_x - coverage_radius)
    initial_y = self._add_offset(node.pos_y - coverage_radius)
    final_x   = self._add_offset(node.pos_x + coverage_radius)
    final_y   = self._add_offset(node.pos_y + coverage_radius)
    for pixel_x in np.arange(initial_x, final_x, GRID_PRECISION):
      for pixel_y in np.arange(initial_y, final_y, GRID_PRECISION):
        distance = calculate_distance_point(pixel_x, pixel_y,
                                            node.pos_x, node.pos_y)
        if distance < coverage_radius and distance > exclusive_radius:
          # paint
          self._paint_pixel(str(pixel_x), str(pixel_y), node.id)


