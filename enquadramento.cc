#include "enquadramento.h"

#include <iostream>
#include <memory.h>
#include <stdexcept>
using namespace std;


  Enquadramento::Enquadramento(Serial & dev, int bytes_min, int bytes_max, int desc) : Handler::Handler(desc), porta(dev){
		min_bytes = bytes_min;
		max_bytes = bytes_max;
	    estado = Q0;
  }

  Enquadramento::~Enquadramento() {}

// o tratador de eventos de uma MEF hipotética
// A MEF aqui representada nada faz de útil ... 
void dump(char * buffer, int len, bool nada, int nadica) {
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

void Enquadramento::receive (char *buffer, int len, bool crc){
}

void Enquadramento::send(char * buffer, int bytes){

/* Esse método tem como função armazenar todos os bytes que devem ser enviados em um buffer
*/

	if(bytes > max_bytes || bytes < min_bytes){
		cout << "Enquadramento: Número de bytes ("<< bytes <<") deveria estar entre " << min_bytes << " e " << max_bytes <<"\n"<< endl;
		throw exception();
	}
	char quadro[max_bytes];
	char temp[2+(bytes+2)*2];
	memcpy(temp, buffer, bytes);

	gen_crc(temp, bytes);

	bytes += 2;	
	quadro[0] = 0x7E;
	int pos = 1;
	for (int i = 0; i < bytes; i++) {
		if (temp[i] == 0x7E or temp[i] == 0x7D) {
			quadro[pos++] = 0x7D;
			quadro[pos++] = temp[i] xor 0x20;
		}else quadro[pos++] = temp[i];
	}
	quadro[pos] = 0x7E;
	quadro[++pos] = '\0';
	cout << "Enquadramento: Quadro original com CRC e flags a ser enviado:" << endl;
	dump(quadro, pos, false, 1);
	int ret_porta = porta.write(quadro, pos);

	if(ret_porta == 0)cout << "Enquadramento: Erro ao enviar o quadro! \n"<< endl;
	else cout << "Enquadramento: Quadro enviado com sucesso!\n"<< endl;
 }

int Enquadramento::handle_data(){
	   start_timeout(100);
	   char byte;

	   int n = porta.read(&byte, 1);

	   buffer[index_buffer_read]=byte;
	   
	   if (fsm(byte)) {
			index_buffer_read = 0;
			//Retirado, pois não há retorno, ou seja, não é copiado nada para o buffer_out			
			//memcpy(buffer_out, buffer, n_bytes);
			
			//cout << "\n Foram recebidos " << n_bytes << " bytes, sendo eles: " << buffer << endl;
			//buffer_out[2]='t'; //para corromper o quadro (para teste)
			bool crc = check_crc(buffer, n_bytes);
			//dump(buffer, n_bytes, true, 0);
			//if(crc == true) upper->receive(buffer, n_bytes); 
			char quadro [n_bytes-2];
			memcpy(quadro, buffer, n_bytes-2);
			dump(quadro, n_bytes-2, true, 0);
			upper->receive(quadro, n_bytes-2, crc); 
			
			return n_bytes;
	   }
	   index_buffer_read++;

}		

void Enquadramento::handle_timeout(){
	cout << "Enquadramento: Timeout do enquadramento \n"<<endl;
	stop_timeout();
	index_buffer_read = 0;	
}

// o tratador de eventos de uma MEF hipotética
// A MEF aqui representada nada faz de útil ... 
bool Enquadramento::fsm(char byte) {
	  switch (estado) {
		case Q0:
		  if (byte == 0x7E) {// ~ é 7E na tabela ASCII
				n_bytes = 0;
				estado = Q1; // muda para Q1
		  }else{
				estado = Q0;
		  }
		  break;

		case Q1: // estado 1
		  if (byte == 0x7D) {// } é 7D na tabela ASCII
				estado = Q2;
				break;
		  }
		  if (byte == 0x7E) {
				estado = Q0;

				if (n_bytes < min_bytes) {
					cout << "Enquadramento: Número de bytes do quadro é menor que o tamanho mínimo.\n" << endl;
					n_bytes = 0;
					estado = Q0;
					break;
				}
				return true;
		  }else{
				if (n_bytes > max_bytes) {
					cout << "Enquadramento: Número maior que a capacidade do buffer.\n" << endl;
					n_bytes = 0;
					estado = Q0;
					break;
				}

					buffer[n_bytes] = byte;

					//cout<<"add no buffer "<< byte << endl;

					estado = Q1;
					n_bytes = n_bytes + 1;

				break;
		  } 		

		case Q2: // estado 2
			// trata os casos de depois da flag 7D, ter informação contendo 7D ou 7E
			if ((byte != 0x7D) && (byte != 0x7E)) { // ^ e ] são, respectivamente, 5D e 5E na tabela ASCII
				buffer[n_bytes] = (byte xor 0x20);
				n_bytes= n_bytes + 1;
				estado = Q1;
			}else{ //erro
								
					cout << "Enquadramento: Erro no escape!\n" << endl;
					n_bytes = 0;
					estado = Q0;
			}			
			break;
	  }
		return false;
	}

	//gera novo crc 
	//compara com valores pré definidos
	//retorna true se estiver ok
bool Enquadramento::check_crc(char * buffer, int len) {
	uint16_t trialfcs = pppfcs16( PPPINITFCS16, buffer, len);
		   
	if ( trialfcs == PPPGOODFCS16 ){
	  cout <<"\nEnquadramento: Recebido sem falhas. (CRC OK)\n" << endl;
	  return true;
	}
	cout <<"Enquadramento: ERRO - Quadro corrompido. (CRC FAIL)\n" << endl;
	
	return false;
  }
  
void Enquadramento::gen_crc(char * buffer, int len){
       uint16_t value = pppfcs16(PPPINITFCS16, buffer, len);
       value ^= 0xffff;                
       buffer[len] = (value & 0x00ff);     
       buffer[len+1] = ((value >> 8) & 0x00ff);
}

uint16_t Enquadramento::pppfcs16(uint16_t fcs, char * cp, int len) { 
       assert(sizeof (uint16_t) == 2);
       assert(((uint16_t) -1) > 0);
       while (len--)
           fcs = (fcs >> 8) ^ fcstab[(fcs ^ *cp++) & 0xff];
       return (fcs);
}

	
