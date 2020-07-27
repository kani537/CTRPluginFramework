all:
	cd Library && $(MAKE) install && cd ..
	cd TestPlugin && $(MAKE) relink && cd ..

clean:
	cd Library && $(MAKE) clean && cd ..
	cd TestPlugin && $(MAKE) clean && cd ..

install:
	@sudo mount -t drvfs g: /mnt/g # Mount sdcard (g drive) in wsl2
	@sudo mv TestPlugin/TestPlugin-release.3gx /mnt/g/luma/plugins/default.3gx

installd:
	@sudo mount -t drvfs g: /mnt/g # Mount sdcard (g drive) in wsl2
	@sudo mv TestPlugin/TestPlugin-debug.3gx /mnt/g/luma/plugins/default.3gx