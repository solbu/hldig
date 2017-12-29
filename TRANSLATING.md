Translators are encouraged to join the [chat
room](https://join.slack.com/t/htdig/shared_invite/enQtMjY3NDU1MjMwODk3LTdmM2I2OWI5NWI4MzU4Y2JmMjk2MzAxNDYzM2IzZjJmMGE2MDZmMWMxNDY3MjAwOGFjMmE1YjM2MmM4MzVkNzk).
We will try to help and answer any questions you may have about the
translation process.

## output messages

The process for translating the output messages is a little different,
but not very difficult. There are some [po editing
utilities](https://www.gnu.org/software/trans-coord/manual/web-trans/html_node/PO-Editors.html)
that can be helpful.

If you have experience translating files in a [po
format](https://www.gnu.org/software/gettext/manual/html_node/PO-Files.html),
then you will be able to skip most of the instructions.

A sample po file (from the GNU coreutils `cp` command)
```
msgid "write error"
msgstr "Schreibfehler"

#: lib/copy-acl.c:54 src/copy.c:1387 src/copy.c:2863
#, c-format
msgid "preserving permissions for %s"
msgstr "Erhalten der Zugriffsrechte für %s"

#: lib/error.c:191
msgid "Unknown system error"
msgstr "Unbekannter Systemfehler"

#: lib/file-type.c:40
msgid "regular empty file"
msgstr "reguläre leere Datei"

#: lib/file-type.c:40
msgid "regular file"
msgstr "reguläre Datei"
```

* You will need the `gettext` package from your OS distribution.
* Follow the steps for [pull requests](https://github.com/andy5995/hldig/blob/master/CONTRIBUTING.md#pull-requests) to start working on patch
* After you clone your fork, go into the hldig source po/ directory
* run `msginit -i hldig.pot -o ??.po -l ??` (Where ?? is your
 [iso-631-1 language code](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes))
* At this point, you may open your new .po file and begin adding translations
* When done, make a [pull request](https://github.com/andy5995/hldig/blob/master/CONTRIBUTING.md#pull-requests)
* Thank you!
