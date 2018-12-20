# Mahjong Solitaire

Requires the latest [CE C libraries](tiny.cc/clibs)

To install, you will need TI-Connect CE or TILP.  
Transfer the C libraries, MAHJONG.8xp, and KYODAI.8xv

Questions? Ask on [Cemetech](https://www.cemetech.net/forum/viewtopic.php?t=15230)

### Running the Kyodai script (not necessary):  
You'll need [ti83f](https://bitbucket.org/keoni29/ti83f) and python 3.  
From the command line, run:
```
python3 kyodai.py NAME file1.lay file2.lay file3.lay ...
```  
which will produce an appvar called NAME.8xv containing all layouts with the correct number of tiles.

### CREDITS:
Thanks to MateoC for creating the C toolchain and putting up with my stupidity.  
Tileset inspired by [GNOME Mahjongg](https://gitlab.gnome.org/GNOME/gnome-mahjongg)  
Kyodai layouts from http://cynagames.com/kyoextra.html  
Kyodai layout loading script based off of [xmahjongg's](https://www.lcdf.org/xmahjongg/)

Source: https://github.com/commandblockguy/mahjong