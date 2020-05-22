/*
    Ao mudar o pulso, verificar o comprimento do pulso anterior
    - Comprimento curto: ~0,5ms
    - Comprimento longo: ~1,0ms
    - Comprimento muito longo: início ou fim de uma transmissão (~11ms)
    - Comprimento muito mais longo: botão foi solto (> 50ms)
*/

#include <Arduino.h>

#define pinoRX      2               //Pino onde está conectado o RX 433
#define tamBf       10              //Tamanho do buffer de código

//Os tempos abaixo são expressos em microssegundos

#define tp0         500             //Tempo do pulso curto
#define tp1         1000            //Tempo do pulso longo
#define tpBtn       11000           //Tempo do início/término de uma transmissão
#define tpFim       50000           //Tempo em low após o botão ser solto
#define tDelta       100             //Valor da margem de segurança nas durs dos pulsos


unsigned long       t0;             //Armazena os milissegundos do t0
unsigned long       t1;             //Usado para calcular a diferença de tempos
unsigned long       dur;            //Duração do pulso registrado

bool                recebendo;      //Indica se um código está sendo recebido

bool                codOk;          //Indica se há um código capturado no buffer
unsigned char       bitsRec;        //Número de bits que já foram recebidos

unsigned char       cod[tamBf];     //Armazena o código disponível
unsigned char       buf[tamBf];     //Buffer para armazenar o código durante a leitura



void reiniciaBuffer()
{
    digitalWrite(13,LOW);
//    codOk = false;
    recebendo = false;
    bitsRec = 0;
    memset(buf,0,tamBf);
//    memset(cod,0,tamBf);
}



void recebePulso()
{
    t1 = micros();
    
    unsigned char numByte;

    //Mede a duração do pulso 
    dur = t1 - t0;
    t0 = t1;


    //Verifica se o botão foi solto
    if ((dur > tpFim - tDelta) ) { // || (dur > tpBtn + tDelta && recebendo)

        reiniciaBuffer();
        codOk = false;

        return;
    }


    //Verifica se houve um pulso de início/término de transmissão
    if ((dur > tpBtn - tDelta) && (dur < tpBtn + tDelta)) {

        digitalWrite(13, HIGH);

        bitsRec = 0;
        recebendo = true;
        memset(buf,0,tamBf);
        return;


        //Verifica se já está recebendo
        if (recebendo) {
  
            // É um pulso de término
  
            //Move o buffer para a área do código
            memcpy(cod, buf, tamBf);
            codOk = true;

            //Limpa o buffer
            bitsRec = 0;
            memset(buf,0,tamBf);

            //Não altera a flag 'recebendo' porque 
            //virá outro trem de pulsos em seguida

        } else {

            // É um pulso de início
            bitsRec = 0;
            recebendo = true;

        }

        return;
    }

    //Verifica se está esperando um pulso de dados
    if (recebendo) {

        //Verifica se o buffer está cheio
        if (bitsRec >= tamBf * 8) {

            reiniciaBuffer();

        } else {

            //Prepara o buffer para receber o bit
            numByte = bitsRec / 8;

            //Verifica se já obteve todos os bits necessários
            if (numByte == 7) {

                //Move o buffer para a área do código
                memcpy(cod, buf, tamBf);
                codOk = true;
    
                //Limpa o buffer
                bitsRec = 0;
                memset(buf,0,tamBf);
            }

            else {
              
                buf[numByte] = buf[numByte] << 1;
               
                //Se recebeu um "bit 1" (pulso longo)
                if ((dur >= tp1 - tDelta) && (dur <= tp1 + tDelta)) {
                    buf[numByte]++;
                    bitsRec++;
                } 
                else
    
                //Se recebeu um "bit 0" (pulso curto)
                if ((dur >= tp0 - tDelta) && (dur <= tp0 + tDelta)) {
                    bitsRec++;
                }
            }
        }
    }
}
