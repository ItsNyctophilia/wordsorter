.DEFAULT_GOAL := ws
CFLAGS += -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal

.PHONY: debug
debug: CFLAGS += -g
debug: ws

.PHONY: clean
clean:
	${RM} ws
