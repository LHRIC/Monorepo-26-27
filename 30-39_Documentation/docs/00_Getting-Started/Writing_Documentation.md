# Writing Documentation

Whenever you start a new project, library, driver, etc. you should create a new
folder to house it's documentation. Even if it is just a simple few sentences
describing what it is, it makes it much easier for others to learn more about
what you're working on.

Writing documentation is as simple as writing some Markdown, which if you're not
familiar with you can check [this guide](https://www.markdownguide.org/); or
check the source for this page or any of the other pages. There are a few quirks
to the system that we're using but most of them can be ignored unless you want
your docs to look super fancy.

## Creating a New Folder

When you start your project it will be in either the Projects, Drivers, Tools,
or Libraries Area of the repository ([Repository Structure](./Navigating_the_Repository.md)).
These correspond with the same folders in the documentation directory structure.
Simply go to the Area that your project is in and create a new folder for your
project as well as an `index.md` file in that folder. This file serves as index
page for your documenation where you can put your overview.

!!! example
    This is what the directory structure would look like if I made a new Project
    in the area Tools called cowsay:
    ``` txt
    .
    ├── docs
    │   ├── 05_Tools
    │   │   ├── Cowsay
    │   │   │   └── index.md
    │   │   └── index.md
    │   └── index.md
    ├── includes
    │   └── abbreviations.md
    ├── justfile
    ├── mkdocs.yml
    └── yaml.schemas
    ```

## Previewing

Before you push any documenation related materials it's a good idea to preview
it first. You can do this by running the following command from witin the "40-49
Documentation" folder.

```sh
just preview
```

This uses docker to build your documentation and host it on `localhost:8000`.
Make sure that you have docker installed, your user is in the docker group and
that you have run this command,

```sh
docker pull squidfunk/mkdocs-material
```

to pull the docker image for mkdocs-material.

## Writing Docs

After setting up the folder and index file for your new project you can go ahead
and begin writing up your one pager. This doesn't have to be anything fancy,
just enough so that if someone checks out your project, they can easily
understand it's purpose and a little bit about how it works. Any additional
documentation can be added as subdirectories of the folder you just set up.
These subpages are where you can go more in depth on how your project works.

## Tips

### Admontitions

You can make cool blocks like this:

!!! tip "Isn't this cool?"
    I have an interesting tip!

by using the following syntax:

```md
!!! tip "Title" 
    content

```

You can learn more [here](https://squidfunk.github.io/mkdocs-material/reference/admonitions/).

### Code Blocks

You can make a code block with the following syntax:

```md
    ```language
    int main() {
        // some code
    }
    ```

```

Where `language` will enable syntax highlighting for that language.
