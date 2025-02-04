#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/timer.h"

const uint8_t led_red_pino = 13;
const uint8_t led_blue_pino = 12;
const uint8_t led_green_pino = 11;

volatile bool estado_red = false;
volatile bool estado_blue = false;
volatile bool estado_green = false;

const uint8_t btn_a = 5;
const uint8_t btn_b = 6;

volatile uint32_t utimo_tempo = 0;
volatile bool trava = false;

void iniciar_pinos();
void gpio_irq_handler(uint btn, uint32_t events);
bool desligar_led(alarm_id_t id, void *user_data);
bool ligar_todos_leds(alarm_id_t id, void *user_data);


int main()
{
    stdio_init_all();
    iniciar_pinos();
    
    gpio_set_irq_enabled_with_callback(btn_a, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(btn_b, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true) {
        printf("Estado dos leds\n\n");
        printf("Vermelho: %d\nAzul: %d\nVerde: %d\n", estado_red, estado_blue, estado_green);
        sleep_ms(1000);
    }
}

void iniciar_pinos(){
    gpio_init(led_red_pino);
    gpio_init(led_blue_pino);
    gpio_init(led_green_pino);
    gpio_set_dir(led_red_pino, GPIO_OUT);
    gpio_set_dir(led_blue_pino, GPIO_OUT);
    gpio_set_dir(led_green_pino, GPIO_OUT);

    gpio_init(btn_a);
    gpio_init(btn_b);
    gpio_set_dir(btn_a, GPIO_IN);
    gpio_set_dir(btn_b, GPIO_IN);
    gpio_pull_up(btn_a);
    gpio_pull_up(btn_b);
}

void gpio_irq_handler(uint btn, uint32_t events){
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());
    if(tempo_atual - utimo_tempo > 200){
        utimo_tempo = tempo_atual;
        if((btn == 5) && (trava == false)){
            trava = true;
            printf("\n\nBTN_A\n\n");
            add_alarm_in_ms(3000, (alarm_callback_t)ligar_todos_leds, (void *)NULL, false);
        }
        else if(btn == 6){
            printf("\n\nBTN_B\n\n");
            reset_usb_boot(0, 0);
        }
    }
}

bool ligar_todos_leds(alarm_id_t id, void *user_data){
    estado_red = true;
    estado_blue = true;
    estado_green = true;
    
    gpio_put(led_red_pino, estado_red);
    gpio_put(led_blue_pino, estado_blue);
    gpio_put(led_green_pino, estado_green);

    add_alarm_in_ms(3000, (alarm_callback_t)desligar_led, (void *)NULL, false);

    return false;
}

bool desligar_led(alarm_id_t id, void *user_data){
    if((estado_red == true) && (estado_blue == true) && (estado_green == true)){
        estado_red = !estado_red;
        gpio_put(led_red_pino, estado_red);
    }
    else if((estado_red == false) && (estado_blue == true) && (estado_green == true)){
        estado_blue = !estado_blue;
        gpio_put(led_blue_pino, estado_blue);
    }
    else if((estado_red == false) && (estado_blue == false) && (estado_green == true)){
        estado_green = !estado_green;
        gpio_put(led_green_pino, estado_green);
        trava = false;
    }

    if(trava == true){
        add_alarm_in_ms(3000, (alarm_callback_t)desligar_led, (void *)NULL, false);
    }

    return false;
}