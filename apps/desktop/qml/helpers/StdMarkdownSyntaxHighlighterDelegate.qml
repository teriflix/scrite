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

import QtQuick

import Scrite.App

import "../globals"

MarkdownSyntaxHighlighterDelegate {
    heading1Color: Runtime.colors.tx("#1d4ed8")
    heading2Color: Runtime.colors.tx("#7c3aed")
    heading3Color: Runtime.colors.tx("#be185d")
    heading4Color: Runtime.colors.tx("#c2410c")
    heading5Color: Runtime.colors.tx("#a16207")
    heading6Color: Runtime.colors.tx("#4d7c0f")
    emphasisColor: Runtime.colors.tx("#0369a1")
    codeColor: Runtime.colors.tx("#9a3412")
    linkColor: Runtime.colors.tx("#2563eb")
    markerColor: Runtime.colors.tx("#6b7280")
    quoteColor: Runtime.colors.tx("#6b7280")
}