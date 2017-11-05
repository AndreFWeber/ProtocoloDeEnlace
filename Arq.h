#ifndef ARQ_H
#define ARQ_H

#include "enquadramento.h"
//#include "tun.h"
#include "dispatcher.h"
#include <cstring>

using namespace std;

class Arq : public Handler{
	private:
		enum Estados {
			estado0, estado1
		};
		// estado atual da MEF
	        int estado;
	        bool sequencia;
		bool crc;
	        char last_frame_sent[1024];//Importante guardar o ultimo quadro para o reenvio.
	        int len_last_frame_sent;//Importante guardar o tamanho do ultimo quadro para o reenvio.
	        bool ack_recebido;
	        bool payload_recebido;

		void reenvia();
		void recebe_quadro(char * buffer, int len);	
		void envia_poll();	

		typedef enum {
			Quadro, Payload, Timeout
		}Evento;



	public:
		Arq (int desc); //Abstraindo...Nao sabemos o que fazer com o tutnutnutntutntu
		~Arq ();

		int encapsula(char * mensagem, int bytes, int tipo);
		int desencapsula(char * mensagem, int bytes);
		int handle_data();
		void send(char *buffer, int len);
		void receive (char *buffer, int len, bool crc); // recebe o vetor de char enviado  
		void fsm(Evento tipo, char * buffer, int len);
  		void handle_timeout(); // trata timeout
};
#endif
