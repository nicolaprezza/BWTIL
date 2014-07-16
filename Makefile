all: 

	make clean
	make cw-bwt
	make dB-hash	
	make bwt-check

cw-bwt:

	make -C tools/cw-bwt

dB-hash:

	make -C tools/dB-hash

bwt-check:

	make -C tools/bwt-check

clean:

	rm -f dB-hash cw-bwt bwt-check
