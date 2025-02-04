# EmbarcaTech-Temporizador-Disparo-One-Shot </br>
# Funcionamento do código </br>
O código funciona através do acionamento do botão A que irá entrar numa rotina de interrupção, onde será criado um alarme com o uso do comado add_alarm_in_m() para realizar a chamada da função de callback para iniciar o loop de acionamento e desligamento dos leds.</br>