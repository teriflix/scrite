# Language Support

Many screenplays today are written in multiple languages. For instance, in
India, dialogues are written in Kannada, Hindi, Marathi, Tamil, Telugu, and
other languages, while other parts of the screenplay are written in English.
Writers from the Malayalam film industry write predominantly in their native
language. Writers in France, Vietnam, Korea, and other countries also prefer to
write in their own languages. Therefore, it is important for a screenwriting app
to support writing in multiple languages, even if the UI language (menu items,
dialog boxes, etc.) is English only.

When we started the Scrite project, it was aimed at being a screenwriting app
primarily for the Indian writer. Initially, the app only supported writing in
English and 11 Indian languages. With version 2, Scrite has evolved to cater to
international writers as well. You can now write in any LTR language. In this
chapter, we walk you through the language support available in Scrite and point
out several nuances.

## Language Menu

On the toolbar of the Scrite main-window an icon associated with the active
language is shown, alongside the language name itself. Clicking on the icon
reveals a language menu in which all languages configured for use in Scrite are
displayed.

<img src="../images/language-support/001-language-menu.jpg" width="60%"/>

Depending on how your computer is configured before Scrite is installed, this
menu shows a different set of languages.

- Users in India may see English and 11 Indian languages listed in this menu.
- If users have already configured one or more language input methods in their
  OS, this menu will only list English and the languages configured.
- International users will see English, and their OS language and any other
  input methods they may have configured.

## Language Settings Dialog

You can click on `Language Settings` option in the language menu to add, remove
or configure languages of your choice.

<img src="../images/language-support/002-language-settings-dialog.jpg"
width="85%"/>

The currently configured languages are listed on the left. You can click on any
of the languages in that list to configure some options for them on the panel to
the right.

### Removing Languages

Click on the delete icon next to any language to remove it from Scrite. It's
that simple!

> NOTE: You cannot remove English language from the list because its the default
> UI language.

<img src="../images/language-support/003-removing-languages.jpg" width="40%"/>

### Adding Languages

To add support for a new language, simply click on the `Add Language` button at
the end of the list.

<img src="../images/language-support/004-add-language.jpg" width="40%"/>

Scrite presents a dialog box for you where you can type name of the language you
want to add.

<img src="../images/language-support/005-add-language.jpg" width="85%"/>

> NOTE: [RTL
> languages](https://en.wikipedia.org/wiki/Category:Right-to-left_writing_systems)
> are currently not supported in Scrite. We will be building support for them in
> a future version.

### Spell Check

Version 1.x only offered spell-checking in English. With version 2, we've
expanded this capability to support more languages. Please note that Scrite does
not bundle dictionaries into the application. Instead, it uses dictionaries
provided by your operating system to check spellings. The Language Settings
dialog box indicates whether a dictionary is available for a given language.

<img src="../images/language-support/006-add-language.jpg" width="85%"/>

When a dictionary is available, Scrite automatically uses it to offer spelling
suggestions.

> NOTE: In a future update, we plan to offer our own spell-check dictionaries
> through services such as [Alar](https://alar.ink), [Olam](https://olam.in),
> and others for select languages.

### Keyboard Shortcuts

You can assign or change keyboard shortcut for any language by clicking on the
shortcut link in the `Keyboard Shortcut` groupbox. If there was no shortcut
assigned, then the shortcut link will report `None Set`. 

<img src="../images/language-support/006-keyboard-shortcut.jpg" width="85%"/>

In the `Change Shortcut` dialog you can assign a keyboard shortcut of your
choice.

<img src="../images/language-support/007-keyboard-shortcut.jpg" width="85%"/>

Once a shortcut has been assigned, it will also be displayed in the language
menu.

<img src="../images/language-support/008-keyboard-shortcut.jpg" width="60%"/>

> NOTE: English and 11 Indian languages have hard coded keyboard shortcuts.
These are shortcuts that have historically been associated with those languages,
and Scrite doesn't allow altering them. You can however change keyboard
shortcuts for any other language.

### Font Association

Every language has a script associated with it. 

- For instance: English & French languages share the Latin script. 
- Hindi, Marathi and Sanskrit share Devanagiri script. 
- Kannada, Tamil, Telugu and Malayalam have their own unique scripts. 

You can associate any font to a language and have that font be used by other
languages that use the same script. 

<img src="../images/language-support/009-font-association.jpg" width="85%"/>

Once assigned all fonts in of a given script type will be rendered as
configured. This takes away the burden of having to select specific text
snippets and assign fonts separately.

<img src="../images/language-support/010-font-association.jpg" width="100%"/>

### Input Method Mapping

While Scrite bundles a [static phonetic
translation](https://phtranslator.sourceforge.net) input method for the 11
Indian languages it supports, it falls back to the default OS input method for
all other languages. 

[Windows](https://support.microsoft.com/en-us/windows/set-up-and-use-indic-phonetic-keyboards-7c4d2e8a-abf2-f200-9866-1a4cead7b127)
and [macOS](https://support.apple.com/en-in/guide/mac-help/mchl84525d76/mac)
offer standard mechanisms for registering input methods for various languages.
On Linux, however, you will have to [install and configure
ibus](https://help.ubuntu.com/community/ibus).

Once you have configured input methods on your OS you can return or restart
Scrite to have it automatically register and map input methods for those
languages.

<img src="../images/language-support/010-input-methods.jpg" width="100%"/>

Whenever you switch languages either using either the language menu within
Scrite, or by using the keyboard shortcuts assigned to them, Scrite toggles the
corresponding OS input method for you. 

<img src="../images/language-support/011-input-methods.jpg" width="85%"/>

If you prefer using a different input method for a specific language, then you
can configure that in the `Language Settings` dialog box by unchecking `Auto
Select` and picking a specific input method.

<img src="../images/language-support/012-input-methods.jpg" width="85%"/>

From then on, Scrite will use the assigned input method while typing in that
language.

<img src="../images/language-support/013-input-methods.jpg" width="60%"/>

Whenever the built-in PhTranslator is assigned to any of the Indian languages,
Scrite also displays an icon on the toolbar for reviewing the alphabet mapping
table.

<img src="../images/language-support/014-input-methods.jpg" width="100%"/>

This alphabet mapping table shows the accurate combination of English alphabets
to use.

> NOTE: By default Scrite insists on handling language input switch all by
> itself, unless you configure the app to [delegate it entirely to the
> OS](#letting-os-manage-input-method-switching).

### More input methods coming up!

In a future update we will be adding native support for
[Nudi](https://nudityping.com), [Baraha](https://baraha.com/main.php),
[ISMv6](https://ismv6.com/ism-v6-software-download-for-windows/),
[Alar](https://alar.ink), [Olam](https://olam.in) and custom [keyboard layout
files](https://www.microsoft.com/en-us/download/details.aspx?id=102134).

### Letting OS Manage Input Method Switching

Some users prefer to let their OS handle all input method switching because they
are used to that with other apps. We understand that habits formed over a long
time are difficult to change. In such cases, simply uncheck the `Handle language
input method switch` option in `Language Settings`.

<img src="../images/language-support/015-delegate-to-os.jpg" width="85%"/>

Scrite will no longer show you language switch menu, nor does it respond to
lanugage shortcuts within the app. All language switching will now have to be
managed by using menus and shortcuts configured with the operating system.
Scrite does however show a language icon in the toolbar to report the language
input method currently activated in the OS. [Font
associations](#font-association), however, will continue to work.

<img src="../images/language-support/016-delegate-to-os.jpg" width="85%"/>

When you click on the language toolbutton in the Scrite UI, a dialog box is
shown as follows.

<img src="../images/language-support/017-delegate-to-os.jpg" width="60%"/>

You can click `Yes` in this message box to open the `Language Settings` dialog
and toggle the `Handle language input method switch` option ON, and have Scrite
resume control of switching between input methods for languages.

