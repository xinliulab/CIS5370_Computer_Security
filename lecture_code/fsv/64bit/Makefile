all:
	@echo "Disabling ASLR..."
	@echo 0 | sudo tee /proc/sys/kernel/randomize_va_space > /dev/null
	gcc -g -o fmtvul fmtvul.c -z execstack -fno-stack-protector -Wno-format-security
	python3 fmtexploit_revised.py

run: all
	@echo "Running exploit..."
	./fmtvul

clean:
	rm -f fmtvul badfile
