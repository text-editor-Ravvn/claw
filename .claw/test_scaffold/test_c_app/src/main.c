#include <stdio.h>
#include "project.h"

void app_init(void) {
    printf("Initializing %s v%s by %s\n", APP_NAME, APP_VERSION, APP_AUTHOR);
}

int main(void) {
    app_init();
    printf("Hello, my_c_project!\n");
    return 0;
}
