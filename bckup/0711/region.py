
class Region(object):
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

