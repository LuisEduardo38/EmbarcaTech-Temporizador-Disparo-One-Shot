#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/timer.h"

//Declaração das variáveis com os pinos dos leds da placa.
const uint8_t led_red_pino = 13;
const uint8_t led_blue_pino = 12;
const uint8_t led_green_pino = 11;

//Declaração das variáveis do tipo bool para saber o estado dos leds, como também ligar/desligar os leds.
volatile bool estado_red = false;
volatile bool estado_blue = false;
volatile bool estado_green = false;

//Declaração das variáveis com os pinos dos botões da placa.
const uint8_t btn_a = 5;
const uint8_t btn_b = 6;

//Declaração das variáveis de controle do código.
volatile uint32_t utimo_tempo = 0;//Variável para uso no debounce
volatile bool trava = false;//Variável para impedir o acionamento repetitivo do código

//Protótipos das funções
void iniciar_pinos();
void gpio_irq_handler(uint btn, uint32_t events);
bool turn_off_callback(alarm_id_t id, void *user_data);
bool ligar_todos_leds(alarm_id_t id, void *user_data);

int main(){
    //Inicializando a biblioteca e os pinos da placa
    stdio_init_all();
    iniciar_pinos();
    
    //Configurando as chamadas de interrupções
    gpio_set_irq_enabled_with_callback(btn_a, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(btn_b, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    //Loop principal para informa o estado os leds da placa
    while (true) {
        printf("Estado dos leds\n\n");
        printf("Vermelho: %d\nAzul: %d\nVerde: %d\n", estado_red, estado_blue, estado_green);
        sleep_ms(1000);
    }
}

//Função para inicializar os pinos da placa
void iniciar_pinos(){
    //Inicializando os leds da placa
    gpio_init(led_red_pino);
    gpio_init(led_blue_pino);
    gpio_init(led_green_pino);
    gpio_set_dir(led_red_pino, GPIO_OUT);
    gpio_set_dir(led_blue_pino, GPIO_OUT);
    gpio_set_dir(led_green_pino, GPIO_OUT);

    //Inicializando os botões da placa
    gpio_init(btn_a);
    gpio_init(btn_b);
    gpio_set_dir(btn_a, GPIO_IN);
    gpio_set_dir(btn_b, GPIO_IN);
    gpio_pull_up(btn_a);
    gpio_pull_up(btn_b);
}

//Função de interrupção para trata de rótina de interrupção
void gpio_irq_handler(uint btn, uint32_t events){
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());
    //IF para controlar o debounce
    if(tempo_atual - utimo_tempo > 200){
        utimo_tempo = tempo_atual;
        //Neste IF está validado o botão e se a variável TRAVA está com o parâmetro FALSE.
        if((btn == 5) && (trava == false)){
            trava = true;//Colocando a trava como TRUE
            printf("\n\nBTN_A\n\n");
            //Criação do alarme para acionamento dos leds da placa, com o tempo de 3000 milissegundos
            add_alarm_in_ms(3000, (alarm_callback_t)ligar_todos_leds, (void *)NULL, false);
        }
        else if(btn == 6){//IF para entrar no modo BOOTSEL
            printf("\n\nBTN_B\n\n");
            reset_usb_boot(0, 0);
        }
    }
}

//Função de callback para acionar todos os leds.
bool ligar_todos_leds(alarm_id_t id, void *user_data){
    estado_red = true;
    estado_blue = true;
    estado_green = true;
    
    gpio_put(led_red_pino, estado_red);
    gpio_put(led_blue_pino, estado_blue);
    gpio_put(led_green_pino, estado_green);

    //Criando um novo alarme, agora chamando outra função de callback, com o de 300 milissegundos
    add_alarm_in_ms(3000, (alarm_callback_t)turn_off_callback, (void *)NULL, false);

    //Aqui estou retornado um FALSE para que esse alarme não seja chamado novamente.
    return false;
}

//Função que desliga os leds da placa
bool turn_off_callback(alarm_id_t id, void *user_data){
    //Nesta sequência, de IF e ELSE IF seguindo as condições, serão desligados os leds.
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
        trava = false;/*Colocando a trava como FALSE novamente, permito novamente o acionamento
        da rotina e o impedimento da criação de um novo alarme.*/
    }

    /*Neste IF irá validar se a TRAVA for TRUE irá criar um novo alarme para desligar o leds na
    sequência, mas caso esteja FALSE não irá criar um novo alarme encerrando o loop.*/
    if(trava == true){
        add_alarm_in_ms(3000, (alarm_callback_t)turn_off_callback, (void *)NULL, false);
    }

    return false;
}