import logging
import numpy as np

import config as cf
from python.utils.utils import *
from python.utils.region import *

"""This module classes are used to calculate the network coverage area
and the network overlapping area. They are optimized in order to speed
up simulation time and therefore coding simplicity is sometimes compro-
mised.
"""

class RegionsConverter(list):
  """Helps to convert a grid to regions."""

  _area_single_pixel = cf.GRID_PRECISION**2
  _exclude_area_if = 0.005 # exclude regions with less area than this

  def __init__(self, grid):
    logging.info('Creating Regions instance.')
    self.extend(grid._exclusive_regions)
    self._grid2regions(grid._pixels)
    #self._remove_small_regions()
    self._extract_exclusive_regions()
    logging.info(self)

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
    owner. This aims to improve performance.
    """
    logging.info('extracting exclusive regions.')
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
    sum = 0.0
    regions_str = ''
    for owner, area in self._exclusive_regions.iteritems():
      regions_str += "%s, %f \n" %(str(owner), area)
      sum += area
    for region in self:
      regions_str += "%s, %f \n" %(str(region.owners), region.area)
      sum += region.area
    regions_str += "total area: %f\n" %(sum)
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

  def _check(self, exclusive, overlapping):
    for owner, area in exclusive.iteritems():
      assert area >= 0.0, "Negative region found!"
    for region in self:
      assert region.area >= 0.0, "Negative region found!"

  def convert(self):
    overlapping_regions = []
    for region in self:
      overlapping_regions.append((list(region.owners), region.area))    

    self._check(self._exclusive_regions, overlapping_regions)
    return self._exclusive_regions, overlapping_regions

