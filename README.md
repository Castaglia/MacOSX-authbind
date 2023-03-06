I needed to get the authbind utility, available from:

  http://www.chiark.greenend.org.uk/ucgi/~ian/git/authbind.git/

up and running on a Mac OSX box (specifically, Mac OSX 10.7 Lion).  I made
a few changes to the source code; most of the changes are in the Makefile.

To install this authbind on your Mac OSX, simply do:

    $ cd MacOSX-authbind
    $ make
    $ sudo make install

Then set up the authbind config files as one would normally do.

> Note: if you're using a Mac with Apple's M-series chips, before running `make`,
edit the `Makefile` by following the instructions [here](./Makefile#L50-53)

Changes
--------
Here's the list of changes I made, and why:
* `PRELOAD_VAR` defined to `DYLD_INSERT_LIBRARIES`, rather than to `LD_PRELOAD`
* Added setting of `DYLD_FORCE_FLAT_NAMESPACE` environment variable, as per
  `dyld(1)` man page recommendation.
* Added more logging to `stderr` of errors found in `libauthbind`, and the
  helper.

The original files are marked as `.orig`; this distribution also contains a
`patches/macosx/` directory with the diffs, for easier inspection.  Note that
I did not make the Makefile intelligent enough to Do The Right Thing(tm) for
Linux platforms; it is currently specific to Mac OSX.
