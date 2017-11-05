#ifndef DISPATCHER_H
#define DISPATCHER_H
 
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <sys/time.h>

using std::vector;
 
// Handler: classe abstrata que deve ser especializada para definir handlers
class Handler {
 protected:
  int fd; // descritor a ser monitorado
  Handler *upper;
  Handler *lower;
  bool ativo;
  timeval to_end; // tempo de referência para o timeout

  // para iniciar um timeout (para uso interno)
  void start_timeout(int t_out);
 
  // para cancelar timeout
  void stop_timeout();

 public:
  Handler(int descriptor);
  ~Handler() {}
  virtual int handle_data() = 0; // trata dado disponível no descritor
  virtual void handle_timeout() = 0; // trata timeout
  virtual void send(char *buffer, int len) = 0; // Envia o vetor de char recebido
  virtual void receive (char *buffer, int len, bool crc) = 0; // recebe o vetor de char enviado  
  
  void set_upper(Handler & u) { upper = &u;}
  void set_lower(Handler & l) { lower = &l;}

  void enable() {ativo = true;}
  void disable() {ativo = false;}
  bool get_status() {return ativo;}

   // retorna valor de timeout em milissegundos
  // se retornar 0 (zero), então não há timeout a ser verificado
  int get_timeout() const;
 
  bool timeout_exceeded() const;
 
  int get_descriptor() const { return fd;}
};
 
// Dispatcher: o monitor e encaminhador de eventos
class Dispatcher {
 private:
  vector<Handler*> handlers;
 public:
  Dispatcher() {}
  ~Dispatcher() {}
  void add_handler(Handler & h); // registra um handler
  void loop(); // aguarda e encaminha eventos indefinidamente
};
 
#endif

