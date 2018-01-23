from config import *
import logging

class EnergySource(object):
  def __init__(self, parent):
    self.energy = INITIAL_ENERGY
    self.node = parent

  def recharge(self):
    self.energy = INITIAL_ENERGY

class Battery(EnergySource):
  def consume(self, energy):
    if self.energy >= energy:
      self.energy -= energy
    else:
      logging.info("node %d: battery is depleted." % (self.node.id))
      self.energy = 0

      # side-effects, coding this here is not clear
      self.node.alive = 0
      self.node.sleep_prob = 0.0

class PluggedIn(EnergySource):
  def consume(self, energy):
    pass

