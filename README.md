# LHRIC Monorepo 26/27

This is the Monorepo containing all of the code for the LHRIC 26/27 season.
Documentation is hosted at a GitHub pages site for this repository. If you're
looking for a broad overview of projects look in the confluence and if you're
looking for specific details about implementations, look at the GitHub pages docs.

## Organization Scheme

This monorepo is organized using a system of Cabinets, Drawers, and Folders.
When directing someone to a specific folder all you need is these digits:

01.00

This is the "address" to find the folder on ESP DAQ it can be broken up like this:

- 00-09: The Cabinet that contains Drawers 00-09. This is where projects go.
- 01: The Drawer containing all projects related to telemetry.
- 01.00: A File containing the ESP DAQ telemetry project.

The numbers are nice because they make it easy to reference folders and they
make it so that folders never move around. They're put first in the name so that
your operating system organizes them using the numbers; if they weren't there
whenever someone makes a new project you're project might move to a different spot.

## Guide to Contributing

Open a new branch whenever you make changes; don't commit directly to main. Work
on your project and then whenever you're done submit a pull request. Someone
will review your work and then you can merge. Make sure you only work in one
Folder so that whenever your work is merged it doesn't cause any conflicts.

## Guide to Documentation

Whenever you start a new project you need to document it. Make sure you do the
following things:

- Make a page in the docs on GitHub and give a summary, doesn't need to be long.
- Make a page in the confluence, can be a mirror of the one in GitHub, and then
  link to the GitHub page.
- From now on any more in depth documentation can be put into the GitHub docs.

Don't worry about documenting every single thing you do, in general just try to
make your code explain itself and add comments if it doesn't. If you ever come
up with a system in your code that requires some extra explanation, say you
added something to ESP DAQ that uses a complex custom data structure, then
that's the time you should make some documentation. In general, if someone ever
asks "Hey what does this do?" when they're reviewing your pull request you
should probably write some documentation for that.

