CROSS = arm-linux-gnueabi-
BUILD_DIR = build

CC = $(CROSS)gcc
CFLAGS = -I. -Wall -D_GNU_SOURCE -Os

all: watchdogd wdt-kick
	@echo "build complete"

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

-include $(BUILD_DIR)/*.d

watchdogd: $(BUILD_DIR)/watchdog.o
	$(CC) $(CFLAGS) $^ -o $@
	$(CROSS)strip $@

wdt-kick: $(BUILD_DIR)/wdt-kick.o
	$(CC) $(CFLAGS) $^ -o $@
	$(CROSS)strip $@

clean:
	rm -rf $(BUILD_DIR) watchdogd wdt-kick
