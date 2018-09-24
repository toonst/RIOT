#include <stdint.h>
#include "board.h"
#include "xtimer.h"
#include "saul_reg.h"

#define BUTTON_PUSHED 1
#define BUTTON_RELEASED 0

int main(void)
{
    puts("Hello Nalys!");
    xtimer_init();

    saul_reg_t *button = saul_reg_find_name("WAKE_UP_BUTTON");

    while(true) {
        xtimer_usleep(200000);
        phydat_t data;
        int ret = saul_reg_read(button, &data);
        if (ret < 0) {
            puts("Error reading button");
        }
        if (data.val[0] == BUTTON_PUSHED) {
            LED0_TOGGLE;
            puts("Toggle");
        }
    }
}
