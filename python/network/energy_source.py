import config as cf
import logging

class EnergySource(object):
  def __init__(self, parent):
    self.energy = cf.INITIAL_ENERGY
    self.node = parent

  def recharge(self):
    self.energy = cf.INITIAL_ENERGY

class Battery(EnergySource):
  def consume(self, energy):
    if self.energy >= energy:
      self.energy -= energy
    else:
      logging.info("node %d: battery is depleted." % (self.node.id))
      self.energy = 0

      self.node.battery_depletion()

class PluggedIn(EnergySource):
  def consume(self, energy):
    pass

