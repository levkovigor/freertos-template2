echo "Setting up SOURCE Flatsat Tunnel"
ssh -L 2336:localhost:2331 \
	source@flatsat.source.absatvirt.lw \
	-t "CONSOLE_PREFIX=[SOURCE Port] /bin/bash"
