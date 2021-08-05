#
#  Piano Video
#  A free piano visualizer.
#  Copyright Patrick Huang 2021
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

__all__ = (
    "Video",
)

import pv
from pv.types import PropertyGroup
from typing import Tuple, Type


class Video:
    """
    This class holds all the settings of a video.
    It is also used like a "context" for all rendering functions.
    """
    resolution: Tuple[int, int]
    fps: float

    def __init__(self, resolution: Tuple[int, int] = (1920, 1080), fps: float = 30.) -> None:
        """
        Initializes the Video.

        :param resolution: (X, Y) pixel resolution.
        :param fps: Frames per second. Can be float, but will be rounded down in export.
        :return: None
        """
        self.resolution = resolution
        self.fps = fps

        self._props = []   # List of PropertyGroups.

        pv.utils.add_callback(self._add_pgroup, ("pgroup",))
        for cls in pv.utils._get_pgroups():
            self._add_pgroup(cls)

    def __getattr__(self, name: str) -> PropertyGroup:
        return pv.utils.get(self._props, name)

    def _add_pgroup(self, cls: Type[PropertyGroup]) -> None:
        """
        Callback function to add PropertyGroup props to internal list.
        """
        self._props.append(cls())
