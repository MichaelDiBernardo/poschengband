----- How to install -----

--- LINUX
  Download and unpack the source archive.

  Make sure you have the appropriate development packages installed.
  For example, you might run (Ubuntu or Mint):
    $ sudo apt-get install autoconf gcc libc6-dev libncurses5-dev libx11-dev

  From the root of the source archive:
    $ sh autogen.sh
    $ ./configure
    $ make

  To install, you may need to elevate your credentials:
    $ su
    $ make install
    $ exit

  Then run poschengband as desired:
    $ poschengband -- -n<number of windows>  ## for normal ASCII graphics
  or
    $ poschengband -g -- -n<# of windows>    ## for 8x8 tile graphics 

  You can change game windows' font, location, and size, by environment 
  variables.

  Ex.
    $ set env ANGBAND_X11_FONT '-*-*-medium-r-normal--24-*-*-*-*-*-iso8859-1'
    $ poschengband -- -n

  Then font size will be changed.

  You can set ANGBAND_X11_FONT_n for specific window which have window number n.

  Location of windows are ANGBAND_X11_AT_X_n, and ANGBAND_X11_AT_Y_n.
  Size of windows are ANGBAND_X11_COLS_n, and ANGBAND_X11_ROWS_n.

  (Thanks to Nick McConnell for implementing and improving building under Linux/MAC!)

--- Windows

  Download the binary archive for Windows.  Unzip it to any location that you 
  will have full permissions and launch poschengband to play.

  To compile, download the poschengband source code and unzip. I currently use
  Visual C++ 2010 Express which is freely available. I tried and don't recommend
  the 2012 Express edition as the MS seems to have "broken" Intellisense once
  again. Open poschengband.sln and compile.


-----  Basic for Playing  ------

  I usually play the first few levels of Camelot immediately and then take the
  Thieve's Quest, both of which are accessible from your starting town of Outpost.
  After the Thieve's I usually accept the Warg quest and dive to L5 in Camelot.
  Beware, the Wargs can be difficult but if you succeed, you will usually be at
  least 10th level and strong enough to leave Outpost in search of greater 
  adventures.  Carry lots of food and fuel, for the journey can be a long one.  
  Since this world is very large, it is convenient to go into global map mode by 
  pressing the '<' key, and then return to local map mode by '>' key when you are 
  at the destination.  When in the wilderness, stay in safe grids such as roads 
  or plains; seas and lava fields are best avoided for a while.

  Many towns and dungeons await you; good ones to explore early on include 
  the orc caves and the Labyrinth.  To the northeast of the town of Morivant, 
  near the center of the map, is the dread dungeon of Angband.  It is home to the 
  Serpent of Chaos, whom you must kill in order to win the game.


---------  Commands  ----------- 

     Please read on-line help (Press the ? key in the game) for full
information of the game.  Here are only descriptions of basic commands.

     The following section lists most commands, and the less frequently used 
ones may be accessed by pressing the return key, or (if using the original 
keyset) the 'x' key, to bring up the command menu.  Almost any command may be 
chosen from the main menu.  "(R) Rest" can be performed by pushing 'R' (in 
uppercase).  "Look around (l/x)" can be performed by pushing 'l' in the Original 
keyset, and by pushing 'x' in the Rogue-like keyset.



  [Movement]

     Original Keyset Directions

              7  8  9
              4     6
              1  2  3

     Roguelike Keyset Directions

              y  k  u
              h     l
              b  j  n

  5/, Stay still
  <   Go up staircase or Enter global map mode in the wilderness.
  >   Go down staircase or return from global map mode in the wilderness
  Shift+direction   run
  Ctrl+direction   attack, open, close, or dig

  [Tools]
  r  Read a scroll
  q  Quaff a potion 
  u/Z  Use a staff
  a/z  Aim a wand
  z/a  Zap a rod
  E  Eat some food
  A  Activate an equipment
  F  Fuel your lantern/torch
  f/t  Fire a missile

  [Action]
  d  Drop an item
  g  Pick up items
  R  Rest for a period
  o  Open a door or chest
  s  Search for traps/doors
  k/Ctrl+d  Destroy an item

  [Magic]
  m  Cast a spell / use mental power
  G  Gain new spells/prayers
  U/O  Use bonus power
  b/P  Examine spells (choose each spell to get a description)

  [Equipment] 
  w  Wear/wield equipment
  t  Take off equipment
  F  Fuel your lantern/torch

  [Info] 
  x  Examine items in a store
  C  Character description
  I  Examine an *identified* item
  i  Inventory list
  l/x  Look around
  M  Full dungeon map
  ~  Display various information

  [Other]
  p  Command pets
  =  Set options
  $  Reload auto-picker preference file
  _  Edit auto-picker preference
  n/[  Repeat previous command
  @  Interact with macros

  [Help]
  ?  View the on-line help 
     (Press a-z to choose a section,  space key to go next page.)


--- Inscriptions

Below, only the new features in Hengband are explained.

- You can inscribe on objects with "@ma" or '@'+<any command letter>+<any tag>.
  Then, type these alphabet letter in 'm' command to cast from the spellbook.

- The inscriptions "@1" or '@'+<any tag number> make the objects able
  to be used by pressing <tag number> key in any command.  Any
  alphabet key cannot be used as a tag in this form.

- You can inscribe on *identified* items with "%" or "%all".
  Then, "%" or "%all" is automatically replaced by strings which describe the 
  item's abilities. 
  Example:  You inscribe with "%all" on a pair of gauntlets of power which 
  provide resistance to fire. The description will look like {St;Fi} which 
  means bonus to Strength and resistance to fire.
  If you inscribe it with "%", its description will look like {Fi} which 
  means its random added ability is resistance to fire.


---- Auto pickup/destroy

Hengband offers two different methods of auto-pickup/destroy.
- Powerful method
  Hengband offers a very powerful auto-pickup/auto-destroy system
  which you may customize.  Press '_' key to enter edit mode.  Please
  read the file "autopick_eng.txt" about the syntax.

- Easy method
  Since the powerful method is slightly difficult to use 
  effectively, there is another very easy option.  you can turn on 
  the destroy_items option and/or the always_pickup option to use 
  this method.  These options are located in (6) Easy Auto-Destroyer 
  Options and (1) Input Options in the Set options ('=') command.

------ Tile graphics, Big tile, and Big screen

- Tile graphics
  To use graphics mode, choose it from menu for Windows and Mac, or
  use -g option for Linux.  There are 8x8 size old tile and 16x16 size
  Adam bolt tile, and you can change from menu for Windows and Mac, or
  use -- -a option to use 16x16 tile for Linux.  Since source archive
  doesn't include 16x16 size tile, you need to download the
  heng_graf.tar.gz.

- Big tile (Double-width tile)
  In the Big tile mode, graphic tile have double width, and become
  square.  You can choose Bigtile from menu for Windows and Mac, or
  use -- -b option for Linux.

- Big screen
  You can resize each window using mouse, which allow you to have
  terminal windows larger than 80x24.  On linux, it's convenience to
  use environment variables: ANGBAND_X11_COLS_0 and
  ANGBAND_X11_ROWS_0.

