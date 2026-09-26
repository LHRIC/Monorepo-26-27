# Git Sparse Checkout

Git Sparse Checkout is a way of picking specific files to pull from a repository
instead of pulling everything all at once. Though I would recommend you to clone 
the whole Monorepo, you may want to check out specific files in certain
instances. This is how you do it.

## Cloning

Normally git will pull all of the files within a repository whenever you clone
it. If you want to use sparse checkout, you need to tell git not to checkout any
files when cloning, and additionally to not pull any files that aren't checked
out. This can be done with the following command:

```console
$ git clone --filter=blob:none --no-checkout git@github.com:LHRIC/Monorepo-26-27.git
```

This will pull in the information that git needs, and create a new directory on
your system to hold the repository. However, it will not pull any files or other
directories. Next you can start sparse checkout by running the following command:

```console
$ cd Monorepo-26-27/
$ git sparse-checkout init --cone
$ git checkout main
```

This will set up the repository so that it uses sparse-checkout[^1], and then it
checks out the main branch. You should now have all the files that exist in the
root of the Monorepo, but none of the directories. You're now ready to use
sparse checkout!

```txt
.
├── justfile
└── README.md
```

## Picking Folders

Sparse checkout—specifically in cone mode—works by allowing you to pick specific
directories that you care about, and only pull those directories and all the
files and directories that are immediately contained in that directory. That
might sound somewhat complicated, but it ends up working really well in the
Monorepo topology.

Say that I want to pull just the Custom IC Project with ID: 04.01. I can use the
following command:

```console
$ git sparse-checkout add 00-09_Projects/04_Recruiting/04.01_CustomIC
```

Git will do all of the work for me, and begin pulling the files that I'm
missing. At the end, you should have the following directory structure:

```txt
.
├── 00-09_Projects
│   └── 04_Recruiting
│       └── 04.01_CustomIC
│           ├── doc
│           │   ├── assets
│           │   │   └── sawcon_logo.png
│           │   ├── SEA-DADSD67.typ
│           │   └── tids.typ
│           ├── fuses.conf
│           ├── include
│           │   ├── delay.h
│           │   ├── rc.h
│           │   └── serial.h
│           ├── justfile
│           ├── makefile
│           ├── src
│           │   ├── delay.c
│           │   ├── main.c
│           │   ├── rc.c
│           │   └── serial.c
│           └── starter_code
│               ├── include
│               │   └── README
│               ├── lib
│               │   └── README
│               ├── platformio.ini
│               ├── src
│               │   └── main.cpp
│               └── test
│                   └── README
├── justfile
└── README.md
```

I now have all of the files and folders that I need to work on the Custom IC
project! You can then follow the same process for pulling in any projects that
you want to work on. Additionally, say that you only wanted to work on power
management for some reason, you can pull all of the power management projects by
running the following command:

```console
$ git sparse-checkout add 00-09/Projects/00_PowerManagement
```

And git will pull in all of the power management projects! The same goes for any
directory that you want to pull in, and because of how the Monorepo is
structured, it should be pretty easy to get the things you want.

## Some Caveats

While sparse checkout is great, there are a few reasons I wouldn't recommend it:

- It somewhat destroys the point of the Monorepo: The reason we switched over to
  a monorepo is because I wanted it to be easier to find projects and for new
  people to get everything they need in one command. Sparse checkout goes
  against that.
- Missing Dependencies: Some of the projects rely on things from the library
  folder, for this reason I recommend you always run this:
  ```console
  $ git sparse-checkout add 10-19_Libraries
  ```
  To pull everything from the libraries folder in case your project needs it.

  [^1]: For most purposes you should only ever use cone mode. Cone mode matches
  entire directories only, whereas non-cone mode will allow you to match
  individual files. Generally, it is easier for things to break whenever you use
  non-cone mode, so it is recommended to always use cone mode.
