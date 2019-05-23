# ----------------------------
# Set NAME to the program name
# Set ICON to the png icon file name
# Set DESCRIPTION to display within a compatible shell
# Set COMPRESSED to "YES" to create a compressed program
# ----------------------------

NAME        ?= MAHJONG
COMPRESSED  ?= YES
ICON        ?= iconc.png
DESCRIPTION ?= "Mahjong Solitaire"

# ----------------------------

include $(CEDEV)/include/.makefile

# ----------------------------

# Make the Kyodai appvar
pack: KYODAI.8xv
KYODAI.8xv: importer/kyodai_pack/*.lay
	python3 importer/kyodai.py KYODAI importer/kyodai_pack/*.lay

clean_gfx:
	-rm src/gfx/convpng.log
	-rm src/gfx/*.c
	-rm src/gfx/*.h
	-rm src/gfx/palette

# Format a release for the Cemetech archives
release:
	make clean
	make clean_gfx

	-rm -r release_tmp
	mkdir release_tmp
	cp -r importer README.md release_tmp

	mkdir release_tmp/source
	cp -r src iconc.png makefile release_tmp/source

	make pack
	cp KYODAI.8xv release_tmp

	make gfx
	make
	cp bin/MAHJONG.8xp release_tmp
	zip -r mahjong.zip release_tmp
	rm -r release_tmp
