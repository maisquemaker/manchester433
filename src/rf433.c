#include <Arduino.h>

//Os tempos abaixo são expressos em microssegundos
#define     RFC_tp0                 500 //Tempo do pulso curto
#define     RFC_tp1                1000 //Tempo do pulso longo
#define     RFC_tMaxBusca         75000 //Tempo máximo que se pode esperar por um botão
#define     RFC_tBtn              11000 //Tempo do início/término de um botão (em LOW)
#define     RFC_tDelta              100 //Margem de erro para os tempos dos bits

#define     RFC_tamCod               10 //Tamanho do buffer para um código de controle
#define     RFC_minBits              50 //Mínimo de bits que formam um código


unsigned char buscaCodigo(uint8_t pino, unsigned char *codigo)
{
    unsigned long   dur;                //Variável para cálculo da duração
    unsigned long   t0;                 //Tempo onde a busca começou
    unsigned long   t1;                 //Usado para calcular o tamanho dos pulsos de dados
    unsigned char   nivel;              //Nível atual do pino
    unsigned char   numBits;            //Número de bits recebidos
    unsigned char   numByte;            //Número do byte atual sendo preenchido


    //Prepara as variáveis iniciais
    t0 = micros();
    nivel = HIGH;
    numBits = 0;

    //Limpa o buffer
    memset(codigo,0,RFC_tamCod);
  
    //Busca pelo máximo tempo permitido
    while (micros() - t0 < RFC_tMaxBusca) {

        //Aguarda um pulso de Start/Stop
        //Observe que no terceiro parâmetro existe um timeout
        dur = pulseIn(pino, LOW, RFC_tMaxBusca);

        //Verifica se o tamanho é semelhante ao do pulso Start/Stop
        if ((dur >= RFC_tBtn - RFC_tDelta) && (dur <= RFC_tBtn + RFC_tDelta)) {

            //Armazena o tempo inicial para poder medir o próximo pulso
            t1 = micros();

            do {
        
                //Se o nível do pino mudou (HIGH->LOW ou LOW->HIGH)
                if (digitalRead(pino) != nivel) {

                    //Calcula sua duração e atualiza o tempo inicial para medir o próximo pulso
                    dur = micros() - t1;
                    t1 = micros();

                    //Inverte o nível a esperar para o próximo pulso
                    nivel = 1 - nivel;

                    //Se for um pulso curto ou um pulso longo
                    if (((dur >= RFC_tp0 - RFC_tDelta) && (dur <= RFC_tp0 + RFC_tDelta)) || 
                        ((dur >= RFC_tp1 - RFC_tDelta) && (dur <= RFC_tp1 + RFC_tDelta))) {

                        //Calcula o byte que está sendo preenchido
                        numByte = numBits++ / 8;

                        //Se o código é maior que o buffer, retorna erro
                        if (numByte >= RFC_tamCod) {
                            memset(codigo,0,RFC_tamCod);
                            return 0;
                        }

                        //Gira o byte à esquerda para receber o próximo bit
                        codigo[numByte] <<= 1;

                        //Pulso longo
                        if (dur >= RFC_tp0 + RFC_tDelta) {
                            codigo[numByte] |= 1;
                        }
                    } 
                    
                    //Se for um pulso de Stop
                    else if (dur >= RFC_tBtn - RFC_tDelta) {

                        //Se não foi obtido o número mínimo de bits, retorna erro
                        if (numBits < RFC_minBits) {
                        
                            //Limpa o buffer
                            memset(codigo,0,RFC_tamCod);
                            return 0;
            
                        } else {

                            //Ajusta os bits do último byte
                            if (numBits % 8) {
                                numByte = numBits / 8;
                                codigo[numByte] <<= (8 - (numBits % 8));
                            }

                            //Retorna o número de bits do código
                            return numBits;
                        }
                    }
                } 
            } while (micros() - t1 <= RFC_tMaxBusca);
        }
    }
    
    //Não localizou pulso de Start/Stop no tempo necessário
    return 0;
}
