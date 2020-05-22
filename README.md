# Projeto Manchester 433

Receptor RF433MHz de vários canais baseado em Arduino feito por [Mais Que Maker](https://maisquemaker.com.br).

## Introdução

Manchester é um projeto de hardware e software utilizando [Arduino](https://www.arduino.cc) para construção de um receptor de RF de 433MHz para o controle de "n" canais independentes. Este receptor possui uma ampla gama de usos, podendo ser aplicado em projetos de automação residencial, segurança, automotivos e outros. Algumas características do projeto:

- Utiliza um receptor 433MHz de baixo custo.
- Suporta vários canais, limitado pelo número de portas digitais do Arduino.
- Cada canal pode ser acionado por vários controles TX de 433MHz.
- Cada canal pode ser configurado independentemente para operar como pulso ou retenção.
- Armazena os códigos dos controles na EEPROM, tendo a capacidade de controles limitada pela memória disponível na MCU.
- Possui as funções aprender, limpeza de canal e limpeza da memória.
- Indicação luminosa das atividades de acionamento, memorização e limpeza da memória interna.

O projeto utiliza apenas a biblioteca externa [EEPROM](https://www.arduino.cc/en/Reference/EEPROM) do Arduino, realizando a leitura e interpretação do receptor via software incluído no projeto (bit banging).

O projeto de hardware pode ser acessado no [Thinkercad](https://www.tinkercad.com/things/kPQMFTWsSEO).

É importante destacar que, apesar de o protótipo ter sido montado com quatro canais, é possível reduzir ou ampliar o número de canais, de acordo com a necessidade do projeto, até o limite de entradas/saídas digitais disponíveis na MCU, levando-se em consideração que cada canal utiliza uma porta de entrada para o seu respectivo switch e uma porta de saída que controlará a carga via transistor.

## Hardware

O sistema foi construído usando a plataforma Arduino, mas aplicações práticas pode utilizar apenas o MCU AVR Atmega328P ou equivalente, acompanhado, naturalmente, da circuitaria de alimentação e do oscilador, necessários para o funcionamento de microcontrolador. Se apenas um canal for utilizado, recomenda-se o uso de um [ATTINY85](https://www.microchip.com/wwwproducts/en/ATtiny85) ou sua plataforma de desenvolvimento baseada no [Digispark](http://digistump.com/wiki/digispark). 

A seguir está a lista de componentes utilizados no protótipo:
  
Nome | Quantidade | Componente | Descrição
:---: | :---: | --- | ---
U1 | 1 | Arduino Uno R3 | Plataforma de prototipação
SW1 | 1 | DIP Switch x 4 | O número de switches deve corresponder ao número de canais
S3 | 1 | Botão | Botão de pressão temporário para ser usado na programação
R1, R2, R3, R4, R5 | 5 | 220 Ω Resistor | Resistores para leds
D1 | 1 | Vermelho LED | Indicador de atividades
D2, D3, D4, D5 | 4 | Verde LED | Indicador de acionamento dos canais
T1, T2, T3, T4 | 4 | Transistor NPN (BJT) | Transistores para ativação dos relés
D6, D7, D8, D9 | 4 | Diodo | Diodo "roda livre" para proteger o circuito de surtos causados por indução
R6, R7, R8, R9 | 4 | 1 kΩ Resistor | Resistores de base dos transistores
K1, K2, K3, K4 | 4 | Relé SPDT | Relés para acionamento das cargas

## Circuito

A seguinte montagem foi realizada na prototipação do projeto:

![Módulo lógico](https://raw.githubusercontent.com/maisquemaker/manchester433/master/circuits/Manchester%20-%20Modulo%20Logico.png)

Os quatro canais utilizados na prototipação podem controlar cargas através de módulos de relés, de acordo com a montagem abaixo. As saídas dos canais do módulo lógico devem ser conectadas nas bases dos transistores através dos resistores R6 a R9.

![Módulo de transistores](https://raw.githubusercontent.com/maisquemaker/manchester433/master/circuits/Manchester%20-%20Modulo%20Transistor.png)

## Operação

Cada canal pode ser configurado independentemente para funcionar com pulso ou retenção. Quando configurado em modo pulso, o canal permanecerá ligado apenas enquanto o botão do controle estiver sendo acionado. Quando configurado em modo retenção, acionamentos do controle anternarão o estado do canal (ligado/desligado).

Para selecionar o modo pulso, o switch do respectivo canal deverá estar na posição ON. Caso este switch esteja na posição OFF, o canal irá operar no modo retenção.

## Programação

Para programar um controle ou outro dispositivo (por exemplo: sensor de abertura de porta por RF, detector de movimento entre outros), o procedimento é o que segue:

- Desligar os switches de todos os canais e deixar ligado apenas o do canal a ser programado.
- Segurar o botão do controle a ser programado (ou ativar o dispositivo).
- Apertar e soltar o botão de programação. A luz irá acender por 3 segundos para indicar que a programação foi bem sucedida
- Soltar o botão do controle.
- Se não houver mais memória para cadastro do controle, o indicador luminoso irá piscar 3 vezes indicando que a operação não pôde ser concluída.
- Repetir a operação para cada um dos botões nos respectivos canais que eles irão controlar.
- Ao final da programação, retornar os switches para a posição de operação (ligado = pulso; desligado = retenção).

## Apagando a memória

O sistema possui duas maneiras de apagar a memória:

- Limpeza de um canal: todos os controles que acionam o canal selecionado serão removidos da memória.
- Limpeza da memória: todos os controles serão apagados de todos os canais, deixando o sistema completamente vazio.

### Limpeza de um canal

Para limpar todos os controles de um canal, o procedimento é o seguinte:

- Desliguar os switches de todos os canais, deixando ligado apenas o switch referente ao canal que se deseja apagar.
- Segurar o botão de programação por 5 segundos. 
- O indicador luminoso irá piscar 5 vezes indicando que a operação foi bem sucedida.

### Limpeza da memória

Para limpar todos os controles de todos os canais:

- Desligar todos os switches
- Segurar o botão de programação por 10 segundos.
- O indicador luminoso irá piscar 5 vezes indicando que a operação foi bem sucedida.

## Contribuição

Por favor, sinta-se à vontade para contribua com o aperfeiçoamento deste projeto. Toda ajuda será sempre bem vinda.
