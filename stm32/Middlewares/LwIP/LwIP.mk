INCLUDES += $(CURRENTPATH)/src/include
INCLUDES += $(CURRENTPATH)/system


# Only add required sources

CSRC += $(wildcard $(CURRENTPATH)/src/core/*.c)
CSRC += $(wildcard $(CURRENTPATH)/src/core/ipv4/*.c)
CSRC += $(wildcard $(CURRENTPATH)/src/api/*.c)
CSRC += $(wildcard $(CURRENTPATH)/src/netif/*.c)