#include <EEPROM.h>
#include "rf433.c"

//Configurações gerais
#define     numCanais                 4 //Número de canais a serem controlados
#define     numBits                  56 //Número de bits do código
#define     maxCont                 128 //Número máximo de controles na memória

//Definições dos pinos
#define     pinoRF                    3 //Pino do receptor RF
#define     pinoProg                  8 //Pino da chave de programação
#define     primPinoCanais            4 //Primeiro pino dos canais
#define     primPinoJmprs             9 //Primeiro pino dos jumpers
#define     pinoLed                  13 //Pino do led

//Tempos expressos em milissegundos
#define     tPulso                  200 //Tempo do pulso
#define     tApagarCanal           5000 //Tempo para apagar a memória de um canal
#define     tApagarTudo           10000 //Tempo para apagar toda a memória

//Número de bytes que um controle gasta na memória
#define     numBytes    ((int)(numBits / 8 + 1) + ((numBits % 8 > 0) ? 1 : 0))

//Códigos do sinal luminoso a ser apresentado
#define     MEM_CHEIA                 1 //Memória cheia
#define     GRAVADO                   2 //Controle gravado com sucesso
#define     APAGA_CANAL               3 //Memória do canal apagada
#define     APAGA_TUDO                4 //Toda a memória apagada


unsigned char codigo[numBytes+1];       //Buffer para armazenar o código obtido do controle
unsigned char qBits;                    //Quantidade de bits do código obtido

unsigned int  ultContr;                 //Índice na memória do último controle pressionado
unsigned long tUltContr;                //Momento em que o último controle foi pressionado
unsigned long tProg;                    //Tempo que o botão prog está pressionado

unsigned long tCanais[numCanais];       //Controle de tempo dos canais (segundos)
unsigned char pCanais[numCanais];       //Pinos das saídas dos canais
unsigned char pJmprs [numCanais];       //Pino do receptor de RF


// #define __DEBUG
// #define __CLEAR



void setup()
{
    //TAREFAS DE DEBUG

    #ifdef __DEBUG
        Serial.begin(115200);

        for (int i = 0, c = 0; i < EEPROM.length(); i++) {
      
            //Limpeza total da memória
            #ifdef __CLEAR
                EEPROM.write(i,255);
            #endif
      
            Serial.print(String(EEPROM.read(i), HEX));
            Serial.print(" ");

            if (++c == 8) {
                c = 0;
                Serial.println();
            }
        }
    #endif


    //SETUP DOS PINOS

    //Ajusta as informações dos canais
    for (int i = 0; i < numCanais; i++) {

        //Saídas dos canais
        pCanais[i] = primPinoCanais + i;
        pinMode(pCanais[i], OUTPUT);

        //Jumpers de configuração
        pJmprs[i] = primPinoJmprs + i;            
        pinMode(pJmprs[i], INPUT_PULLUP);

        //Controle de tempo de cada canal
        tCanais[i] = 0;
    }

    //Entrada do receptor RF
    pinMode(pinoRF, INPUT);

    //Botão de programação
    pinMode(pinoProg, INPUT_PULLUP);

    //Led
    pinMode(pinoLed, OUTPUT);
}



void loop()
{
    //Atualiza os tempos de timeout
    atualizaTempos();

    //Busca um código no RF
    qBits = buscaCodigo(3,codigo);

    //Se o código for válido
    if (qBits >=  numBits) {

        //Se o botão de programação estiver apertado...
        if (digitalRead(pinoProg) == LOW) {

            //...programa o canal. Senão, ...
            programaCanal();

        } else {

            //...atualiza o status dos relés
            atualizaCanais();
        }
    }
}



//Atualiza os temporizadores do programa (canais, último botão e botão prog)
void atualizaTempos()
{
    //Verificação dos tempos todos os canais
    for (int i = 0; i < numCanais; i++) {

        //Verifica se o jumper do canal está fechado
        if (digitalRead(pJmprs[i]) == LOW) {

            //Trata apenas os canais de pulso
            
            //Se algum botão estiver pressionado e o canal estiver ligado, estende o tempo de timeout
            if ((ultContr < maxCont) && (tCanais[i] > 0)) {

                tCanais[i] = millis();

            //Se não houver botão pressionado e o canal estiver ligado, desliga o canal se houver timeout
            } else {

                if ((millis() - tCanais[i] > tPulso) && (tCanais[i] > 0)) {
                
                    digitalWrite(pCanais[i], LOW);
                    tCanais[i] = 0;
                }
            } 
        }
    }
    
    //Verificação do timeout do último botão apertado
    if (tUltContr && (millis() - tUltContr > tPulso)) {

        tUltContr = 0;
        ultContr = 65535;

        //Apaga o led indicativo de controle reconhecido
        digitalWrite(pinoLed, LOW);
    }
    
    //Verifica o tempo do botão usado para apagar a memória
    if (digitalRead(pinoProg) == LOW) {

        if (tProg == 0) {

            //Registra o momento em que o botão foi pressionado
            tProg = millis();
        
        } else {
        
            //Verifica se já está pressionado tempo suficiente para apagar toda a memória
            if (millis() - tProg > tApagarTudo) {
          
                apagaMemoria(true);
            }
        
            //Verifica se já está pressionado tempo suficiente para apagar o canal selecionado
            else if (millis() - tProg > tApagarCanal) {

                apagaMemoria(false);
            }
        }

    } else {

        //Se o botão prog não estiver apertado, zera o registro de tempo do botão
        tProg = 0;
    }
}



//Atualiza o status dos canais caso o controle pressionado seja reconhecido
void atualizaCanais()
{
    unsigned int    endereco;           //Endereço do controle detectado
    unsigned char   canais;             //Canais associados ao controle detectado


    //Verifica se é um controle reconhecido
    endereco = encontraControle();
    if (endereco >= maxCont) {
        return;
    }
  
    //Acende o led indicando que um controle foi reconhecido
    digitalWrite(pinoLed, HIGH);

    //Obtém os canais ativados por este controle
    canais = encontraCanais(endereco);

    //Verifica para quais canais o controle está ativo
    for (int i = 0; i < numCanais; i++) {

        //Este canal está associado ao controle (obtendo apenas o bit mais à direita)
        if ((canais & 1) == 1) {

            //Se o canal for pulso, liga o canal e grava o momento em que foi pressionado
            //Se o canal já estiver ligado, renova o timeout, pois o botão continua pressionado
            if (digitalRead(pJmprs[i]) == LOW) {

                digitalWrite(pCanais[i], HIGH);
                tCanais[i] = millis();
            }

            //Se o canal for retenção, verifica a necessidade de alternar o status
            else {
                
                //Verifica se o botão continua pressionado
                if (ultContr != endereco) {

                    tCanais[i] = 1 - tCanais[i];            //Por definição, HIGH = 1 e LOW = 0
                    digitalWrite(pCanais[i], tCanais[i]);
                }
                
            }
        }

        //Gira os bits do byte que indica os canais associados a este controle para a direita
        canais >>= 1;
    }

    //Registra este controle e o seu tempo como o último controle utilizado
    ultContr = endereco;
    tUltContr = millis();
}



//Programa o canal do controle recebido para operar o canal cujo jumper estiver inserido
void programaCanal()
{
    unsigned char numC = 0;             //Auxiliar para verificar se apenas um canal está selecionado   
    unsigned int  endereco;             //Endereço obtido para armazenar o controle (0, 1, 2, 3, ...)
    unsigned char canal;                //Canal a ser associado ao controle


    //Para a contagem do botão prog, para evitar apagamento da memória
    tProg = 0;

    //Verifica se apenas um jumper está inserido
    for (int i = 0; i < numCanais; i++) {
        if (digitalRead(pJmprs[i]) == LOW) {
            numC++;
            canal = i;
        }
    }
    
    //Se não houver canal ou houver mais de um selecionado, não continua
    if (numC != 1) {
        return;
    }

    //Verifica se este mesmo controle já está associado a algum canal
    endereco = encontraControle();

    //Se não existir na memória
    if (endereco >= maxCont) {
      
        //Tenta encontrar uma posição vazia na memória
        endereco = posicaoVazia();

        //Verifica se tem espaço na memória
        if (endereco >= maxCont) {
            avisoLuminoso(MEM_CHEIA);
            return false;
        }
    
        //Armazena o byte inficativo do canal
        EEPROM.write(numBytes * endereco, 1 << canal);

        //Armazena o código do controle
        for (int i = 0; i < numBytes - 1; i++) {
            EEPROM.write((numBytes * endereco) + i + 1, codigo[i]);
        }
  
    } else {

        //O controle já existe na memória: apenas atualiza os canais que ele ativa
        EEPROM.write(numBytes * endereco, (EEPROM.read(numBytes * endereco) | (1 << canal)));
    }

    avisoLuminoso(GRAVADO);
}



//Retorna os canais que o controle opera
unsigned char canaisControle() {

    unsigned int    endereco;           //Endereço na memória do controle pesquisado


    //Tenta encontrar o endereço desse controle
    endereco = encontraControle();

    //Se não encontrar, retorna 0
    if (endereco >= maxCont) {
        return 0;
    }

    //Se encontrou o controle na memória, retorna os canais que ele controla
    return encontraCanais(endereco);
}



//Retorna o índice da memória onde o controle está armazenado (0, 1, 2, 3, ...)
unsigned int encontraControle()
{
    unsigned char   canais;             //Canais do controle encontrado
    unsigned char   iguais;             //Variável para contar os bytes iguais do controle

    unsigned char   b;                  //Variável auxiliar para armazenar um byte lido da EEPROM


    //Antes, verifica se o controle é válido
    if (qBits < numBits) {
        return 65535;
    }
  
    //Verifica todos os controles da memória
    for (unsigned int numCont = 0; numCont < maxCont; numCont++) {
        
        //Busca o primeiro byte do controle armazenado (os canais)
        canais = EEPROM.read(numBytes * numCont);

        //Se a posição da memória não estiver vazia (1º byte entre 1 e 254)
        if (canais != 0 && canais != 255) {  
            
            //Agora, compara o código do controle com a memória 
            iguais = 0;
            for (int i = 0; i < numBytes-1; i++) {

                b = EEPROM.read(numBytes * numCont + i + 1);
                if (b == codigo[i]) {
                    iguais++;
                }
            }

            if (iguais == numBytes-1) {
                return numCont;
            } 
        }
    }

    //Não encontrou. Retorna um número maior que o possível
    return maxCont + 1;
}



//Tenta encontrar uma posição vazia na memória para armazenar um controle
unsigned int posicaoVazia()
{
    unsigned char   canais;             //Variável para auxiliar a análise do primeiro byte do código

    //Verifica todos os controles da memória
    for (unsigned int numCont = 0; numCont < maxCont; numCont++) {
        
        canais = EEPROM.read(numBytes * numCont);

        //Se encontrar uma posição vazia (1º byte = 0 ou 255), retorna este índice
        if (canais == 0 || canais == 255) {
            return numCont;
        }
    }

    //Retorna um valor grande para indicar que não há espaço na memória
    return 65535;
}



//Retorna os canais de um dado endereço da memória (0, 1, 2, ...)
unsigned char encontraCanais(unsigned int endereco)
{
    unsigned char   canais;             //Armazena o byte com os canais controlados (1º byte do código)


    //Verifica se é um endereço inválido
    if (endereco >= maxCont) {
        return 0;
    }
    
    //Lê o primeiro byte do canal cujo índice foi passado
    canais = EEPROM.read(numBytes * endereco);

    //Ajusta 255 para 0
    if (canais == 255) {
        canais = 0;
    }

    //Retorna o byte obtido
    return canais;
}



//Apaga a memória de todos os controles
//O parâmetro indica se é para apagar TUDO ou apenas um canal
//Importante notar que no caso de apagamento total, apenas o primeiro byte de cada posição é apagado 
void apagaMemoria(bool tudo)
{
    unsigned char   numC = 0;           //Auxiliar para verificar se apenas um canal está selecionado   
    unsigned char canal;                //Caso se trate de apagar um canal, indica que canal é esse                
    unsigned char b;                    //Variável auxiliar para armazenar um byte vindo da EEPROM


    //Se for para apagar apenas um canal
    if (!tudo) {

        //Verifica se apenas um jumper está inserido
        for (int i = 0; i < numCanais; i++) {

            if (digitalRead(pJmprs[i]) == LOW) {
                numC++;
                canal = i;
            }
        }
        
        //Se não houver canal ou houver mais de um selecionado, não continua
        if (numC != 1) {
            return;
        }

        //Percorre a memória apagando todas as programações do canal
        for (int i = 0; i < maxCont; i++) {

            b = EEPROM.read(numBytes * i);

            if (b != 0 && b != 255) {

                //Operação necessária para apagar o bit referente ao canal
                EEPROM.write(numBytes * i, b & (~(1 << canal)));
            }
        }

        //Emite o aviso luminoso adequado
        avisoLuminoso(APAGA_CANAL);

    } else {

        //Apaga o primeiro byte de cada posição na memória
        for (int i = 0; i < maxCont; i++) {
            EEPROM.write(numBytes * i, 0);
        }

        //Emite o aviso luminoso adequado
        avisoLuminoso(APAGA_TUDO);
    }
}



//Rotina genérica para avisos luminosos
void avisoLuminoso(unsigned char tipo)
{
  switch(tipo) {

    case MEM_CHEIA:
      for (int i = 0; i < 3; i++) {
        digitalWrite(pinoLed, HIGH);
        delay(200);
        digitalWrite(pinoLed, LOW);
        delay(200);
      }
      break;

    case GRAVADO:
      digitalWrite(pinoLed, HIGH);
      delay(3000);
      digitalWrite(pinoLed, LOW);
      break;

    case APAGA_CANAL:
      for (int i = 0; i < 5; i++) {
            digitalWrite(pinoLed, HIGH);
            delay(300);
            digitalWrite(pinoLed, LOW);
            delay(300);
      }
      break;
    
    case APAGA_TUDO:
      for (int i = 0; i < 5; i++) {
            digitalWrite(pinoLed, HIGH);
            delay(600);
            digitalWrite(pinoLed, LOW);
            delay(600);
      }
      break;    
  }
}
