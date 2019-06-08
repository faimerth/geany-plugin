COMPILE_OPTIONS=-static -fno-builtin -fno-stack-protector -fno-stack-limit -march=native -mcmodel=large -I "$$HOME/FDTH/CCE/Headers" -I "$$HOME/FDTH/CCE/EHeaders" -w
LINK_OPTIONS=-static -L "$$HOME/FDTH/CCE/Library" -L "$$HOME/FDTH/CCE/ELibrary" -lm

./lib/Faimerth.so:./tmp/Faimerth_api.o ./tmp/Faimerth_core.o ./lib
	gcc ./tmp/Faimerth_core.o ./tmp/Faimerth_api.o -flto -fPIC -o ./lib/Faimerth.so -shared $(pkg-config --libs geany)

./tmp/Faimerth_api.o:Faimerth_api.c Faimerth_core.h Faimerth_config.h Faimerth_gui.h ./tmp
	gcc -c -O3 Faimerth_api.c -o ./tmp/Faimerth_api.o -flto -fPIC $$(pkg-config --cflags geany) -I "$$HOME/FDTH/CCE/Headers" -w

./tmp/Faimerth_core.o:Faimerth_core.c ./tmp
	gcc -c -O3 Faimerth_core.c -DNOMAIN -flto -fPIC -o ./tmp/Faimerth_core.o $(COMPILE_OPTIONS)

./tmp:
	mkdir ./tmp

./lib:
	mkdir ./lib

clean:
	rm ./tmp/Faimerth_api.o ./tmp/Faimerth_core.o ./lib/Faimerth.so