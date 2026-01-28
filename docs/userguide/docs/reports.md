# Reports

Scrite’s **Reports** lets you generate ready‑made, exportable summaries of your
screenplay from a variety of perspectives. Most reports can be exported as a PDF
or ODT file (and some as CSV) and is designed to help you analyze, share, or
present your work.

 - [Character Report](#character-report) – an extract of dialogues for one or
   more selected characters.
 - [Character Screenplay](#character-screenplay) – an extract of scenes in which
   the chosen characters appear, with their dialogue highlighted in yellow.
 - [Location Report](#location-report) – an extract of all locations used and
   the scenes in which they appear.
 - [Notebook Report](#notebook-report) – anextract of all story, scene, and
   character notes captured in the Notebook.
 - [Scene‑Character Matrix](#scenecharacter-matrix) – table showing which
   characters appear in each scene.
 - [Screenplay Subset](#screenplay-subset) – an extract of a manually selected
   subset of scenes or of scenes that match specific keywords, beats, or
   episodes.
 - [Statistics Report](#statistics-report) – Single‑page PDF with visual charts
   (action‑to‑dialogue ratio, location distribution, character presence,
   timeline, pacing, etc.).
 - [Two‑Column Report](#twocolumn-report) – PDF/ODT of the screenplay in a
   two‑column audio‑video format.

## Reports Menu

When you have a screenplay open in Scrite, click on the Report menu to get a
list of report options.

<img src="../images/reports/001-reports-menu.jpg" width="60%"/>

Another way to get to export options is by typing `Reports` in the [Command
Center](./user-interface.md#command-center).

<img src="../images/reports/002-report-options.jpg" width="100%"/>

You can assign a shortcut of your choice to frequently used reports to invoke
them quickly. Additionally, certain reports can be invoked from a scene or
character context. We'll look into all those details in one or more sections
below.

## Layout of Report Configuration Dialogs

Reports can be finely configured before generating files. Unlike export, the
configuration dialog box for reports are a bit more elaborate in that they
present options across one or more pages.

<img src="../images/reports/003-dialog-box-layout.jpg" width="60%"/>

We will not be going through each option in every report within this chapter,
but we will broadly present the kind of options available in each report. We
encourage you to switch pages in a report configuration dialog box to explore
all options available.

In general 

- The **Basic** page lets you select export file type (PDF vs ODT), and specify
  watermark and comment texts for use in PDF.
- The **Options** page lets you toggle one or more options to control report
  generation
- Other pages let you filter or lookup aspects of the screenplay to include in
  the report.

## Character Report

Exports a PDF/ODT containing all dialogues for one or more selected characters.
Useful for reviewing a character’s voice or for creating a character‑specific
script.

Upon invoking this report in the menu or command center, you can switch to the
Characters tab to select one or more characters whose dialogues you want
included in the report.

<img src="../images/reports/004-character-report.jpg" width="85%"/>

Hit the `Generate` button once you've selected characters to include to see a
preview of the report.

<img src="../images/reports/005-character-report.jpg" width="100%"/>

The preview can then be saved to disk using the `Save As` option in the dialog
box footer bar.

This report, and others that support filtering by character names, can be
invoked by clicking on the character name bubble as well.

<img src="../images/reports/006-character-report.jpg" width="60%"/>

## Character Screenplay

Creates a PDF/ODT that lists every scene in which the chosen characters appear,
with their dialogue highlighted in yellow. Great for quick reference or for
sending a focused script to collaborators.

Upon invoking this report in the menu or command center, you can switch to the
Characters tab to select one or more characters whose dialogues you want
included in the report.

<img src="../images/reports/007-character-screenplay.jpg" width="85%"/>

To further filter for scenes that match a keyword or story beat, you can switch
to the `Keywords` and `Tags` page.

<img src="../images/reports/008-character-screenplay.jpg" width="85%"/>

Similarly, if you are working on a multi-episode screenplay switch to the
`Episodes` page and select the episodes you want to filter.

Hit the `Generate` button once you've configured all options.

<img src="../images/reports/009-character-screenplay.jpg" width="100%"/>

The preview can then be saved to disk using the `Save As` option in the dialog
box footer bar.

This report, and others that support filtering by character names, can be
invoked by clicking on the character name bubble as well.

<img src="../images/reports/010-character-screenplay.jpg" width="60%"/>

## Location Report

Generates a PDF/ODT that lists all locations used in the screenplay and the
scenes in which they appear. Handy for production planning and location
scouting.

Upon invoking this report in the menu or command center, you will see a dialog
box like this. This is a very simple report so there aren't many options to
configure.

<img src="../images/reports/011-location-report.jpg" width="85%"/>

Hit the `Generate` button to generate the report.

<img src="../images/reports/012-location-report.jpg" width="100%"/>

The preview can then be saved to disk using the `Save As` option in the dialog
box footer bar.

If you select Open Document Text in the report dialog box, then Scrite will
generate an ODT file for you with the same contents.

<img src="../images/reports/013-location-report.jpg" width="100%"/>

## Notebook Report

Exports a PDF/ODT that compiles all notes from the Notebook, including story,
scene, and character annotations. Ideal for keeping a single source of truth for
all research and ideas.

<img src="../images/reports/014-notebook-report.jpg" width="100%"/>

More information about all the variations of this report are explained in the
chapter on [Notebook Tab](./notebook.md).

## Scene‑Character Matrix

Produces a PDF or CSV table that shows which characters appear in each scene.
Useful for tracking character presence and balancing scenes, and during
production planning.

Upon invoking this report in the menu or command center, you will see a dialog
box like this. 

<img src="../images/reports/015-scene-character-matrix.jpg" width="85%"/>

By default all characters are selected, but you can shortlist only a subset of
characters if you like. The PDF report looks like this.

<img src="../images/reports/016-scene-character-matrix.jpg" width="100%"/>

When exported in ODT format, the app actually generates a CSV file which can
then be opened in any spreadsheet app.

<img src="../images/reports/017-scene-character-matrix.jpg" width="100%"/>

## Screenplay Subset

Creates a PDF/ODT of a selected subset of scenes. You can choose scenes manually
or automatically include scenes that match specific keywords, beats, or
episodes.

A typical usecase of this report is to extract scenes for a specific location.
In the configuration dialog for this report, switch to the `Scenes` page and
enter the location name in the filter above to get a filtered set of scenes.
Then hit `Select All` to select them.

<img src="../images/reports/018-screenplay-subset-report.jpg" width="85%"/>

Then hit the `Generate` button to generate a PDF of just the selected scenes.

<img src="../images/reports/019-screenplay-subset-report.jpg" width="100%"/>

Another way to genrate this report is by select a bunch of scenes on the [scene
list panel](./screenplay.md#scene-list-panel), and invoking this report from the
context menu.

<img src="../images/reports/020-screenplay-subset-report.jpg" width="85%"/>

Scenes selected on the scene list panel will already show up checked in the
dialog box. You can configure more options if you like and generate the report
from there.

## Two‑Column Report

Generates a PDF/ODT that presents the screenplay in a two‑column audio‑video
format, making it easier to read on screens or to use as a cue sheet.

This report is much like the [screenplay subset report](#screenplay-subset),
except it generates output in two column format. You can pick from one of the
layouts listed in the configuration dialog box, and even customise the space
distribution between the columns.

<img src="../images/reports/021-two-column-report.jpg" width="85%"/>

Upon generating the report you will notice that the entire screenplay is
presented in a two column format. All the visuals in the column on the left,
with the dialogues on the right column.

<img src="../images/reports/022-two-column-report.jpg" width="100%"/>

## Statistics Report

Outputs a single‑page PDF with visual charts that display key metrics such as
action‑to‑dialogue ratio, location distribution, character presence, timeline,
pacing, and more. Great for quick analytics.

The configuration dialog box for this report lets you pick characters and
locations to include. By default the report picks up to 6 most prominent
characters and locations.

<img src="../images/reports/023-statistics-report.jpg" width="85%"/>

Upon hitting `Generate` button you will get a one page PDF that looks like this.

<img src="../images/reports/024-statistics-report.jpg" width="100%"/>

By making use of graphs this one single page communicates the ratio of action to
dialog, interior to exterior scenes, time distribution between various acts and
story beats, pacing of scenes, the way in which character presense and location
occupancy is spread across the entire timelength of the screenplay.
