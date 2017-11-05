#include <sys/select.h>
#include "dispatcher.h"
#include <iostream>

Handler::Handler(int descriptor) : fd(descriptor) {
   // o descritor associado ao handler deve estar em modo não-bloqueante ...
   int op = fcntl(fd, F_GETFL);
   fcntl(fd, F_SETFL, op | O_NONBLOCK);
   ativo = true;
}
 
int Handler::get_timeout() const {
  timeval agora;

  // sem timeout por enquanto ...
  if (to_end.tv_sec == 0 and to_end.tv_usec == 0){
	 return 0;
  }
  gettimeofday(&agora, NULL);
  int t_out = (to_end.tv_sec - agora.tv_sec) * 1000;
  t_out += (to_end.tv_usec - agora.tv_usec) / 1000;

  return t_out;

}
 
bool Handler::timeout_exceeded() const {
  return get_timeout() < 0;
}
 
void Handler::start_timeout(int t_out) {
  gettimeofday(&to_end, NULL);
  long us = to_end.tv_usec + t_out * 1000;

  to_end.tv_sec += us / 1000000;
  to_end.tv_usec = us % 1000000;  
}
 
void Handler::stop_timeout() {
  to_end.tv_sec = 0;
  to_end.tv_usec = 0;
}  

void Dispatcher::add_handler(Handler & h) {
   handlers.push_back(&h);
}
 
void Dispatcher::loop() {
   while (true) {
     // cria um conjunto de descritores
     fd_set r;
     int fd_max = 0; // descritor com maior valor
 
     // inicia o conjunto de descritores, e nele
     // acrescenta descritores dos handlers
     FD_ZERO(&r);
     int tmin = -1;
     for (auto h : handlers) {
       //Se o status deste Handler for false (desativado) ele não é adicionado
       //à lista de descritores.	
       int tout = h->get_timeout();

       if (tout > 0) {
         if (tmin < 0) tmin = tout;
         else if (tmin > tout) tmin = tout;
       }


       if(h->get_status() == false) continue;
      	
       int fd = h->get_descriptor(); 
       if (fd > fd_max) fd_max = fd;
       FD_SET(fd, &r);
     }
 
     // define o valor de timeout a ser usado pelo select
     // se nenhum handler definiu um timeout, então select também
     // não tem timeout
     timeval timeout;
     timeval * to_ptr = NULL;
     if (tmin > 0) {
       timeout.tv_sec = tmin / 1000;
       timeout.tv_usec = 1000 * (tmin % 1000);
       to_ptr = &timeout;
     }

     // chama select para monitorar o conjunto de descritores
     // como não foi especificado timeout (último
     // parâmetro), select pode ficar bloqueado para sempre
     // O valor de retorno de select é a quantidade de
     // descritores prontos para serem acessados
     int n = select(fd_max+1, &r, NULL, NULL, to_ptr);
 
     if (n == 0) {
       // timeout ...
       for (auto h : handlers) {
         if (h->timeout_exceeded()){
		 h->handle_timeout();  
	 } 
       }
    } else {
       for (auto h : handlers) {
         if (h->get_status()) {
           int fd = h->get_descriptor();
 
           // testa se descritor está pronto para ser acessado
           if (FD_ISSET(fd, &r)) h->handle_data();
         }
       }
     }
  }
}
