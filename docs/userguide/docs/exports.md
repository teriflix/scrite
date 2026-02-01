# Exporting Files

Screenplays written in Scrite can be exported to several formats, each suited
for different purposes:

- [Adobe PDF](#adobe-pdf) — Share a read-only copy with others
- [Final Draft](#final-draft) — Use with production planning and script
  breakdown software
- [Fountain](#fountain) or [Plain Text](#plain-text) — Edit with markdown
  editors
- [Open Document Format](#open-document-format) — Edit in word processing
  software like Microsoft Word, Apple Pages, Google Docs, Zoho Docs, or
  Open/Libre Office
- [HTML](#html) — Publish on your website as a continuous scroll

## Export Menu

When you have a screenplay open in Scrite, click on the Export menu to get a
list of export options.

<img src="../images/exports/001-export-options.jpg" width="60%"/>

Another way to get to export options is by typing `Export` into the [Command
Center](./user-interface.md#command-center).

<img src="../images/exports/002-export-options.jpg" width="100%"/>

As you can notice the `Ctrl+P` shortcut is assigned to PDF Export by default.
You can assign a shortcut of your choice to all other export options.

## Adobe PDF

PDF is the most widely used format for sharing screenplays. It preserves your
screenplay's formatting perfectly across all devices and platforms, making it
ideal for sending to actors, directors, producers, or anyone who needs to read
your script without making changes.

When you select Adobe PDF from the [export menu](#export-menu), or [Command
Center](./user-interface.md#command-center), or by using the
[shortcut](./user-interface.md#keyboard-shortcuts) `Ctrl+P`, you will see a
dialog box like this.

<img src="../images/exports/003-pdf-export.jpg" width="85%"/>

This dialog box presents several options to customize the way in which you
generate the PDF export. Listed below are the options

|Option|Description|
|------|-----------|
|List characters for each scene|When checked, [character names](./screenplay.md#character-presense) are listed below each scene heading in the PDF file.|
|Include title & synopsis of each scene, if available|When checked, [synopsis and title](./screenplay.md#scene-meta-data) for each scene is included in a box directly below scene heading of each scene.|
|Include featured image for scene, if availale|When checked, the [featured image](./screenplay.md#featured-image) of each scene will be embedded right after the scene heading of each scene.|
|Use scene colors as background for scene headings|When checked, [color assigned to the scene](./screenplay.md#color-coding-scenes) is used as background to the scene heading in PDF.|
|Include scene comments, if available|When checked, any [comments captured against the scene](./screenplay.md#comments) will also be included in the PDF file.|
|Include scene content|This is checked by default, and it ensures that [scene content](./quickstart.md#typing-scene-content) is included in the PDF|
|Generate title page|This is checked by default, and it ensures that a properly formatted [title page](./screenplay.md#title-page) is generated as the first page of the PDF|
|Include logline in title page|When checked, it ensures that [logline](./screenplay.md#capturing-the-logline) is included in the title page if the previous option is also checked.|
|Use MORE and CONT'D breaks where appropriate|This is checked by default, and it ensures that dialog paragraphs at page boundaries are broken into two distinct blocks using MORE and CONT'D markers.|
|Include scene numbers|This is checked by default, and it ensures that [scene numbers](./screenplay.md#custom-scene-numbers) are mentioned against each scene in the generated PDF.|
|Include scene icons|This is checked by default, and it ensures that [scene icons](./screenplay.md#scene-type) are included next to the scene headings.|
|Print each scene on a new page|When checked, each scene is generated on a new page. This option is especially useful when you want to print the PDF file and distribute preproduction work for scenes among various AD teams.|
|Print each act on a new scene|When checked, it ensures that each act is generated on a new page in the PDF.|
|Include act breaks|When checked, it ensures that act breaks are clearly stated in the PDF.|

In addition to the checkable options stated above, you can customize the
[watermark text and comment](./screenplay.md#page-setup-and-watermark) to
optionally include in header or footer for this specific PDF file.

<img src="../images/exports/004-pdf-export.jpg" width="85%"/>

Upon clicking the `Generate PDF` button in the export dialog box, a PDF file is
generated and displayed for preview. To save this file, click on the `Save As`
button along the bottom bar and select a location to save the file.

<img src="../images/exports/005-pdf-export.jpg" width="100%"/>

> NOTE: Language fonts are bundled into the PDF so that they render exactly as
> displayed on your computer anywhere they get opened. We recommend generating
> PDFs with watermark to make it difficult for anybody to plagarize your work.

## Final Draft

Writers sometimes use multiple screenwriting apps while building their story.
With the Final Draft export option, we make it easy for writers to move between
various apps. FDX files exported from Scrite preserve a lot of formatting and
meta‑data. This also means that files imported from other FDX‑compatible apps
will retain formatting as well. The whole Final Draft export/import feature is
there to support the writer’s choice.

When you select Final Draft from the [export menu](#export-menu), or [Command
Center](./user-interface.md#command-center), you will see a dialog box like
this.

<img src="../images/exports/006-final-draft.jpg" width="85%"/>

By default files exported are stored in the Downloads folder within your HOME
directory, but you can select any path you like by clicking on the `Change Path`
link as pointed out in the screenshot above.

> FDX files exported by Scrite are compatible on versions up to Final Draft 13.

You can change the file name in the field provided for that, and optionally
configure the following additional options:

|Option|Description|
|------|-----------|
|Explicitly mark text-fragments of different languages|When checked, Scrite will mark text-fragments for each language and assign a font for use while rendering it in Final Draft app. Please note that it is not possible to embed fonts into FDX files, so it is possible that the Final Draft app itself may not use the suggested font. If unchecked, then Final Draft will use a single font for all languages which is typically `Courier Final Draft`.|
|Include scene synopsis|When checked, Scrite will include scene syposis for each scene whereever available.|

Click on the `Export` button in the dialog box to export the FDX file. Typically
Scrite will launch a `Finder` or `File Explorer` window with the exported file
selected, so that you can quickly open it in another app.

<img src="../images/exports/007-final-draft.jpg" width="100%"/>

At any point you can [import the changed Final
Draft](./imports.md#drag-and-drop-to-import) file back into Scrite and continue
working on it. Successive export & imports will not result in loss of screenplay
content itself, but may lead to loss of other meta data like notes, tags and so
on.

## Fountain

The [Fountain](https://www.fountain.io) export makes it easy to work with
screenwriting apps that may not support the FDX format. By exporting to
Fountain, you can keep your script in a plain‑text, markdown‑style format that
is widely supported by many writing tools, ensuring smooth collaboration and
flexibility. Importantly, Fountain preserves formatting even when using Indic
languages, making it a reliable choice for multilingual scripts.

When you select Fountain from the [export menu](#export-menu), or [Command
Center](./user-interface.md#command-center), you will see a dialog box like
this.

<img src="../images/exports/008-fountain.jpg" width="85%"/>

You can change the file name in the field provided for that, and optionally
configure the following additional options:

|Option|Description|
|------|-----------|
|Use ., @, !, > to explicitly mark scene heading, character, action and transisitions.|When checked, Scrite ensures the use of property [Fountain syntax](https://fountain.io/syntax/) for marking each paragraph type. This is especially useful if your screenplay uses non-English languages.|
|Use *, ** and _ to highlight italics, bold and underlined text.|When checked, Scrite marks texts formatted as bold, italics and underline.|

Once you are done with the configuration options, you can either click on the
`Export` option to export to a file, or click on the `Copy to Clipboard` option
to copy a fountain representation of your entire screenplay to the clipboard.
This is especially useful if you just want to paste the content to another app.

When exported, typically Scrite will launch a `Finder` or `File Explorer` window
with the exported file selected, so that you can quickly open it in another app.

<img src="../images/exports/009-fountain.jpg" width="100%"/>

At any point you can [import the changed Fountain
File](./imports.md#drag-and-drop-to-import) file back into Scrite and continue
working on it. Successive export & imports will not result in loss of screenplay
content, however you may need to reformat in parts occassionally.

## Plain Text

The plain‑text export does not use explicit markers for paragraph formatting
like Fountain does, but it relies on indentation to give the text file a
screenplay‑like appearance. This makes it easy to read and edit in any text
editor while still preserving the overall structure of the script.

When you select Text File from the [export menu](#export-menu), or [Command
Center](./user-interface.md#command-center), you will see a dialog box like
this.

<img src="../images/exports/010-plain-text.jpg" width="85%"/>

You can change the file name in the field provided for that, and optionally
configure the following additional options:

|Option|Description|
|------|-----------|
|Number of characters per line|The column width to use while generating text files. Scrite wraps texts to the next line once this character limit is reached.|
|Include scene numbers|When checked, scene numbers are included in the text file.|
|Include episode and act breaks|When checked, act and episode breaks are included in the text file.|
|Include scene synopsis.|When checked, scene synposis if available will be included in the text file.|

Once you are done with the configuration options, you can either click on the
`Export` option to export to a file, or click on the `Copy to Clipboard` option
to copy a plain text representation of your entire screenplay to the clipboard.
This is especially useful if you just want to paste the content to another app.

When exported, typically Scrite will launch a `Finder` or `File Explorer` window
with the exported file selected, so that you can quickly open it in another app.

<img src="../images/exports/011-plain-text.jpg" width="100%"/>

At any point you can [import the changed text
file](./imports.md#drag-and-drop-to-import) file back into Scrite and continue
working on it. Successive export & imports will not result in loss of screenplay
content, however you may need to reformat in parts occassionally.

> NOTE: While Scrite encodes exported text in UTF-8, not all plain-text viewers
> and editors render unicode fonts properly. Please refer to the documentation
> and support of third party text viewer or editor software for more
> information.

## Open Document Format

The Open Document Format (ODF) export gives users the flexibility to work with
professional word‑processing applications such as Microsoft Word, OpenOffice,
Google Docs, and others. The exported file retains the screenplay’s structure
and formatting, allowing writers to edit, annotate, and collaborate in familiar
office environments.

When you select Open Document Format from the [export menu](#export-menu), or
[Command Center](./user-interface.md#command-center), you will see a dialog box
like this.

<img src="../images/exports/012-odf.jpg" width="85%"/>

You can change the file name in the field provided for that, and optionally
configure the following additional options:

|Option|Description|
|------|-----------|
|List characters for each scene|When checked, [character names](./screenplay.md#character-presense) are listed below each scene heading in the ODT file.|
|Include title & synopsis of each scene, if available|When checked, [synopsis and title](./screenplay.md#scene-meta-data) for each scene is included directly below scene heading of each scene.|
|Include featured image for scene, if availale|When checked, the [featured image](./screenplay.md#featured-image) of each scene will be embedded right after the scene heading of each scene.|
|Use scene colors as background for scene headings|When checked, [color assigned to the scene](./screenplay.md#color-coding-scenes) is used as background to the scene heading in ODT.|
|Include scene comments, if available|When checked, any [comments captured against the scene](./screenplay.md#comments) will also be included in the ODT file.|
|Include scene content|This is checked by default, and it ensures that [scene content](./quickstart.md#typing-scene-content) is included in the ODT|
|Include scene numbers|This is checked by default, and it ensures that [scene numbers](./screenplay.md#custom-scene-numbers) are mentioned against each scene in the generated ODT.|

Once you are done with the configuration options, you can click on the `Export`
option to export to a ODT file. Typically, Scrite will launch a `Finder` or
`File Explorer` window with the exported file selected, so that you can quickly
open it in another app.

<img src="../images/exports/013-odf.jpg" width="100%"/>

At the moment it is not possible to import a ODT or DOCX file in Scrite.
However, you can copy a plain-text representation of the entire screenplay to
the clipboard and use the [New from Clipboard](./imports.md#new-from-clipboard)
option to import the copied text into Scrite.

> NOTE: While Scrite encodes formatting options in the generated ODT file, you
> may notice formatting errors when the file is opened in a third party app.
> Please refer to the documentation and support of third party text viewer or
> editor software for more information.

## HTML

Exporting to HTML is ideal for publishing a screenplay directly on a website.
The resulting file preserves the layout and formatting of the script while being
lightweight and web‑friendly, making it easy to embed or link to from a site.

When you select HTML from the [export menu](#export-menu), or [Command
Center](./user-interface.md#command-center), you will see a dialog box like
this.

<img src="../images/exports/014-html.jpg" width="85%"/>

You can change the file name in the field provided for that, and optionally
configure the following additional options:

|Option|Description|
|------|-----------|
|Include scene numbers|When checked, it ensures that [scene numbers](./screenplay.md#custom-scene-numbers) are mentioned against each scene in the generated HTML.|
|Export with scene colors|When checked, [color assigned to the scene](./screenplay.md#color-coding-scenes) is used as background to the scene heading in HTML.|

Once you are done with the configuration options, you can click on the `Export`
option to export to a HTML file. Typically, Scrite will launch a `Finder` or
`File Explorer` window with the exported file selected, so that you can quickly
open it in a browser. 

> Please note that a fonts folder is also generated with all the fonts
> referenced in the HTML file. When you distribute or upload the HTML to a site,
> please ensure that the fonts folder is also placed on the server in the same
> folder as the `.html` file. <img src="../images/exports/015-html.jpg"
> width="40%"/>

<img src="../images/exports/016-html.jpg" width="100%"/>

At the moment it is not possible to import a HTML file in Scrite.

> NOTE: While Scrite encodes formatting options in the generated HTML file, you
> may notice formatting errors when the file is opened in certain browsers.

## Persistent Export Options

Checkable options in each of the export dialog boxes are persistent across
multiple use and sessions. This means if you have checked the `Generate title
page` option in [PDF Export](#adobe-pdf) dialog box, it will remain checked in
all future use of the same. This is done to save you the burden of having to
configure export options each time.

File name, comment and watermark texts however are not persistent.