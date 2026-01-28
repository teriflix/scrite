# Screenplay Tab

The screenplay shown in screenshots are from the 2019 film Knives Out. The
original screenplay can be downloaded from
[here](https://lionsgate.brightspotcdn.com/fb/14/23cd58a147afbb5c758ecb3dff0a/knivesout-final.pdf)
and all rights for the screenplay rests with its owners. We are using the
screenplay only for explaining features in the product.

<img src="../images/screenplay/001-knives-out.jpg" width="100%"/>

## Capturing The Logline

A logline is a 2–3 line pitch for your screenplay. It often appears as the short
description of your film on IMDb.

If you want to capture the logline of your screenplay in Scrite, you can toggle
the logline editor in the Screenplay Editor Options Dialog.

<img src="../images/screenplay/002-logline.jpg" width=480"/>

Once enabled, Scrite presents a logline field below the title page on the
Screenplay Editor. You can capture the logline in this field.

<img src="../images/screenplay/003-logline.jpg" width=640"/>

As you can see, Scrite highlights the last few words in the logline in red
color. This is because they spill over the recommend character limit for
loglines. While it is recommended that you don't exceed this limit, Scrite does
not prohibit you from capturing longer logline.

## Color Coding Scenes

While plotting, many writers use post‑it notes on a pinboard. Some prefer color
coding; others don’t. If you like color coding scenes, Scrite supports it.

Select one or more scenes on the Scene List Panel, and right-click to view a
menu. From here you can select a color to assign to your selected scenes.

<img src="../images/screenplay/004-scene-colors.jpg" width=640"/>

As you assign colors to different scenes, the Scene List panel uses the same
color coding to display scene headings, as does the scene heading area in the
screenplay editor.

Infact, you can also open the scene options menu in the screenplay editor to
assign scene color for a specific scene.

<img src="../images/screenplay/005-scene-colors.jpg" width=480"/>

## Color Intensity

Some writers prefer to see intense colors for their scenes, and some prefer
subdued colors. You can change the color intensity by clicking on Screenplay
Editor Options Dialog, and dragging to color intensity slider as you see fit.

<img src="../images/screenplay/006-color-intensity.jpg" width=480"/>

## Scene Blocks

As mentioned in the quick start, Scrite offers a scene centric approach to
writing screenplays. Each scene is a distint block of text, even if they show up
line a single long document. If you prefer to see each scene as a distinct
block, then toggle the `Scene Blocks` option either in Screenplay Editor
Options, or by using the [`Command Center`](./user-interface.md#command-center).

<img src="../images/screenplay/006a-scene-blocks.jpg" width=480"/>

When `Scene Blocks` is toggled ON, each scene is shown as a separate block on
the screenplay editor.

<img src="../images/screenplay/006b-scene-blocks.jpg" width=720"/>

## Scene List Panel

You can toggle the visibility of the Scene List panel by clicking on the button
along the left edge of the app window, or by using the keyboard shortcut
`Alt+0`. 

<img src="../images/screenplay/007-scene-list-panel.jpg" width=720"/>

Using the Scene List Panel you can get a quick overview of all scenes in your
screenplay and even jump to a scene.

By default, clicking on a scene in the Scene List panel causes the scene to get
selected. You can select multiple scenes using `Ctrl+Click`.

<img src="../images/screenplay/008-multi-select.png" width=720"/>

Or hold `Shift` to select a range.

<img src="../images/screenplay/009-multi-select.png" width=720"/>

Whether you select one scene or many, you can right‑click to change colors for
all selected scenes.

<img src="../images/screenplay/004-scene-colors.jpg" width=640"/>

### Scene Heading vs Scene Summary

By default the Scene List Panel shows scene heading for each scene in a list
view. You can, however, configure the scene list panel to show scene synopsis or
summary instead.

<img src="../images/screenplay/010-scene-list-panel-summary-text.jpg"
width=640"/>

You can change the number of lines from the synopsis to display in Settings.

<img src="../images/screenplay/011-scene-list-panel-summary-text.jpg"
width=640"/>

### Empty Scene Icon

The Scene List panel shows an “empty‑scene” icon for scenes with only a
scene‑heading but no content. This icon disappears as soon as some content is
typed or pasted into the scene.

<img src="../images/screenplay/012-empty-scene-icon.png" width=360"/>

### Length Estimates

You can toggle display of scene lengths against each scene in the scene list
panel, either in terms of page count or screentime.

<img src="../images/screenplay/013-scene-lengths-in-scene-list-panel.jpg"
width=640"/>

Notice how the scene list panel shows aggregate time for act breaks. This helps
in getting a quick impression of pacing in your screenplay.

### Tooltips

You can enable tooltips and get additional information about each scene as you
hover over them in the scene list panel. Additionally, when you select a bunch
of scenes a separate tooltip shows up along the top with aggregate lengths of
the selected scenes.

<img src="../images/screenplay/014-scene-list-panel-tooltips.jpg" width=640"/>

### Sequences

As a writer you may want to bundle multiple scenes into a sequence, because they
all form a logical group in your story. Simply select a range of scenes and
bundle them together into a sequence.

<img src="../images/screenplay/015-scene-list-panel-sequence.jpg" width=640"/>

Scenes that are part of a sequence have a arrow prior to their scene number in
the scene list panel.

You can easily add/remove scenes to a sequence by using the context menu.
Bundling scenes together like this allows you to capture rich detail about the
way you look at elements of the story unfolding across scenes in the screenplay.

### Story Tracks

Scrite lets you tag keywords and formal story beats (from Save The Cat, or your
own beat sheet) against each scene, or on a bunch of them at once. You can
enable the display of tracks to get a quick overview of story elements along
side the scene headings in the scene list panel.

<img src="../images/screenplay/016-scene-list-panel-tracks.jpg" width=640"/>

Notice how the tracks on the left also shows a bar for each sequence (#1, #2
etc..) in a separate track.

More on formal story beats, and scene keywords in a later section.

## Page Number and Screen Time

Although Scrite isn’t page‑centric, page counts still matter:

- Each page ≈ 1 minute of screen time, so page count helps estimate duration.
- Many structures are page‑centric (e.g., Save The Cat breaks into Act II at
  page 25).

Scrite lets you keep track of current page, total page count, current time and
total time on the status bar. In the editor area, page breaks are shown in a
bubble to the left of the editing area.

<img src="../images/screenplay/017-page-time-number.jpg" width=640"/>

> **NOTE**: When exporting to PDF, page count may change depending on MORE and
> CONT’D usage.

Time values are approximate (1 page ≈ 1 minute). You can adjust 'Time Per Page'
in Settings → Screenplay → Page Setup.

<img src="../images/screenplay/018-time-per-page.jpg" width=640"/>

Scrite automatically recalculates page count and time estimate as you type in a
background thread. You can, however, force a full refersh of these estimates by
clicking on the reload icon in the status bar.

<img src="../images/screenplay/018-2-page-time-number.jpg" width=640"/>

> **NOTE**: Page counts vary across apps. Final Draft, Scrite, and Fade In Pro
may report different counts for the same screenplay; making them identical is
rarely possible. Infact, page count estimates done by Scrite on Windows, macOS
and Linux vary as well.

## Formatting Paragraphs

You can format paragraph types and have that format applied to all paragraphs of
those types. Go to Settings → Screenplay → Formatting Rules to view and edit the
currently applied rules. 

<img src="../images/screenplay/019-formatting.png" width=720"/>

Click any paragraph in the preview and alter properties. 

Close the dialog and Scrite applies your formatting rules across all scenes. 

<img src="../images/screenplay/020-formatting.png" width=720"/>

These formats are used in preview, exported PDFs, and reports.

<img src="../images/screenplay/021-formatting.png" width=720"/>

Scrite does not apply special formatting to fragments of text (bold, italic,
underline, color) within paragraphs via Formatting Rules. Use [Markup
Tools](#markup-tools) for inline formatting.

Formatting options are saved with the file and reused each time you open it,
even if another file used different options. Scrite maintains two sets of
formatting options:

1. Default formatting options used for new files.
2. Document‑specific options used for the current file.

Click “Factory Reset” to discard document‑specific options and use defaults.
Click “Make Default” to set current options as your defaults.

<img src="../images/screenplay/022-formatting.png" width=720"/>

## Page Setup and Watermark

You can configure the page size, and content to show in header & footer of
generated PDFs by opening Settings > Screenplay > Page Setup tab.

<img src="../images/screenplay/053-page-setup.jpg" width="100%">

In the same page you can also configure the watermark to use in generated PDF
files.

### Watermarks are File Specific

Please note that watermarks are file-specific. This means that any change you
make to watermark settings only applies to the file that is currently open. If
you open another file, the watermark settings wont carry over to that file. Said
in other words, watermark settings are saved along with the file and will be
loaded whenever that file is opened.

You could save modified watermark settings as defaults, so that new documents
created from then on will use those settings.

### Impromptu Watermarks

While generating PDF of your screenplay or extracting reports, you can change
the watermark text in the [report generation](./reports.md) dialog box.

> NOTE: The ability to configure or turn off watermark is restricted in certain
> subscription plans. Please lookup plan features for more information.

## Title Page

Editing the title page is described in the [Quick
Start](./quickstart.md#editing-the-title-page) guide. 

## Markup Tools

Markup Tools let you apply inline formatting to specific snippets of text. The
Markup Tools dock is hidden by default; toggle it from the Screenplay Editor
Options menu. You can move the dock to any part of the screen.

<img src="../images/screenplay/023-markup-tools.jpg" width=640"/>

Select any text snippet and use Markup Tools to apply custom formatting. Hover
over buttons to see shortcuts:

- Bold: `Ctrl+B`
- Italics: `Ctrl+I`
- Underline: `Ctrl+U`
- Strikeout: `Ctrl+R`
- ALL CAPS: `Shift+F3`
- small caps: `Ctrl+Shift+F3`

<img src="../images/screenplay/024-markup-tools.jpg" width=640"/>

## Custom Scene Numbers

By default, Scrite generates scene numbers automatically and regenerates them if
you change order or insert scenes. 

<img src="../images/screenplay/025-custom-scene-numbers.png" width=720"/>

You can apply custom numbers (e.g., 1, 1A, 1B) by editing the scene number
field. Automatic numbering resumes for scenes without custom numbers. Custom
numbers appear in preview, exported PDFs, and reports.

<img src="../images/screenplay/026-custom-scene-numbers.png" width=720"/>

## Scene Type

Many writers, specificially Indian writers, prefer to mark a scene as action,
montage or song scene. This helps them with production planning and more.

<img src="../images/screenplay/026a-scene-type.jpg" width=640"/>

## Side Panel

Scrite lets you capture additional information per scene. While the Notebook tab
is best for capturing this information, the Screenplay editor also offers access
to Comments, Featured Image, and Index Card Fields.

<img src="../images/screenplay/027-enable-side-panel.jpg" width=720"/>

### Comments

Check the "Show Comments Panel" in Screenplay Editor Options to show a pullout
for each scene. 

<img src="../images/screenplay/027-scene-comments-panel.jpg" width=720"/>

Clicking it opens a comment box per scene. For long scenes, the comment box
scrolls with the scene and shows a title bar for clarity.

<img src="../images/screenplay/028-scene-comments-panel.jpg" width=720"/>

### Featured Image

The comment box has multiple tabs. Click the Featured Image tab to assign a
photo per scene. Drag and drop or click “Select Photo”. Typically this is a
storyboard sketch or location photo, but you can choose any image.

<img src="../images/screenplay/029-featured-image.jpg" width=720"/>

### Index Card Fields

Use the last tab to capture structured metadata per scene. By default, Index
Card Fields includes:

- Conflict
- Emotional Change
- Page Target

You can edit these fields by clicking the Edit icon; configure up to five
fields. These fields can also be edited in the Structure tab.

<img src="../images/screenplay/030-index-card-fields.jpg" width=720"/>

### Scene Meta Data

When synopsis, characters & tags are hidden in screenplay editor, they show up
in the fourth tab on the comments panel. This can be handy, because the comments
panel scrolls along with the scene.

<img src="../images/screenplay/031-meta-data.jpg" width=720"/>

## Character Presense

Scrite automatically tags character presence in a scene if they have dialogue.

For example, in the scene #23 below Linda has a dialogue, and you can notice
that Scrite has automatically captured the presence of this character.

<img src="../images/screenplay/032-auto-character-tagging.jpg" width=720"/>

Same is the case with scene #24, where Blank, Joni, Elliot, Linda, Richard and
Walt are captured as characters present in that scene.

This kind of tagging is especially useful when it comes to extracting reports.
For example, you may want to extract dialogues of a specific character across
all scenes, or get a table of character presence across all scenes in the film.

### Adding Mute Characters

Notice that in Scene #23, while Linda has a dialogue, Meg, Richard and Joni are
also present in the scene, but they are mute. So, strictly speaking they are
present in the scene and as such should be tagged as well.

You can add mute characters to the scene either manually, or by asking Scrite to
scan for mute characters automatically. To manually add a mute character, click
on the + icon and enter the character name.

<img src="../images/screenplay/033-add-mute-character.jpg" width=360"/>

You can add as multiple mute characters. Notice that mute characters will have a
cancel icon, which makes it possible to remove them from the scene.

<img src="../images/screenplay/034-add-mute-character.jpg" width=640"/>

To automatically scan for mute characters, trigger the "Scan Mute Characters"
action from the [command center](user-interface.md#command-center).

<img src="../images/screenplay/035-scan-mute-characters.jpg" width="85%"/>

Scrite will look for references to character names in the scene, who may not
have a dialogue but are present in the scene nevertheless. However, please note
that Scrite only recognizes characters as mute in a scene if they are present in
atleast one other scene in the screenplay, either because they have a dialogue
or because they are mute.

> NOTE: Scanning for mute characters currently works only for character names in
English.

### Invisible Characters

In some cases, it is possible that a character has a dialogue in the scene but
is invisible.

<img src="../images/screenplay/036-capture-invisible-characters.jpg"
width="85%"/>

In the scene above, Joni has a dialogue but is invisible, because we only hear
Joni’s voice but don’t actually see Joni on the scene. Said in other words, Joni
is invisible.

While extracting character presence reports, we need to make sure that Joni’s
presence is not marked in this scene, let it impacts scheduling chart that an AD
may create for the actor who plays Joni.

Invisible character names are shown in italics, if "Capture Invisible Character"
is checked in Settings > Screenplay > Options page.

<img src="../images/screenplay/037-capture-invisible-characters.jpg"
width="85%"/>

### Rename Character

If you have “Show Scene Characters and Tags” turned on in Screenplay Editor
options, then you can click on a character name in the character list shown
under scene heading and select “Rename/Merge Character” option ….

<img src="../images/screenplay/038-rename-characters.jpg" width="85%"/>

… and Scrite will then present a dialog box for you where you can provide a new
name for your character.

<img src="../images/screenplay/039-rename-characters.jpg" width="85%"/>

In this dialog you can either provide a new name and Scrite will rename it for
you.

Once renamed, the changes cannot be undone. However, you can rename the new name
back to its original name.

Scrite not only changes the name in character paragraphs, but also in scene
headings and other paragraphs, including references to the name in scene
synopsis and notes.

NOTE: Character renaming works best with English names written in Latin charset.
If you have written your entire Screenplay in other language(s), then Scrite
can’t properly rename your characters unless an entire word is found with the
character name in it.

### Merge Characters

Suppose that you have a screenplay where you may have accidentally used two
names for the same character. In this case, Maya and Mayavi.

<img src="../images/screenplay/040-merge-characters.png" width="85%"/>

You may want to merge Mayavi into Maya across the entire screenplay, even if the
name is used in action or dialogue paragraphs. Just like you would rename
characters, you can click on Mayavi and select the “Rename/Merge Character”
option.

<img src="../images/screenplay/041-merge-characters.png" width="85%"/>

Scrite will present a dialog for you to enter the new character name. In that
dialog box, write the name of the character you want to merge this one with.

<img src="../images/screenplay/042-merge-characters.png" width="85%"/>

Now, upon clicking the “Rename” button, you will notice that Scrite recognizes
it as a merge workflow and will ask you to confirm it.

<img src="../images/screenplay/043-merge-characters.png" width="85%"/>

Upon clicking “Yes” in this dialog box, Scrite not only changes Mayavi to Maya
everywhere in the screenplay, but it will also move notes, attachments, and
photos associated with Mayavi with the notes, attachments, and photos of Maya.

> NOTE: This works works best with English names written in Latin charset. If
> you have written your entire Screenplay in other language(s), then Scrite
> can’t properly rename your characters unless an entire word is found with the
> character name in it.

## Story Beats & Keywords

Scrite lets you tag additional meta-data to your scenes to get an overall
impression of your story structure, design or flow. The way in which all of this
is shown in the screenplay tab differs from the way in which its shown in the
structure tab. Together they give you a comprehensive view of your story.

### Formal Story Beats

By formal story beats we mean beats from story structures like `Save The Cat`,
or `Hero's Journey`, or even custom beat sheets you may create.

With `Characters & Tags` enabled you can click on the plus sign next to `Formal
Tags` to tag the scene with one or more beats from your story structure.

<img src="../images/screenplay/054-formal-tags.jpg" width="85%"/>

By default, Scrite offers beats from `Save The Cat` beat sheet. But you can add
your own beats by clicking on the `Customize` button in the popup.

<img src="../images/screenplay/055-formal-tags.jpg" width="85%"/>

The syntax for describing story beats is self-explanatory.

### Keywords

In addition to formal story beats, you can tag your scenes with custom keywords
which can offer additional context to your scenes.

Just click on the plus icon next to `Keywords` ...

<img src="../images/screenplay/056-keywords.jpg" width="85%"/>

... and add a keyword in the text field shown there. You can add any number of
keywords to a scene.

Keywords and tags thus added will be rendered as tracks along side scenes in the
[scene list panel](#scene-list-panel).

### Sequences

As a writer you may want to bundle multiple scenes into a sequence, because they
all form a logical group in your story. Simply select a range of scenes and
bundle them together into a sequence.

<img src="../images/screenplay/015-scene-list-panel-sequence.jpg" width=640"/>

Scenes that are part of a sequence have a arrow prior to their scene number in
the [scene list panel](#scene-list-panel).

You can easily add/remove scenes to a sequence by using the context menu.
Bundling scenes together like this allows you to capture rich detail about the
way you look at elements of the story unfolding across scenes in the screenplay.

## Writing with Scene‑Centric Precision in Scrite

Scrite’s scene‑centric architecture treats each scene as a discrete block of
text, loading only what’s visible to keep memory usage low and performance
smooth. Most professional screenplays use short scenes, with occasional longer
ones. Scrite is optimized for this style. If most scenes exceed two pages,
performance may degrade.

Best practices:

- Keep most scenes under a page.
- Use a few two‑page scenes sparingly.
- For long sequences in one location, break into smaller scenes: start with a
  proper heading, divide into logical beats, and omit headings for subsequent
  beats if desired.

### Long Scene Warning

By default, Scrite displays a warning icon next to scenes that have more than
150 words.

<img src="../images/screenplay/057-long-scene-warning.jpg" width="85%"/>

You can edit this treshold, or turn off long scene warning all together in
Settings > Screenplay > Options page.

<img src="../images/screenplay/058-long-scene-warning.jpg" width="100%"/>

### Scenes Without Heading

You can turn off scene headings for specific scenes. Pull out the scene menu and
uncheck “Scene Heading”. The editor shows “NO SCENE HEADING”. Such scenes are
shown as is in preview and exported as such in PDF and reports.

<img src="../images/screenplay/044-no-scene-heading.png" width="100%"/>

### Inserting Scenes

To insert a scene in-between two existing scenes in the screenplay, simply make
the first of the two scenes active…

<img src="../images/screenplay/045-insert-scenes.png" width="100%"/>

… then click on the add scene button in the toolbar …

<img src="../images/quickstart/013-adding-elements.png" width="40%"/>

… or use the [keyboard shortcut](./user-interface.md#keyboard-shortcuts)
`Ctrl+Shift+N`.

<img src="../images/screenplay/046-insert-scenes.png" width="100%"/>

Scrite automatically adds a scene, whose heading is a slight variation of the
previous scene. You can edit both the scene heading and the scene contents.

By default, Scrite places cursor in the newly added scene's content area. If you
want to alter the auto-generated heading for the new scene, then you can hit
`Ctrl+0` to switch focus to the scene heading. Alternatively, you can check the
"Set Cursor on Heading in New Scenes" options in "Screenplay Editor Options"
accessible from the [command center](user-interface.md#command-center).

<img src="../images/screenplay/047-insert-scenes.png" width="85%"/>

### Removing Scenes

Removing a scene is as simple as pulling out the scene menu and selecting the
“Remove” option.

<img src="../images/screenplay/048-remove-scene.jpg" width="50%"/>

> NOTE: Removed scenes are not deleted, they are just no longer a part of the
> screenplay. You can always add them back by switching to the Structure tab.

### Omit Scenes

Sometimes, the entire script is handed over to the production team, who may have
used the scene numbers as stated in the screenplay. Later, you might want to
remove a scene. However, removing a scene alters the scene numbers of all
subsequent scenes, which can be an issue. Instead, you can omit the scene.

<img src="../images/screenplay/051-omit-scene.jpg" width="50%"/>

Omitted scenes appear in the screenplay but are hidden in the UI and generated
PDFs. This reduces the screenplay’s time and page count. You can click the
"Include" button to bring them back.

<img src="../images/screenplay/052-omit-scene.jpg" width="100%"/>

This approach keeps the scene numbers intact while effectively removing the
scene.

### Splitting Scenes

Place the cursor before the first character of the paragraph where you want to
split, then right‑click and select “Split Scene” or use `Ctrl+Shift+Enter`
(Windows/Linux) or `⌘+Shift+Return` (macOS). Dialogue character lists are split
accurately; mute characters remain in the original scene.

<img src="../images/screenplay/049-split-scene.jpg" width="85%"/>

### Merging Scenes

Place the cursor at the first line of the scene to merge into the previous one,
then right‑click “Join Previous Scene” or use `Ctrl+Shift+Backspace`
(Windows/Linux) or `⌘+Shift+Backspace` (macOS). Scrite inserts a separator
paragraph; you can delete it.

<img src="../images/screenplay/050-merge-scene.jpg" width="85%"/>

### Reordering Scenes

Drag scenes in the Scene List panel to reorder. Heading‑less scenes inherit
context from the previous scene in their new location.

<img src="../images/quickstart/009-reorder-scenes.gif" width="40%"/>

### Page Breaks

While generating PDF files you can configure Scrite to generate each scene, or
episode, or act on a new page. In addition to that, you can also insert page
breaks before and after scenes. This allows you to bunch a group of scenes in
sequence while generating PDFs.

<img src="../images/screenplay/059-page-breaks.jpg" width="100%"/>

## Acts

You can break your screenplay into multiple acts by adding act breaks. Simply
click on the `Act Break` icon in the toolbar, or use the keyboard shortcut
`Ctrl+Shift+B` to insert an act break after the current scene.

<img src="../images/quickstart/013-adding-elements.png" width="40%"/>

By default, Scrite uses `Save The Cat` story structure. So acts are named ACT 1,
ACT 2A, ACT 2B and ACT 3. But you can offer your own act naming convention by
editing the [formal story beats](#formal-story-beats) used in your screenplay.

<img src="../images/screenplay/070-act-breaks.jpg" width="100%"/>

You can move act breaks up and down the scene list panel, just like you can
[move up and down scenes](#reordering-scenes).

## Episodes

Just like act breaks, you can add episode breaks as well. This is especially
useful when you are writing screenplay for multiple episodes in a single
document. To insert an episode break after the current scene use the `Episode
Break` button on the toolbar, or the keyboard shortcut `Ctrl+Shift+P`.

<img src="../images/screenplay/071-episode-breaks.jpg" width="100%"/>

### Episode Scene Numbers

Act numbers reset after each episode, but scene numbers remain linear by
default. You can restart scene numbers after each episode by toggling the
`Restart Episode Scene Numbers` option.

<img src="../images/screenplay/072-episode-breaks.jpg" width="100%"/>

## Copy / Paste

You can copy a text snippet, a couple of paragraphs, an entire scene or several
scenes at once to the clipboard. You can also pasted text from a second Scrite
instance, or from another application too. Copy/paste in an app like Scrite,
while easy to use, has certain nuances which is useful to be aware of.

### Coping text snippets

When you select a snippet of text and copy `Ctrl+C`, Scrite copies the selected
text to clipboard. 

<img src="../images/screenplay/060-copy-text-snippet.jpg" width="100%"/>

You can paste the copied text elsewhere in the same Scrite document, or into
another, or into a document opened in any 3rd party app on your Desktop.

<img src="../images/screenplay/061-copy-text-snippet.jpg" width="100%"/>

### Copying an entire scene

If your selection includes more than one block of text or an entire scene, ... 

<img src="../images/screenplay/062-copy-full-scene.jpg" width="100%"/>

...then by default, Scrite copies a Fountain representation of the selection
when you trigger Copy `Ctrl+C`.

<img src="../images/screenplay/063-copy-full-scene.jpg" width="100%"/>

The benefit of copying text in Fountain format is that it makes pasting into
Scrite, or 3rd-party screenwriting apps more productive because Fountain
preserves formatting.

However, if you prefer to copy in plain-text, then you can uncheck the `Copy
text in Fountain format` in Settings > Screenplay > Options.

<img src="../images/screenplay/064-copy-full-scene.jpg" width="100%"/>

### Copying several scenes at once

You can select one or more scenes on the scene list panel, right click and click
on `Copy` or use keyboard shortcut `Ctrl+Shift+C` to copy the selected scenes at
once.

<img src="../images/screenplay/065-copy-many-scenes.jpg" width="100%"/>

Scrite copies a Fountain representation to the clipboard.

<img src="../images/screenplay/066-copy-many-scenes.jpg" width="100%"/>

### Pasting content

Text snippets are pasted inline, much like other apps do.

However if the text on the clipboard contains several blocks of text, then
Scrite parses it as a Fountain file and lets you paste content into a specific
scene. You could also select the `Paste After` option (`Ctrl+Shift+V`) to paste
content after the current scene. This is especially useful while copy/pasting
content to 3rd party apps.

### Copy/Pasting Scenes to 3rd Party Apps

After selecting one or more scenes on the scene list panel, you can right click
and copy selected scenes in Fountain format to the clipboard. If the clipboard
has plain text or Fountain, "Paste After" lets you insert it after the current
scene.

This way you can copy content out of Scrite to third party apps, and back.

![type:video](https://www.youtube.com/embed/GbAsuGV2lmY)

## Undo / Redo

Scrite offers undo `Ctrl+Z` and redo `Ctrl+Y` or `Ctrl+Shift+Z` much like other
Desktop apps. However, since Scrite has a hierarchy of documents some edits
cannot be undone. 

> Note: As of writing, Undo/Redo is a work in progress. As you face issues
> please consider adding a description of the issue to this [dedicated
> thread](https://discord.com/channels/867082699716689951/1460194668959633534/1460194668959633534)
> on our Discord community server.

## Spell Check

Scrite integrates with your operating system’s spell check, sharing dictionaries
with other apps. If the language pack installed in your OS for a given language
comes bundled with a spell-check dictionary, then Scrite will simply make use of
that to mark mispelled words and offer suggestions from the same.

<img src="../images/screenplay/067-spell-check.jpg" width="100%"/>

> NOTE: Scrite completely relies on the operating-system to drive the
> spell-check functionality. It does not come bundled with a dictionary of its
> own.

Misspelled words are highlighted in red; right‑click for corrections. 

<img src="../images/screenplay/068-spell-check.jpg" width="40%"/>

Choose “Add to dictionary” (system‑wide) or “Ignore” (current document only).
Scrite recognizes character names as correct by default. 

> NOTE: Grammar check is not yet available.

You can enable/disable spell-check in Settings > Screenplay > Options page.

<img src="../images/screenplay/069-spell-check.jpg" width="100%"/>


