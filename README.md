# Projeto Manchester

Receptor RF433MHz de vários canais baseado em Arduino feito por [Mais Que Maker](https://maisquemaker.com.br).



Manchester é um projeto de hardware e software utilizando [Arduino](https://www.arduino.cc) para construção de um receptor de RF de 433MHz para o controle de "n" canais independentes. Este receptor possui uma ampla gama de usos, podendo ser aplicado em projetos de automação residencial, segurança, automotivos e outros. Algumas características do projeto:

- Utiliza um receptor 433MHz de baixo custo.
- Suporta vários canais, limitado pelo número de portas digitais do Arduino.
- Cada canal pode ser acionado por vários controles TX de 433MHz.
- Cada canal pode ser configurado independentemente para operar como pulso ou retenção.
- Armazena os códigos dos controles na EEPROM, tendo a capacidade de controles limitada pela memória disponível na MCU.
- Possui as funções aprender, esquecer canal e apagar a memória.
- Indicação luminosa das atividades de acionamento, memorização e limpeza da memória interna.

O projeto utiliza apenas a biblioteca externa [EEPROM](https://www.arduino.cc/en/Reference/EEPROM) do Arduino, realizando a leitura e interpretação do receptor via software incluído no projeto (bit banging).

O projeto de hardware pode ser acessado no [Thinkercad](https://www.tinkercad.com/things/kPQMFTWsSEO).

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
