	cd ../..
    ./makeall --nosample
	cd addon/linux
	make
	cd ../vc4
	./makeall
	cd ../../test/sound-controller
	make