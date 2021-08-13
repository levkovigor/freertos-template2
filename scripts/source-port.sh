echo "Setting up SOURCE Flatsat Tunnel"
echo "-L 2336:localhost:2331 for remote debugging"
echo "-L 2337:localhost:7301 for TMTC commanding"
ssh -L 2336:localhost:2331 \
	-L 2337:localhost:7301 \
	source@flatsat.source.absatvirt.lw \
	-t 'CONSOLE_PREFIX="[SOURCE Port]" /bin/bash'
