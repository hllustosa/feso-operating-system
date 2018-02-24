#!/bin/bash

cd src/kernel
make
cd ../..
cp src/kernel/bin/kernel.bin isofiles/boot
cp src/kernel/bin/trampolim.o isofiles/drivers/tramp.o

cd src/dev/Console
make
cd ../../..
cp src/dev/Console/bin/console.o isofiles/drivers

cd src/dev/Timer
make
cd ../../..
cp src/dev/Timer/bin/timer.o isofiles/drivers

cd src/dev/Fat
make
cd ../../..
cp src/dev/Fat/bin/fat.o isofiles/drivers

cd src/dev/Iso9660
make
cd ../../..
cp src/dev/Iso9660/bin/iso9660.o isofiles/drivers

cd src/dev/Ide
make
cd ../../..
cp src/dev/Ide/bin/ide.o isofiles/drivers

cd src/app/Editor
make
cd ../../..
cp src/app/Editor/bin/editor.o isofiles/app

cd src/app/Idle
make
cd ../../..
cp src/app/Idle/bin/idle.o isofiles/app

cd src/app/Shell
make
cd ../../..
cp src/app/Shell/bin/shell.o isofiles/app

cd src/dev/Hdd
make
cd ../../..
cp src/dev/Hdd/bin/hdd.o isofiles/drivers

cd src/app/Egauss
make
cd ../../..
cp src/app/Egauss/bin/egauss.o isofiles/app

cd src/app/Strassen
make
cd ../../..
cp src/app/Strassen/bin/strassen.o isofiles/app

cd src/app/Matrizes
make
cd ../../..
cp src/app/Matrizes/bin/matrizes.o isofiles/app

cd src/app/Tetris
make
cd ../../..
cp src/app/Tetris/bin/tetris.o isofiles/app

mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o  feso.iso isofiles
