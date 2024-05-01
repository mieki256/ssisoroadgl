TARGET = motosuko.h housakatouge.h bandaiazumaskyline.h yasyajintouge.h

all: $(TARGET)

%.h: %.csv
	python make_spline_road.py -i $< > $@

.PHONY: clean
clean:
	rm -f $(TARGET)

