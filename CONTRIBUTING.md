# CONTRIBUTING

If you find an issue that interests you, please leave a note asking
about it first. If you don't see that anyone has inquired about it, you
can simply leave a note saying you are going to work on it. If people
work on an issue without saying they are working on it, the result is
sometimes three people submit a PR for the same issue.

If you find a problem for which no ticket has yet been created, please
don't hesitate to open a new ticket, and let us know if you are going
to work on that issue.

Please leave another note if you change your mind or if you get busy
with other things and are unable to finish it. That lets me and other
people know the ticket is available to be worked on by other people.

## Coding style

The goal is [GNU style formatting][GNU_style_guide]

Sometimes a patch will be a single line in a single file; other times a
single from each other (i.e. a separate PR for each patch).

## Web site

Most of the files for the [web site][hldig_website] are in
[docs/infiles][site_infiles] and can be changed with a pull request
through GitHub. To preview your changes, you can use
docs/src/simplecgen to generate new html files in docs/

Thank you!

## Pull Requests

1. Join the [##hldig channel on Freenode
IRC[(http://webchat.freenode.net/?channels=%23%23hldig&uio=d4), so if
any problems arise, you can get support.

2. [Fork the repo][fork] (if you haven't already done so)

3. Clone it to your computer

4. When you're ready to work on an issue, be sure you're on the
**master** branch. From there, [create a separate branch][sep_branch]
(e.g. issue_32)

5. Make your changes. If you're unsure of some details while you're
making edits, you can discuss them on the ticket.

6. If you are a first time contributor, please add your name and link
of your choice to [AUTHORS.md][authors].

6. Commit your changes. [git-cola][git-cola] is a nice GUI front-end
for adding files and entering commit messages (git-cola is probably
available from your OS repository).

7. Push the working branch (e.g. issue_32) to your remote fork and make
your [pull request][PR].
    * Do not merge it with the master branch on your fork. That would
    result in multiple, or unrelated patches being included in a single
    PR.

8. If any further changes need to be made, comments will be made on the
pull request.

It's possible to work on two or more different patches (and therefore
multiple branches) at one time, but it's recommended that beginners
only work on one patch at a time.

### Syncing ###

Periodically, especially before starting a new patch, you'll need the sync your
repo with the remote upstream. GitHub has instructions for doing this:

1. [Configuring a remote for a fork](https://help.github.com/articles/configuring-a-remote-for-a-fork/)
    * For step 3 on that page, use https://github.com/andy5995/hldig
    for the URL.

2. [Syncing a Fork](https://help.github.com/articles/syncing-a-fork/)
    * On that page, it shows how to merge the **master** branch (steps
    4 & 5 of **Syncing a Fork**).

[GNU_style_guide]: https://www.gnu.org/prep/standards/html_node/Formatting.html
[hldig_website]: https://andy5995.github.io/hldig/
[site_infiles]: https://github.com/andy5995/hldig/blob/master/docs/infiles
[authors]: https://github.com/andy5995/hldig/blob/master/AUTHORS.md
[git-cola]: https://git-cola.github.io/
[fork]: https://github.com/andy5995/hldig/fork
[sep_branch]: https://github.com/Kunena/Kunena-Forum/wiki/Create-a-new-branch-with-git-and-manage-branches
[PR]: https://help.github.com/articles/creating-a-pull-request-from-a-fork/
