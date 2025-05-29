OUTPUT = amdgpucontrol

all:
	cc -Wall -Wextra -Wpedantic main.c -o $(OUTPUT)

install:
	cp -f $(OUTPUT) /usr/local/bin
	cp -f $(OUTPUT).conf /etc

clean:
	rm -f $(OUTPUT)