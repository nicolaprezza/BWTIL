all: 

	make clean
	make sFM-index
	make cw-bwt
	make dB-hash	
	make bwt-check
	make bwt-to-sa
	make sa-to-bwt
	make bwt-invert
	
cw-bwt:

	make -C tools/cw-bwt

dB-hash:

	make -C tools/dB-hash

bwt-check:

	make -C tools/bwt-check

bwt-to-sa:

	make -C tools/bwt-to-sa
	
sa-to-bwt:

	make -C tools/sa-to-bwt
	
bwt-invert:

	make -C tools/bwt-invert
	
sFM-index:

	make -C tools/sFM-index

clean:

	rm -f dB-hash cw-bwt bwt-check bwt-to-sa sa-to-bwt bwt-invert sFM-index
