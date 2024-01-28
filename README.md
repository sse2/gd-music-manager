# Geometry Dash Music Manager - Geode Mod
Replace and randomize music without changing the original files. Compatible with Mega Hack and textureldr!

## Build
Use VS2022.

## How to use
Create a "music-manager" directory inside your Geometry Dash root. Create a folder with the name of the track you want to change (see below for caveats). Put the replacement tracks inside the folder (name and format don't matter, just make sure you only have sounds inside the folder or else things might break).

## Caveats
I don't know if this is the best way of achieving this result, but I don't care.
Geode isn't fully complete so functions like playEffectAdvanced crash when calling original after hooking so we can't replace SFX (yet). Other functions such as queueStartMusic are not implemented yet therefore can't be hooked.