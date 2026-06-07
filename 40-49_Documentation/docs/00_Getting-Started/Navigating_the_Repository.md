# Navigating the Repository

The structure of this repository is, for the most part, based of of [this
system](https://johnnydecimal.com/documentation/introduction). The main reason for all of the numbers
is so that it is easier to guide someone to a folder if you are describing what
you are working on. Each folder gets its own ID and you can easily tell someone
where you're working by telling them the ID.

The ID's can be broken down like this:

- Areas: The first digit in an ID this is for broad categories: Projects,
  Libraries, Tools, etc.
- Categories: The second digit in an ID, for specific categores. For Example the
  Projects Area has categories: Telemetry and Sensors.
- ID's: This is the part after the '.' in a full ID, these point to the actual
  folder that is being navigated to. These are given in order and shouldn't be
  changed, as to avoid confusion.

## Example

Say I'm working on ESP-DAQ and I want Kamran to help me out with a feature I'm
implementing. I can tell him it's at 01.00 and this tells him:

- 00: It is in the Projects Area.
- 01: It is in the Telemetry Category.
- 01.00: It is folder 01.00.

While I could've told Kamran that I'm working on ESP-DAQ and he likely would've
found it by looking at the names of the directories, by telling him the ID he
can stop thinkig and simply click till he gets to the ID. This avoids Kamran
confusing ESP-DAQ for being something other than a project as he navigates the directories.
