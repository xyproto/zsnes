This is the version of winlink.cpp with vsync and triple buffering enabled
  by default.  The big change is that originally, ZSNESw wasn't using
  exclusive mode in full-screen and just changed the windows desktop
  resolution and created a window the size of the resolution.  So exclusive
  mode was added.  This file is separate from the sources since it still
  has problems (note below).

**WARNING** this will freeze the O/S if you switch video modes
  in ZSNESw.  I cannot seem to figure out why though, but I do know that
  when switching from full-screen to windowed will cause a create primary
  buffer error (can't exactly see the error though), while switching
  from full to another full will cause a different error.

Problem #2: It seems like the first call to startgamefull() will cause
  an error in setcooperativelevel with the error InvalidParams.  But it
  also seems like the same call later in the future turns out to be okay.
  Weird.

The routines that are changed are:
- startgame() is modified
- startgamefull() is added (for exclusive mode)
- DrawScreen() has been changed to add in the back buffer/swap blitter
- initwinvideo() is also changed.

Note: initwinvideo() seems to be called twice (or more?) when ZSNESw starts
  and also once when the game starts (not sure about exiting the game though)
