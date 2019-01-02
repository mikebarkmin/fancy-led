all:
	platformio run

upload:
	platformio run --target upload

clean:
	platformio run --target clean

program:
	platformio run --target program

uploadfs:
	platformio run --target uploadfs

update:
	platformio update
