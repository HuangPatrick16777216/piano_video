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
    "PropertyGroup",
    "DataGroup",
)

from typing import List, TYPE_CHECKING, Any, Dict, Union
from .props import BoolProp, Property

Video = None
if TYPE_CHECKING:
    from pvkernel import Video


class PropertyGroup:
    """
    A collection of Properties.

    When creating your own PropertyGroup, you will inherit a class from
    this base class. Then, define:
    
    * ``idname``: The unique idname of this property group.
    * properties: Define each property as a static attribute (shown below).

    .. code-block:: py

        class MyProps(pv.types.PropertyGroup):
            prop1 = pv.props.BoolProp(name="hi")
    """
    idname: str

    def __getattribute__(self, name: str) -> Any:
        attr = object.__getattribute__(self, name)
        if isinstance(attr, Property):
            return attr.value
        else:
            return attr

    def __setattr__(self, name: str, value: Any) -> None:
        self._get_prop(name).value = value

    def _get_prop(self, name: str) -> Property:
        """
        You can use this to bypass ``__getattribute__`` and get the
        actual Property object.
        """
        return object.__getattribute__(self, name)


class DataGroup:
    """
    A group of data pointers.

    When creating your own DataGroup, inherit and define the idname.

    Then, you can run ``video.data_idname.value = x`` or ``video.data_idname.value2``
    to access and set values.

    The values can be any type.
    Value names cannot be ``idname`` or ``items``, as they will overwrite internal variables.
    """
    idname: str
    items: Dict[str, Any]

    def __getattr__(self, name: str) -> Any:
        return object.__getattribute__(self, "items")[name]

    def __setattr__(self, name: str, value: Any) -> None:
        if not hasattr(self, "items"):
            object.__setattr__(self, "items", {})
        object.__getattribute__(self, "items")[name] = value


class Operator:
    """
    A function that is positioned at ``pv.ops.group.idname``.
    It can be displayed in the GUI.

    The return value will always be None.

    To create your own operator, inherit and define:

    * ``group``: Operator group.
    * ``idname``: Unique operator idname.
    * ``label``: The text that will show on the GUI (as a button).
    * ``description``: What this operator does.
    * ``execute(video)``: This will be run when the operator is called.
      The first parameter is the video class (``pvkernel.Video``)
    """
    group: str
    idname: str
    label: str
    description: str

    def __init__(self, video: Video) -> None:
        self._video = video

    def __call__(self) -> None:
        self.execute(self._video)

    def execute(self, video: Video) -> None:
        ...

class OpGroup:
    """
    Internal class.
    Positioned at ``pv.ops.group``
    """
    idname: str
    operators: List[Operator]
    video: Video

    def __init__(self, idname: str, video: Video) -> None:
        self.idname = idname
        self.operators = []
        self.video = video

    def __getattr__(self, name: str) -> Operator:
        from .utils import get
        return get(self.operators, name)
