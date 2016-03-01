How to Contribute
=================

Being an open source software company, we love contributions from
outside our organization. But getting to know a projects way's is
never easy and can often be daunting.  That is why we have written
this.  The hope is that all the information you need to successfully
contribute to this project is right here.  If something is missing or
inaccurate, let us know and we will be more than happy to correct it.

# Opening Pull Requests

So... you think you have what it takes eh?  Awesome.  We would be more
than happy to consider your code for inclusion to the project (and
give you credit for it).  But to keep things sane, we have to follow
rules.  These are outlined below.  Please read them.

## Coding Standard

We have a [coding standards document](/CODING_STANDARD.md) in this
repository.  Please read it and follow it.  Yes you may not agree with
everything it says/does (like the use of tabs) but that is the way this
project is and we follow it strictly.  Please adhere to it or your patches
may be rejected.

## Branching

We use a paradigm similar to [Vincent Driessen's branching model](
http://nvie.com/posts/a-successful-git-branching-model) for our
branches.  But rather than maintain a devel branch, we simply
contribute directly towards branches that represent the next release.
Sounds easy, and it mostly is, but there is a catch.  Since we use a
triple tiered versioning strategy (X.Y.Z) you must first understand our
release types.  Understanding that will help you pick the correct branch
to base your changes against.

## Release Types

As previously mentioned, we have three types of releases.  They are as
follows:

### Bugfix Releases (Z Releases)
These are the most common and likely what you should be patching
against. Important notes are as follows:

* No new features.
* Enhancements to existing features are considered.  Depends on
   invasiveness.
* No changes to the LoggerConfig ABI allowed here (no modifying the
   struct layout).

### Features Releases (Y Releases)
These types of releases are less common than Bugfix Releases, but do
happen somewhat frequently. Any change that can go into a Bugfix
Release may also be applied here if you so choose.  Important notes
are as follows:

* New Features go here.
* Changes to the LoggerConfig ABI are allowed.

### Backwards Incompatible Releases (X Releases)
These releases are the most uncommon type of release and happen very
infrequently.  Their intention is to have a type of release where we
can make changes that are backwards incompatible and be able to see
that in our Apps and Race Capture Live platform.  Any changes that
apply towards a Feature Release can also be applied here if you so
choose.  Important notes are as follows:

* Saved for major changes for things like API changes.
* Implies that associated Apps must be changed to support this new
   version.

This release type may disappear in the future due to non-use and the
introduction of the API version under the hood.

## Choosing the Correct Branch to Rebase Your Patch Against

Now that you know what type of release type your patch applies to, the
next step is to rebase your patch set against the latest version of
that release branch.  By doing this, you will reduce if not eliminate
the chances of a merge conflict when presenting your PR.

To do this correctly, you need to be able to locate the correct
branch.  All of our release branchs live under the /r/ area of our
tree.  Branch names look like this:

* /r/2.8.7
* /r/2.8.8
* /r/2.9.0

Based on what you now know about release types, picking the correct
branch should be easy.  Once you know the correct branch, its time to
rebase your commits.  If you are not familiar with [git
rebase](https://git-scm.com/docs/git-rebase), now is the time to learn
about it before proceeding.  Its a great tool.

Rebasing is done using the following steps from the command line:

* Add our remote if you haven't already.  Here is the command to help
  you out:
  <br><br>
  ```
  $ git remote add rcp_firmware https://github.com/autosportlabs/RaceCapture-Pro_firmware.git
  ```
  <br><br>
  This would add a remote named `rcp_firmeware` that points
  to our public repo.
* Ensure your local git tree has up to date information about our git
  tree by running a fetch against our repository.  This is done using
  the git fetch command.  Here is an example using the `rcp_firmware`
  remote as the target:
  <br><br>
  ```
  $ git fetch rcp_firmware
  ```
  <br><br>
  Obviously your remote name may be different.  Adjust accordingly.
* Rebase your branch against the appropriate release branch.  Assuming
  that your branch is named `my_feature` and the release branch is
  named `/r/2.8.8` in the remote `rcp_firmware`, here is what the
  command would look like:
  <br><br>
  ```
  $ git checkout my_feature
  $ git rebase rcp_firmware/r/2.8.8
  ```
  <br><br>
  That should get you rebased against the latest version of the 2.8.8
  branch.  Enssure you have followed all of git's instructions during
  this process.  Remember that if things get too hairy... you can
  always abort the rebase using the `git rebase --abort` command.

## Building and Testing

Hopefully this is the easiest part of this whole task.  It is assumed
that you have a build tree setup.  If you don't, check out the
[README](/README.md) to figure out how to set one up.  In the top
level directory there is a script named `do_release.sh`.  Run it from
within the top level directory.  It will do the same set of tests that
our build bot will do.  If it passes, you have a 99% chance of success
against the bot.  If it doesn't... then there is a bug.  Fix it and
try again.

## Opening the Pull Request

Whew... that was a fair bit right?  Almost there.  Use this checklist
to ensure you have done it all correctly before you open a PR:

* DID YOU READ AND FOLLOW THE ABOVE NOTES.  Yes all of them.  If you
  didn't, then there is a good chance your PR will get NAK'd. Please
  read and follow them.
* Ensure your code follows our coding standard.
* Ensure you code is based off of the correct branch.
* Build & Test your code locally and ensure it compiles before posting.
  Nothing sucks more than to have our Jenkins bot NAK your PR
  automatically because it fails to build.

Done all that?  Cool.  Then you should be good to go.  Open the PR
against our project and ensure you open it against the correct branch
on the GitHub site.  One of the maintainers of the project will review
your code shortly.
