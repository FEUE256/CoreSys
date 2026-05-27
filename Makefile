all:
	$(MAKE) -C build/master all -B
	
run:
	$(MAKE) -C build/master run -B

clean:
	$(MAKE) -C build/master clean -B
