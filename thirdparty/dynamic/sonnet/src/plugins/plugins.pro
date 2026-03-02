TEMPLATE = subdirs
CONFIG += ordered

macx: SUBDIRS = nsspellchecker
win32: SUBDIRS = ispellchecker
linux: SUBDIRS = hunspell
