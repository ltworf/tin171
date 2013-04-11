import os

from distutils.core import setup

version = '0.1'

setup(
  name='chinese-checkers',
  version=version,
  description='Python implementation of the chinese checkers game, GUI and AI',
  packages=['chinese-checkers','chinese-checkers.bot'],
  #  scripts=['chinese-checkers/chinese-checkers-gui'],
  license = "GPL3",
  url = "https://github.com/ltworf/tin171",
)
