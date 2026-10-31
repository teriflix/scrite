/****************************************************************************
**
** Copyright (C) 2020 Prashanth N Udupa
** Author: Prashanth N Udupa (prashanth@scrite.io,
**                            prashanth.udupa@gmail.com,
**                            prashanth@vcreatelogic.com)
**
** This code is distributed under GPL v3. Complete text of the license
** can be found here: https://www.gnu.org/licenses/gpl-3.0.txt
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

import QtQml
import QtQuick
import QtQuick.Controls

import Scrite.App

import "../controls"

/**
  A menu that shows everything that a group has: the actions of its action manager, its entries,
  what its delegate makes, and the groups that are merged into it. The menu keeps up with the
  group. It is how submenus of hosted menus, menu buttons on toolbars, and menus in the menu bar
  on Linux are made.

  Nested menus are made by SctMenu, from this same file, when it needs them.
  */
SctMenu {
    id: root

    // The group is what this menu shows, and it is given when the menu is made.
    required group

    title: group ? group.title : ""
}
