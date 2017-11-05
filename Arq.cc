#include "Arq.h"
#define SIZE_HEADER 3; 
#define TIMEOUT 1000; 

using namespace std;

Arq::Arq (int desc): Handler::Handler(desc){
	estado=0;
	sequencia=false;
	ativo=true;
	ack_recebido = false;
	payload_recebido = false;
	len_last_frame_sent = 0;
}
Arq::~Arq (){}

// o tratador de eventos de uma MEF hipotética
// A MEF aqui representada nada faz de útil ... 
void dump(char * buffer, int len, bool nada) {
   int m = 0, line = 0;
 
    while (m < len) {
        printf("%02X: ", line*16);
 
        for (int n=0; n < 16 and m < len; n++, m++) {
            int x = (unsigned char)buffer[m];
            printf("%02X ", x);
        }
        puts("");
        line++;
    }        
}


int Arq::encapsula(char * mensagem, int bytes, int tipo){
	//no cabeçalho será adicionado:
	//tipo: 8bits
	//sequencia:8bit
	//formato:8bits
	char quadro[1024];
	char temp[2+(bytes+2)*2];
	memcpy(temp, mensagem, bytes);

	/*
	 *bytes: Representa o tamanho total do quadro
	 *	Neste caso soma-se 3 pois o cabeçalho aumenta a mensagem em 3 bytes.
	 *  MODIFICAR SE O TAMANHO DO CABEÇALHO FOR MUDADO	
	 */	
	bytes += SIZE_HEADER;	

	/*
	 *Tipo: Na posição 0 do quadro
	 *	- Tipo = 0 É um quadro de informação
	 *	- Tipo = 1 É um quadro de confirmação
	 *	- Tipo = 2 É um quadro de poll	
	 */	
	switch(tipo){
		case 0:
			quadro[0] = 0x00;
			len_last_frame_sent = bytes; //Importante guardar o tamanho do ultimo quadro para o reenvio.
			break;
		case 1:
			//if(!this->crc) sequencia=!sequencia;
			quadro[0] = 0x01;
			break;
		case 2:
			quadro[0] = 0x02;
			quadro[1] = 0xFF;
			break;
	}
	
	/*
	 *Sequencia: A sequência será zero ou um 
	 */	
	if(tipo!=2) {
		quadro[1] = sequencia ? 0x00 : 0x01;
		sequencia=!sequencia;
	}

	/*
	 *Formato: Especificará qual o tipo da informação (IPV4...)
	 * 		Não sabemos bem ao certo como fazer, ou como especificar...quais o tipos...etc...	
	 */	
	//Deixar 1 só para teste...Depois especificar melhor
	quadro[2]=0x01;
	
	/*
	 * No loop, a mensagem a ser encapsulada será copiada para o quadro que ja tem 3 bytes de cabeçalho
	 */	
		
	int pos = SIZE_HEADER;
	for (int i = 0; i < bytes; i++) {
	     quadro[pos++] = temp[i];
	}

	if(tipo==0)memcpy(last_frame_sent, quadro, bytes);

	memcpy(mensagem, quadro, bytes);
	return bytes;
}

int Arq::desencapsula(char * mensagem, int bytes){
	char quadro[2*bytes];
	char temp[2+(bytes+2)*2];
	memcpy(temp, mensagem, bytes);

	/*
	 *bytes: Representa o tamanho total do quadro
	 *	Neste caso soma-se 3 pois o cabeçalho aumenta a mensagem em 3 bytes.
	 *  MODIFICAR SE O TAMANHO DO CABEÇALHO FOR MUDADO	
	 */	
	int pos = 0;
	int i = SIZE_HEADER-1;
	for (i; i < bytes; i++) {
	     quadro[pos++] = temp[i];
	}

	memcpy(mensagem, quadro, bytes);

	bytes -= SIZE_HEADER;	

	return bytes;
}

void Arq::reenvia(){
	lower->send(last_frame_sent, len_last_frame_sent);
	envia_poll();		
}

void Arq::envia_poll() {
	int size = SIZE_HEADER + 1;
	char buf[size];
	encapsula(buf,size,2);
	lower->send(buf, size);
}

void Arq::recebe_quadro(char * buffer, int len){
				if(buffer[0]==0x02 && ack_recebido){
						this->enable();
						lower->disable();//desativa o enquadramento
						ack_recebido=false;
				}
				if(buffer[0]==0x02 && payload_recebido){
						this->enable();
						lower->disable();//desativa o enquadramento

						int size = SIZE_HEADER+1;		

						char buf[size];
						buf[0] = !sequencia ? 0x00 : 0x01;
						
						encapsula(buf, size, 1);//Monta um quadro de ACK
						lower->send(buf, size);
						cout << "Arq: ACK Enviado\n" << endl;

				}

				if(buffer[0]==0x01){//eh um ack
					char seq = !sequencia ? 0x00 : 0x01;
					if(buffer[1] == seq) {
						cout << "Arq: ACK recebido \n" << endl;
					
						ack_recebido = true;
							
						write(get_descriptor(), buffer, len);
											
						estado = estado0;
					}else{
						cout << "Arq: ACK corrompido \n" << endl;
						reenvia();
					}
	
				}else if(buffer[0]==0x00){ //não é um ACK, é um quadro de payload

						cout << "Arq: Quadro com payload recebido!\n" << endl;

						int aux = desencapsula(buffer, len);
						//dump(buffer, aux, true);
						lower->enable();
						write(get_descriptor(), buffer, aux);

						this->payload_recebido = true;					

				}
}



void Arq::fsm(Evento tipo, char * buffer, int len) {

	switch (estado) {
		case estado0:
			if(tipo==Payload){
				//  tun?payload/(!dataN, timeout)
				len = encapsula(buffer, len, 0);

				lower->send(buffer, len);

				envia_poll();
				//Liga o time_out
	   			start_timeout(1000);

				//!  tun?payload/(!dataN, timeout)
				this->disable();
				lower->enable();

				estado = estado1;
			}

			if(tipo==Quadro){ 		 	//recebe do enquadramento
				//cout<<"chegou no quadro 1"<<endl;
				recebe_quadro(buffer, len);	
				estado = estado1;
			}
			break;

		case estado1: // estado 1
		       /* Estado 0 enviou o dataN e veio pro estado 1
			* Estado 1 deve esperar o ACK que vem do enquadramento via "upper.receive()"
			*/	
			if(tipo==Quadro){ 		 	//recebe do enquadramento
				recebe_quadro(buffer, len);	
				estado = estado0;			
			}	
			if(tipo==Timeout){ 		 	//Trata o timeout
				cout << "Arq: Estouro do timeout detectado! Quadro será reenviado.\n"<<endl;
				stop_timeout();
				reenvia();
	   			start_timeout(1000);
			}	


			break; 
	  }
}



int Arq::handle_data() {
	char buff_recebido[1024];
	int qtd_bytes;

	qtd_bytes = read(get_descriptor(), buff_recebido, 1024);
	
	fsm(Payload, buff_recebido, qtd_bytes);

}

void Arq::handle_timeout() {
  fsm(Timeout, NULL, 0);
}

void Arq::send(char *buffer, int len){}

 // recebe o vetor de char recebido pela serial (vindo do enquadramento)   
void Arq::receive (char *buffer, int len, bool crc){
	this->crc = crc;
	fsm(Quadro, buffer, len);
}



