# User Interface
Scrite's interface is primarily a tab-view. It consists of three tabs

- Screenplay Tab (`Alt+1`)
- Structure Tab (`Alt+2`)
- Notebook Tab (`Alt+3`)

Each tab offers a perspective and mode of interaction with your screenplay.
Irrespective of which tab you choose to work with, they all help you work on a
single screenplay in the document that is currently open.

You can switch between these tabs by clicking on the tab buttons to the top
right of the window, or by using the keyboard shortcuts as listed above.

## Screenplay Tab
The screenplay tab offers a scene-after-scene view of your screenplay, as you
might have already seen in the chapters before. 

<img src="../images/quickstart/012-second-scene.png" width="100%"/>

Each scene is a distinct block of text. Infact, if you toggle the Scene Blocks
option ON ...

<img src="../images/screenplay-tab/001-scene-blocks.jpg" width="85%"/>

... the Screenplay Editor shows each scene as a distinct block.

<img src="../images/screenplay-tab/002-scene-blocks.jpg" width="85%"/>

Scrite's document model stores each scene as a distinct object, and within each
scene it stores each paragraph as a distinct object with a unique ID. This
allows for rapid manipulation of the document by various internal modules, and
by extensions and plugins we plan to feature in future updates.

Each scene has a scene heading, followed by scene content. The UI also shows
scene number and a page number bubble at approximately the place within the
scene where the page break is likely to land.

<img src="../images/screenplay-tab/003-parts-of-a-scene.jpg" width="100%"/>

As [mentioned earlier](index.md#pagecentric), Scrite is structure-first and
scene-centric. Even so, it updates the page count and current page as you type.

It does this by building an internal, paginated document from all scenes in your
screenplay. This work happens in a background thread to keep the UI smooth and
responsive.

Note that this internal pagination is approximate. It does not apply certain
page layout rules—e.g., a dialogue block that spans pages should be split with
MORE and CONT'D markers. The generated PDF does apply these rules, so the page
markers you see in the editor may not match the final PDF boundaries.

Treat the editor’s page markers as a guide to scene length, not as exact
pagination.

> **NOTE**: Scrite does not position itself as a page centric editor, and as
> such does not offer page layouting on par with other screenwriting apps.

Further reading: [Screenplay Tab](screenplay.md)

## Structure Tab
When Scrite began, this was the only tab. It remains central to how we think
about screenplays. At its core, the tab has three parts:

<img src="../images/structure-tab/001-parts-of-structure-tab.jpg" width="100%"/>

- The structure canvas lets you shape your story by placing index cards on a
  wide, open canvas.
- You can pull scenes from the canvas to the timeline and sequence them into a
  narrative.
- You can edit the sequenced scenes in the screenplay editor area.

This tab is inspired by popular video editing apps: the canvas is for gathering
and shaping material, the timeline is for sequencing, and the screenplay editor
is for detailed writing. This way you can simultaneously view the big picture on
the canvas, the details in the editor, and the sequence and pacing on the
timeline. Together, they give you a comprehensive view of your screenplay.

Further reading: [Structure Tab](structure.md)

## Notebook Tab
Some writers like to start writing scenes right away. Others prefer to plot the
story on a canvas or whiteboard—in which case the Structure tab, described
earlier, is ideal.

<img src="../images/notebook-tab/001-story-structure.jpg" width="100%"/>

Many writers begin by capturing notes and research: story ideas, character
details, relationships, and a breakdown of acts, key scenes, or story beats.

Others write the screenplay first—at least a first draft—and add notes to scenes
and characters later.

In both cases, the Notebook provides a comprehensive set of tools for capturing
notes about scenes, characters, and the story.

<img src="../images/notebook-tab/002-notebook.jpg" width="100%"/>

By default, the Notebook has its own tab. You can also stack it inside the
Structure tab to reference notes while writing.

<img src="../images/notebook-tab/003-notebook-within-structure.jpg"
width="85%"/>

Toggle the “Stack Structure & Notebook” option in Settings to view both in a
single tab and automatically pull up scene or character notes as you type.

<img src="../images/notebook-tab/004-notebook-within-structure.jpg"
width="100%"/>

Further reading: [Notebook Tab](notebook.md)

## Keyboard Shortcuts
Most menu items, toolbars, and buttons in Scrite have keyboard shortcuts. Hover
over any control to see its shortcut.

<img src="../images/ui/001-shortcut-in-tooltip.jpg" width="65%"/>

You can also open the Shortcuts dialog by using `Ctrl+E` on Windows & Linux,
`Cmd+E` on macOS to view all shortcuts and customize them.

<img src="../images/ui/002-shortcuts-dialog.jpg" width="85%"/>

Example: You can launch the Settings dialog by using `Ctrl+,` on Windows and
Linux, and `Cmd+,` on macOS.

<img src="../images/ui/003-settings-dialog.jpg" width="85%"/>

Some actions don’t have built-in shortcuts, but you can assign them. For
example, you can set `Ctrl+Alt+T` for the Statistics Report. If a shortcut you
assign clashes with another, then Scrite show you a message and lets you
configure another shortcut.

<img src="../images/ui/004-customising-shortcuts.jpg" width="85%"/>

From then on, that shortcut opens the Statistics Report dialog—configure options
and press `Enter` to generate.

<img src="../images/ui/005-customising-shortcuts.jpg" width="85%"/>

By clicking on the Revert icon next to the shortcut, you can revert to default.

It’s that simple. Explore existing shortcuts and customize them as you like.

> NOTE: Shortcuts for switching between languages can only be configured in the
> [Language Settings](./languages.md#keyboard-shortcuts) dialog box.

## Command Center

Scrite has hundreds of actions, and it’s hard to remember every shortcut. The
Command Center helps: press `Ctrl+/` or `Cmd+/` to open it.

<img src="../images/ui/006-command-center.jpg" width="100%"/>

Type a few letters of the command you need; the Command Center filters and shows
matching actions.

<img src="../images/ui/007-command-center.jpg" width="85%"/>

Use the arrow keys to select an action and press `Enter`. That’s it.

Optionally, you can click on the Assign link next to command for which there is
no shortcut, and assign a custom shortcut.

The command center also lists templates, scripts from Scriptalay and your
recently open files as well. You can look for them by typing file name, or
author, title, subtitle, or any word in the logline.

<img src="../images/ui/008-command-center.jpg" width="85%"/>

> **NOTE**: Command center only lists actions that are available in the current
> context. So its possible that you won't find one or more commands in here,
> even though they are listed in the shortcuts dialog box.

## Jump to Scene
Working in a large screenplay? Press `Ctrl+G`, type a scene/act/episode, and
press `Enter` to jump.

<img src="../images/ui/009-jump-to-scene.jpg" width="85%"/>

## Scene Navigation
With keyboard focus on the scene content area in any scene within the screenplay
editor, use `Alt+Up`/`Alt+Down` to jump to the previous/next scene. Use
`Ctrl+Alt+Up`/`Ctrl+Alt+Down` to jump to the first/last scene.

Additionally, while typing scene content, you can also:

|Action|Shortcut|
|:-----|:-------|
|Toggle Synopsis|`Ctrl+Alt+S`| 
|Toggle Character & Scene Tags|`Ctrl+Alt+C`| 
|Edit Scene Heading|`Ctrl+0`| 
|Edit Scene Number|`Ctrl+Alt+0`|

You don’t have to memorize these, open the Shortcuts Dialog (`Ctrl+E`/`Cmd+E`)
to discover.

<img src="../images/ui/010-scene-navigation.jpg" width="85%"/>
