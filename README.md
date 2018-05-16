# HGCal-RPi:sync-auto-detect

This branch changes one thing: the `rdout_mask` set by the user is not used. It is now automatically set by the `cables_mask` that the sync board provides. This allows easier swapping/addition of readout boards, since the `rdout_mask` doesn't have to be set whenever the cables get changed.`
