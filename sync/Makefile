sync_debug : obj/sync_orm.o obj/spi_common.o
	gcc -O src/sync_debug.c obj/sync_orm.o obj/spi_common.o -I include/ -l bcm2835 -o bin/sync_debug.exe

obj/%.o : src/%.c
	gcc -c $< -I include/ -l bcm2835 -o $@
