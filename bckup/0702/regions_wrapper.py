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

  def __str__(self):
    to_print = ""
    for owner in self.owners:
      to_print += " " + str(owner)
    return to_print + " " + str(self.area)

class Regions(list):
  """A list of regions"""

  _area_single_pixel = GRID_PRECISION**2
  _exclude_area_if = 0.005 # exclude regions with less area than this

  def __init__(self, grid, cluster):
    logging.info('Creating Regions instance.')
    self._cluster = cluster
    self.extend(grid._exclusive_regions)
    self._grid2regions(grid._pixels)
    self._remove_small_regions()
    self._extract_exclusive_regions()
    logging.info(self)

    self.initial_coverage = self._get_total_coverage()
    init = self.get_both()
    self.initial_coverage    = init[0]
    self.initial_overlapping = init[1]

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

  def _extract_exclusive_regions(self):
    """Separate regions that overlap from regions that have a single
    owner. This improves performance of get_both method.
    """
    self._exclusive_regions = {}
    del_idx = []
    for idx, region in enumerate(self):
      if len(region.owners) == 1:
        owner = list(region.owners)[0]
        self._exclusive_regions[owner] = region.area
        del_idx.append(idx)

    for idx in del_idx[::-1]:
        del self[idx]

  def _remove_small_regions(self):
    """Removing small regions improves performance."""
    logging.info('removing small regions.')
    total_coverage = self._get_total_coverage()
    #print("total coverage %f" %(total_coverage))
    del_idx = []
    for idx, region in enumerate(self):
      percentage = region.area/total_coverage
      if percentage < self._exclude_area_if:
        del_idx.append(idx)

    for idx in del_idx[::-1]:
        del self[idx]

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
    for owner, area in self._exclusive_regions.iteritems():
      regions_str += "%s, %f \n" %(str(owner), area)
    for region in self:
      regions_str += "%s, %f \n" %(str(region.owners), region.area)
    return regions_str

  def _get_total_coverage(self):
    """Sums the areas from every region."""
    coverage = 0.0
    if hasattr(self, '_exclusive_regions'):
      for area in self._exclusive_regions:
        coverage += area
    for region in self:
      coverage += region.area

    return coverage

  def get_both(self, ignore_nodes=[]):
    """Get both network coverage and overlapping areas."""
    coverage = self.initial_coverage
    overlapping = 0.0
    
    for node in ignore_nodes:
      if node.id in self._exclusive_regions:
        coverage -= self._exclusive_regions[node.id]
      
    s = set(ignore_nodes)
    for region in self:
      res = len(region.owners - s)
      if res == 0:
        coverage -= region.area
      elif res > 1:
          overlapping += region.area

    #print("cov %f, overlap %f" %(coverage, overlapping))
    return coverage, overlapping

