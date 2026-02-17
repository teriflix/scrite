# File Management

The basics file operations in Scrite have already been described in the [Quick
Start](./quickstart.md) and [User Interface](./user-interface.md) chapters. In
this chapter will look at a few nuances.

## Shortcuts

The following table lists all shortcuts related to file operations in Scrite.

|Shortcut|Description|
|--------|-----------|
|`Ctrl+O`|*Open File*: Launches [Home Screen](./quickstart.md#home-screen). You can open a file from [Scriptalay](./scriptalay.md), [Recent Files](#recent-files) list or from the disk|
|`Ctrl+N`|*New File*: Launches [Home Screen](./quickstart.md#home-screen). You can start a new file from a [template](./templates.md), or [clipboard](./imports.md#new-from-clipboard).|
|`Ctrl+Shift+O`|*Scriptalay*: Launches [Home Screen](./quickstart.md#home-screen) and switches to the [Scriptalay](./scriptalay.md) page if its not already visible.|
|`Ctrl+S`|*Save File*: Saves the currently open file. If the file is being saved for the first time, a file dialog is displayed to ask for folder & file name.|
|`Ctrl+Shift+S`|*Save As*: Always shows a file dialog box to ask for a new name to save the file|
|`Ctrl+W`|*Close File*: Closes the current file and loads a blank document with one empty scene|

> NOTE: Replace `Ctrl` with `Cmd` on macOS.

## Command Center

Each of the actions listed above can also be accessed from the [Command
Center](./user-interface.md#command-center). Additionally, there are a few more
options you can lookup in the command center by typing one or more of the
following keywords.

|Keyword|Description|
|------|-----------|
|Protect Document|Allow edits on this document by only a few people, more about this in the [Shield](#shield) section.|
|Import|Import a screenplay from Fountain, Final Draft or Plain Text files.|
|Open Backup Copy|[Load a backup](#loading-a-backup) of the currently open file.|
|Recover|Recover an unsaved document from the [vault](#vault).|
|Recent...|Just typing Recent in the command center lists up to 10 recently opened files, and you can quickly select from them. Alternatively, you can also type author name or a couple of words from the title or logline in the command center, and it will shortlist relavent options for you from Scriptalay, Templates and Recent Files|

> NOTE: Aside from recent files, you can assign a shortcut of your choice to
> each of the above commands.

## Recent Files

Scrite lists up to 10 recent files in the [Home
Screen](./quickstart.md#home-screen). You can hover over any of the recently
opened files to see the cover page, logline and authors. Simply click on a file
you want and have Scrite open it. It's that simple.

To configure the contents of the recent files list, click on the edit icon.

<img src="../images/files/001-recent-files.jpg" width="85%"/>

In the resulting dialog box, you can remove one or more files from the list.

<img src="../images/files/002-recent-files.jpg" width="85%"/>

## Auto Save

Scrite auto‑saves documents every 60 seconds by default. Auto‑save kicks in only
after you save your work to a `.scrite` file once. Scrite creates a folder
alongside your `.scrite` file and stores backups periodically.

You can toggle auto‑save, change the interval, and limit the number of backups
in Settings → Application → Options.

<img src="../images/files/003-auto-save.jpg" width="100%"/>

## Loading a Backup

Everytime a file is saved either manually, or as a result of [auto
save](#auto-save), Scrite stores a backup. Up to 15 backups are saved by default
for each file, however you can configure that to any other value you like.

<img src="../images/files/004-loading-a-backup.jpg" width="100%"/>

Whenver you open a file the toolbar shows an icon for loading a backup copy of
that file.

<img src="../images/files/005-loading-a-backup.jpg" width="50%"/>

Click it to open a dialog and select a backup to load in a new or the current
window. 

<img src="../images/files/006-loading-a-backup.jpg" width="85%"/>

> NOTE: Backups load anonymously—you must save them explicitly with a new file
name, and they won’t overwrite the source file.

## Vault

Did you ever face a situation where you wrote something, but forgot to save it?
Worry not - happens to the best of writers! Scrite keeps a copy of your unsaved
files in the vault from which you can recover. 

The vault can be accessed from the [Home Screen](./quickstart.md#home-screen) or
by typing Recover in the [Command Center](#command-center).

<img src="../images/files/007-vault.jpg" width="100%"/>

Files recovered from the vault are opened anonymously and have to be saved to
disk explicitly.

> NOTE: Vault only keeps a copy of files that have *not* been saved even once. 

## Lock File
Scrite creates a .lock file next to your document every time it is opened. This
is done to ensure that a second instance of Scrite doesn’t open the same
document and cause read-write-locks, or worse corrupt your document.

This .lock file is automatically deleted when you close the Scrite app or open
another document in it. In odd cases where the .lock file was not removed you
will see an error message like this:

<img src="../images/files/012-lockfile.jpg" width="65%"/>

As the dialog box suggests,

- First verify that there are no other instances of Scrite either in your
  computer or elsewhere (over the network) where the document in question is
  opened.
- Click “Ok” on the dialog box.
- Scrite would have launched File Explorer (on Windows), Finder (on macOS) or
  File Manager (on Linux) where the lock file in question can be found. If you
  dont see it, check if they are opened in a background window. Go ahead and
  delete the .lock file manually.
- You can then come back and reopen the document and it should work fine.

*If you see this message all the time*, then its possible that Scrite isn’t
closing properly or that you are saving directly on a cloud-synced folder and
the syncing of files across devices is not working properly. Try saving your
Scrite file on a local folder and check again.

## Shield

By default Scrite stores screenplays in an interoperable format as described in
this [page here](https://www.scrite.io/interoperable-document-format/). This
means anybody can unpack the contents of the Scrite document and further process
its contents. This ensures easy integration with other tools, and even custom
tools written by studios and independent software developers.

If you want to prohibit anybody from opening your Scrite document, and ensure
that it opens only on those systems where your email-id is used to activate the
Scrite installation, then you can use the Sheild feature to lock your
screenplays.

Click on the shield icon on the bottom left corner of the screenplay editor.

<img src="../images/files/008-shield.jpg" width="60%"/>

This launches a dialog box where you can toggle protection, to lock the
screenplay so it can only be opened on devices where Scrite is activated using
your email-id.

<img src="../images/files/009-shield.png" width="100%"/>

Optionally, you can add email ids of one or more collaborators who you want to
hand-pick and permit opening the file. This means that the file can now be
opened on devices with Scrite activated using either your email-id or any one of
your collaborator’s email ids.

> Note: Adding a collaborator here doesn't mean Scrite will automatically share
> the screenplay with your collaborators. You will still have to explicitly
> share the file with them via email or cloud storage services like Google
> Drive, OneDrive etc. Please also note that collaborators added thus cannot
> edit the file in real time, since [we do not support
> that](#no-real-time-collaboration) just yet.

If anybody else attempts to open the screenplay, they will see this message.

<img src="../images/files/010-shield.png" width="60%"/>

Once locked, the shield icon on the status bar shows up as checked.

<img src="../images/files/011-shield.png" width="85%"/>

At any time, you can turn off Shield protection for your files by going back to
the shield dialog box.

Please note that your collaborators will not be able to turn off shield, or add
additional collaborators. That right rests only with you.

> NOTE: By turning on Shield, contents of your screenplay, notebook and
> structure tabs will be encrypted. However, photos and attachments in notebook
> and the title page will not be encrypted. In a future version we hope to
> encrypt them as well. Please be aware that while our encryption keys are not
> public, we do not guarantee that the encryption mechanism is hack-proof. It is
> possible that someone may be able to decrypt the contents of your file, but we
> imagine that it isn’t too easy to do so.

## No Real-Time Collaboration

Scrite does not offer real time collaborative editing yet. We hope to build
support for that in a future update.