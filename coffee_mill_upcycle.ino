#include "app_controller.h"

app_controller app;

void setup() {
    app.init();
}

void loop() {
    app.update();
}