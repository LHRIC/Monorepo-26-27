# Navigating the Repository

The structure of this repository is, for the most part, based of of [this
system](https://johnnydecimal.com/documentation/introduction). The main reason for all of the numbers
is so that it is easier to guide someone to a folder if you are describing what
you are working on. Each folder gets its own ID and you can easily tell someone
where you're working by telling them the ID.

The ID's can be broken down like this:

- Areas: The first digit in an ID this is for broad categories: Projects,
  Libraries, Tools, etc.
- Categories: The second digit in an ID, for specific categories. For Example the
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

## Current Areas

Here's a description of what should go into each of the currently made areas.
Make sure to update this if you add any other areas in the future.

- 00-09 Projects: This is for anything that will actually end up going on the
  car, as well as projects we work on for things like recruiting.
    - 00 Power Management: This one's pretty self explanatory. Also includes BSPD.
    - 01 Telemetry: This is for anything that consumes data from the CAN bus and
      then uses it to do something. This includes DAQ and things like Custom
      Display.
    - 02 Sensors: These are for sensor boards. Generic CAN boards are given ID's
      above 50.
    - 03 Powertrain: This is for things that interact with the powertrain on the
      car. If it goes near the engine, it's probably a powertrain project.
    - 04 Recruiting: Things for trial workday.
- 10-19 Libraries: This is for anything that is generally reusable, or would
  nice to be reused in the future.
  - 11 Drivers: Drivers for different IC's and Sensors
  - 12 CAN: This has the CAN DBC and some libraries to help you do CAN things
- 20-29 Tools: These are things that help us make the car, but generally will
  never go on the car. They are also more likely to be used by people outside of
  electronics, so make sure your documentation is good.
  - 20 Electronics Testing: This is things like breakout boards and HWIL.
  - 21 User Applications: These are apps we've made to help out with various
    tasks around the garage.
- 99.99 Documentation: Documentation get's its own thing cause its special :).
