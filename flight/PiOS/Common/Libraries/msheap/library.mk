#
# Rules to add the MSHeap allocator to a PiOS target
#

MSHEAP_DIR		:=	$(dir $(lastword $(MAKEFILE_LIST)))
SRC			+=	$(wildcard $(MSHEAP_DIR)*.c)
EXTRAINCDIRS		+=	$(MSHEAP_DIR)

