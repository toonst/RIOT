APPLICATION = driver_mcp2515
include ../Makefile.tests_common

FEATURES_REQUIRED = periph_gpio periph_spi periph_gpio_irq

USEMODULE += shell
USEMODULE += can
#USEMODULE += auto_init_can
USEMODULE += mcp2515

# set default device parameters in case they are undefined
TEST_MCP2515_SPI   ?= SPI_DEV\(0\)
TEST_MCP2515_CS    ?= GPIO_PIN\(PORT_B,4\)
TEST_MCP2515_RESET ?= GPIO_PIN\(PORT_B,2\)
TEST_MCP2515_MODE  ?= GPIO_PIN\(PORT_B,3\)

# export parameters
CFLAGS += -DTEST_MCP2515_SPI=$(TEST_MCP2515_SPI)
CFLAGS += -DTEST_MCP2515_CS=$(TEST_MCP2515_CS)
CFLAGS += -DTEST_MCP2515_RESET=$(TEST_MCP2515_RESET)
CFLAGS += -DTEST_MCP2515_MODE=$(TEST_MCP2515_MODE)

include $(RIOTBASE)/Makefile.include
