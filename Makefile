all:
	$(MAKE) -C build/master all -B
	
qemu:
	$(MAKE) -C build/master qemu -B

clean:
	$(MAKE) -C build/master clean -B
