TARGET = car.h scooter.h

all: $(TARGET)

car.h: car.obj car.mtl
	python pyobj2c.py $< > $@

scooter.h: scooter.obj scooter.mtl
	python pyobj2c.py $< > $@

.PHONY: clean
clean:
	rm -f $(TARGET)
