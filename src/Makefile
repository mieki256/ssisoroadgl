# use MinGW (gcc 6.3.0)

TARGET = ssisoroadgl.scr
OBJS = ssisoroadgl.o render.o settings.o resource.o
DATAS = motosuko.h housakatouge.h bandaiazumaskyline.h yasyajintouge.h car.h scooter.h

all: $(TARGET)

$(TARGET): $(OBJS)
	g++ -o $@ $(OBJS) -static -lstdc++ -lgcc -lscrnsave -lopengl32 -lglu32 -lgdi32 -lcomctl32 -lshlwapi -lwinmm -mwindows

ssisoroadgl.o: ssisoroadgl.cpp render.h settings.h
	g++ -o $@ -c $<

render.o: render.cpp render.h settings.h glbitmfont.h $(DATAS)
	g++ -o $@ -c $<

settings.o: settings.cpp settings.h resource.h
	g++ -o $@ -c $<

resource.o: resource.rc resource.h icon.ico
	windres $< $@

.PHONY: cleanall
cleanall:
	rm -f $(TARGET) *.o

.PHONY: clean
clean:
	rm -f *.o
