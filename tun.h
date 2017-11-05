#ifndef TUN_H_
#define TUN_H_

using namespace std;

class tun {
public:
	tun(char *name, char * from, char * to);
	~tun();

	int tun_alloc(char *dev);
	int set_ip(char *dev, char * ip, char * dst);
	int get_fd_tun();

protected:	
	int _fd_tun;
};

#endif /* TUN_H_ */
