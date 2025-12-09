# Advanced Editing Features

The screenplay shown in screenshots are from the 2019 film Knives Out. The original screenplay can
be downloaded from
[here](https://lionsgate.brightspotcdn.com/fb/14/23cd58a147afbb5c758ecb3dff0a/knivesout-final.pdf)
and all rights for the screenplay rests with its owners. We are using the screenplay only for
explaining features in the product.

<img src="../images/advanced-editing/001-knives-out.jpg" width="720"/>

## Capturing The Logline

A logline is a 2–3 line pitch for your screenplay. It often appears as the short description of your
film on IMDb.

If you want to capture the logline of your screenplay in Scrite, you can toggle the logline editor
in the Screenplay Editor Options Dialog.

<img src="../images/advanced-editing/002-logline.jpg" width=480"/>

Once enabled, Scrite presents a logline field below the title page on the Screenplay Editor. You can
capture the logline in this field.

<img src="../images/advanced-editing/003-logline.jpg" width=640"/>

As you can see, Scrite highlights the last few words in the logline in red color. This is because
they spill over the recommend character limit for loglines. While it is recommended that you don't
exceed this limit, Scrite does not prohibit you from capturing longer logline.

## Color Coding Scenes

While plotting, many writers use post‑it notes on a pinboard. Some prefer color coding; others
don’t. If you like color coding scenes, Scrite supports it.

Select one or more scenes on the Scene List Panel, and right-click to view a menu. From here you can
select a color to assign to your selected scenes.

<img src="../images/advanced-editing/004-scene-colors.jpg" width=640"/>

As you assign colors to different scenes, the Scene List panel uses the same color coding to display
scene headings, as does the scene heading area in the screenplay editor.

Infact, you can also open the scene options menu in the screenplay editor to assign scene color for
a specific scene.

<img src="../images/advanced-editing/005-scene-colors.jpg" width=480"/>

## Color Intensity

Some writers prefer to see intense colors for their scenes, and some prefer subdued colors. You can
change the color intensity by clicking on Screenplay Editor Options Dialog, and dragging to color
intensity slider as you see fit.

<img src="../images/advanced-editing/006-color-intensity.jpg" width=480"/>

## Scene List Panel

You can toggle the visibility of the Scene List panel by clicking on the button along the left edge
of the app window. Using the Scene List Panel, you can rapidly jump to a scene.

By default, clicking on a scene in the Scene List panel causes the scene to get selected. You can
select multiple scenes using Ctrl+Click, or hold Shift to select a range.

Whether you select one scene or many, you can right‑click to change colors for all selected scenes.
You can also copy selected scenes in Fountain format to the clipboard. If the clipboard has plain
text or Fountain, “Paste After” lets you insert it after the current scene.

< screenshot >

### Empty Scene Icon

The Scene List panel shows an “empty‑scene” icon for scenes with only a scene‑heading but no
content. This icon disappears as soon as some content is typed or pasted into the scene.

< screenshot >

## Page Number and Screen Time

Although Scrite isn’t page‑centric, page counts still matter:

- Each page ≈ 1 minute of screen time, so page count helps estimate duration.
- Many structures are page‑centric (e.g., Save The Cat breaks into Act II at page 25).

Scrite lets you keep track of current page, total page count, current time and total time on the
status bar. This is turned off by default because estimating page count and time while working is
computationally intensive, which can make the app feel sluggish on large screenplays. We’re working
on faster algorithms; until then, page counting is off by default.

Whenever you open a Scrite document, you will notice a tooltip along the bottom‑left corner of the
screenplay editor. You can toggle page counting by clicking the page icon.

Page counting will be disabled if you close and reopen a document, even if you toggled it on. To
preserve your preference across sessions, go to Settings → Screenplay → Options and enable “Remember
Time & Page Count Settings”. You can toggle time and page counting anytime using the book/time icon
at the bottom‑left of the status bar.

When enabled, the status bar shows page at cursor, total page count, time at cursor, and total time.
A page break marker with page number appears to the left of the page boundary in the editor. As you
move the cursor, Scrite updates these values; as you scroll, you’ll notice page number bubbles
indicating page breaks within your scene.

Note: When exporting to PDF, page count may change depending on MORE and CONT’D usage. Time values
are approximate (1 page ≈ 1 minute). You can adjust “Time Per Page” in Settings → Screenplay → Page
Setup.

Note: Page counts vary across apps. Final Draft, Scrite, and Fade In Pro may report different counts
for the same screenplay; making them identical is rarely possible.

< screenshot >

### Viewing Scene Lengths on the Scene List Panel

With page‑counting ON, you can toggle scene lengths in the Scene List panel to display approximate
scene duration or page length for each scene.

< screenshot >

## Formatting Paragraphs

You can format paragraph types and have that format applied to all paragraphs of those types. Go to
Settings → Screenplay → Formatting Rules to view and edit the currently applied rules. Click any
paragraph in the preview and alter properties. Close the dialog and Scrite applies your formatting
rules across all scenes. These formats are used in preview, exported PDFs, and reports.

Scrite does not apply special formatting to fragments of text (bold, italic, underline, color)
within paragraphs via Formatting Rules. Use Markup Tools for inline formatting.

Formatting options are saved with the file and reused each time you open it, even if another file
used different options. Scrite maintains two sets of formatting options:

1. Default formatting options used for new files.
2. Document‑specific options used for the current file.

Click “Factory Reset” to discard document‑specific options and use defaults. Click “Make Default” to
set current options as your defaults.

< screenshot >

## Markup Tools

Markup Tools let you apply inline formatting to specific snippets of text. The Markup Tools dock is
hidden by default; toggle it from the Screenplay Editor Options menu. You can move the dock to any
part of the screen.

Select any text snippet and use Markup Tools to apply custom formatting. Hover over buttons to see
shortcuts:

- Bold: Ctrl+B
- Italics: Ctrl+I
- Underline: Ctrl+U
- Strikeout: Ctrl+R
- ALL CAPS: Shift+F3
- small caps: Ctrl+Shift+F3

< screenshot >

## Custom Scene Numbers

By default, Scrite generates scene numbers automatically and regenerates them if you change order or
insert scenes. You can apply custom numbers (e.g., 1, 1A, 1B) by editing the scene number field.
Automatic numbering resumes for scenes without custom numbers. Custom numbers appear in preview,
exported PDFs, and reports.

< screenshot >

## Scene Comments, Featured Image and Index Card Fields

Scrite lets you capture additional information per scene:

- Comments
- Featured Image
- Index Card Fields
- Photos, Videos, Documents
- Rich Text Notes

The Notebook tab is best for capturing this information, but the Screenplay editor offers access to
Comments, Featured Image, and Index Card Fields.

### Comments

Enable “Scene Comments” in Screenplay Editor Options to show a pullout for each scene. Clicking it
opens a comment box per scene. For long scenes, the comment box scrolls with the scene and shows a
title bar for clarity.

< screenshot >

### Featured Image

The comment box has three tabs. Click the Featured Image tab to assign a photo per scene. Drag and
drop or click “Select Photo”. Typically this is a storyboard sketch or location photo, but you can
choose any image.

< screenshot >

### Index Card Fields

Use the last tab to capture structured metadata per scene. By default, Index Card Fields includes:

- Conflict
- Emotional Change
- Page Target

You can edit these fields by clicking the Edit icon; configure up to five fields. These fields can
also be edited in the Structure tab.

< screenshot >

## Tagging Character Presence in Scenes

Scrite automatically tags character presence in a scene if they have dialogue. This tagging is
useful for extracting reports (e.g., dialogues of a specific character or character presence across
scenes).

### Adding Mute Characters

Add mute characters manually via the + icon, or automatically using “Scan for Mute Characters” in
Screenplay Editor Options. Scrite looks for references to character names present in the scene
without dialogue. Mute characters are recognized if they appear in at least one other scene.
Scanning currently works only for English names.

< screenshot >

### Invisible Characters

Characters with dialogue may be invisible (e.g., voice heard but character not seen). Invisible
character names are shown in italics if “Capture Invisible Character” is checked in Settings →
Screenplay → Options.

< screenshot >

### Rename Character

With “Show Scene Characters and Tags” enabled, click a character under the scene heading and select
“Rename/Merge Character” to rename. Scrite updates character names across headings, paragraphs,
synopsis, and notes. Works best with English names in Latin script.

< screenshot >

### Merge Characters

If you used two names for the same character, use “Rename/Merge Character” to merge. Scrite updates
occurrences across the screenplay and moves notes, attachments, and photos accordingly. Works best
with English names in Latin script.

< screenshot >

## Writing with Scene‑Centric Precision in Scrite

Scrite’s scene‑centric architecture treats each scene as a discrete block of text, loading only
what’s visible to keep memory usage low and performance smooth. Most professional screenplays use
short scenes, with occasional longer ones. Scrite is optimized for this style. If most scenes exceed
two pages, performance may degrade.

Best practices:

- Keep most scenes under a page.
- Use a few two‑page scenes sparingly.
- For long sequences in one location, break into smaller scenes: start with a proper heading, divide
  into logical beats, and omit headings for subsequent beats if desired.

### Scenes Without Heading

You can turn off scene headings for specific scenes. Pull out the scene menu and uncheck “Scene
Heading”. The editor shows “NO SCENE HEADING”. Such scenes are shown as is in preview and exported
as such in PDF and reports.

< screenshot >

### Inserting Scenes

Make the target scene active, then click “Add Scene” on the toolbar or use `Ctrl+Shift+N`. Scrite
inserts a new scene with a heading based on the previous scene; edit heading and contents as needed.

< screenshot >

### Removing Scenes

Pull out the scene menu and select “Remove”. Removed scenes aren’t deleted; add them back from the
Structure tab.

< screenshot >

### Splitting Scenes

Place the cursor before the first character of the paragraph where you want to split, then
right‑click and select “Split Scene” or use `Ctrl+Shift+Enter` (Windows/Linux) or `⌘+Shift+Return`
(macOS). Dialogue character lists are split accurately; mute characters remain in the original
scene.

< screenshot >

### Merging Scenes

Place the cursor at the first line of the scene to merge into the previous one, then right‑click
“Join Previous Scene” or use `Ctrl+Shift+Backspace` (Windows/Linux) or `⌘+Shift+Backspace` (macOS).
Scrite inserts a separator paragraph; you can delete it.

< screenshot >

### Reordering Scenes

Drag scenes in the Scene List panel to reorder. Heading‑less scenes inherit context from the
previous scene in their new location.

< screenshot >

## Spell Check

Scrite integrates with your operating system’s spell check, sharing dictionaries with other apps. As
of writing, Scrite supports spell check for English; more languages are planned.

Enable/disable in Settings. Misspelled words are highlighted in red; right‑click for corrections.
Choose “Add to dictionary” (system‑wide) or “Ignore” (current document only). Scrite recognizes
character names as correct by default. Grammar check is not yet available.

< screenshot >

## Auto Save

Scrite auto‑saves documents by default every 60 seconds. Auto‑save kicks in only after you save your
work to a `.scrite` file once. Scrite creates a folder alongside your `.scrite` file and stores
backups periodically.

You can toggle auto‑save, change the interval, and limit the number of backups in Settings →
Application → Options.

< screenshot >

## Loading A Backup

The toolbar shows the number of backups available for the current screenplay. Click it to open a
dialog and select a backup to load in a new or the current window. Backups load anonymously—you must
save them explicitly with a new file name, and they won’t overwrite the source file.

< screenshot >

## Shield

By default, Scrite stores screenplays in an interoperable format (see
https://www.scrite.io/interoperable-document-format/). If you want to restrict opening to devices
where Scrite is activated using your email ID, use Shield to lock screenplays.

Click the shield icon at the bottom‑left of the editor to open the dialog and toggle protection. You
can add collaborator email IDs to allow opening on their devices as well. Collaborators cannot turn
off Shield or add more collaborators.

Note: Shield encrypts contents of screenplay, notebook, and structure tabs. Photos/attachments in
notebook and the title page aren’t encrypted yet. While our keys aren’t public, we don’t guarantee
the mechanism is hack‑proof.

< screenshot >

## What’s next?

In the next article, learn how to write in Indian languages using Scrite:
https://www.scrite.io/index.php/typing-in-multiple-languages/
