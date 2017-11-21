# CONTRIBUTING

If you find an issue that interests you, please leave a note asking about it
first. If you don't see that anyone has inquired about it, you can simply
leave a note saying you are going to work on it. If people work on an issue
without saying they are working on it, the result is sometimes three people
submit a PR for the same issue.

If you find a problem for which no ticket has yet been created, please don't
hesitate to open a new ticket, and let us know if you are going to work on
that issue.

Please leave another note if you change your mind or if you get busy with other
things and are unable to finish it. That lets me and other people know the
ticket is available to be worked on by other people.

## Coding style
The goal is [GNU style formatting](https://www.gnu.org/prep/standards/html_node/Formatting.html)

Sometimes a patch will be a single line in a single file; other times a single
patch will consist of changes to several files. Keep unrelated patches separate
from each other (i.e. a separate PR for each patch).

## Pull Requests
1. Join the [chat room](https://join.slack.com/t/htdig/shared_invite/enQtMjY3NDU1MjMwODk3LTdmM2I2OWI5NWI4MzU4Y2JmMjk2MzAxNDYzM2IzZjJmMGE2MDZmMWMxNDY3MjAwOGFjMmE1YjM2MmM4MzVkNzk)
so if any problems arise, you can get support.
2. [Fork the repo](https://github.com/andy5995/htdig#fork-destination-box) (if you haven't already done so)
3. Clone it to your computer
4. When you're ready to work on an issue, be sure you're on the **master** branch. From there,
[create a separate branch](https://github.com/Kunena/Kunena-Forum/wiki/Create-a-new-branch-with-git-and-manage-branches)
(e.g. issue_32)
4. Make your changes. If you're unsure of some details while you're making edits, you can
discuss them on the ticket.
5. Commit your changes
6. Push the working branch (e.g. issue_32) to your remote fork and make your
[pull request](https://help.github.com/articles/creating-a-pull-request-from-a-fork/)
    * Do not merge it with the master branch on your fork. That would result in multiple, or
    unrelated patches being included in a single PR.
7. If any further changes need to be made, comments will be made on the pull request.

It's possible to work on two or more different patches (and therefore multiple branches) at
one time, but it's recommended that beginners only work on one patch at a time.

### Syncing ###
Periodically, especially before starting a new patch, you'll need the sync your
repo with the remote upstream. GitHub has instructions for doing this:

1. [Configuring a remote for a fork](https://help.github.com/articles/configuring-a-remote-for-a-fork/)
    * For step 3 on that page, use https://github.com/andy5995/htdig for the URL.
2. [Syncing a Fork](https://help.github.com/articles/syncing-a-fork/)
    * On that page, it shows how to merge the **master** branch (steps 4 & 5 of **Syncing a Fork**).
