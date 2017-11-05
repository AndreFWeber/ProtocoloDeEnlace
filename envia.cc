#include <iostream>
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>

#include <memory.h>

#include "enquadramento.h"
#include "tun.h"
#include "dispatcher.h"
#include "Arq.h"

using namespace std;
 
void dump(char * buffer, int len) {
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

int main(int argc, char * argv[]) {
	if (argc != 2) {
		cout << "Main: Especifique apenas o modo de operação: Mestre(M) ou Escravo(E)!\n" << endl;
		return 0;
	}
	char * modo = argv[1];

	//Instanciamento da Serial
	Serial s("/dev/ttyUSB0", B9600);
	//Instanciamento da TUN
	tun tun("interface_tun", "10.0.0.1", "10.0.0.2");

	int desc1 = s.get();
	int desc2 = tun.get_fd_tun();

	//Instanciamento do Framming
	Enquadramento enq(s,1,4096,desc1);

	Arq arq(desc2);

	if (strcmp(modo,"M") == 0) {
		cout << "Main: Protocolo iniciado como Mestre\n\n" << endl;
		arq.enable();
	} else if (strcmp(modo, "E") == 0)  {
		cout << "Main: Protocolo iniciado como Escravo\n\n" << endl;
		enq.enable();
		arq.disable();
	}else{
		return 0;
	}

	arq.set_lower(enq);
	enq.set_upper(arq);

	Dispatcher monitor;

	monitor.add_handler(arq);
	monitor.add_handler(enq);

	monitor.loop();
}
