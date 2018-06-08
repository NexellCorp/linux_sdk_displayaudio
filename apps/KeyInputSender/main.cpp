#include <stdint.h>
#include <stdlib.h>	//	atoi
#include <unistd.h>	//	optarg/getopt
#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void usage( char *cmd )
{
	printf(" Usage : %s [options]\n", cmd);
	printf("   Optons :\n");
	printf("      -a : adress( default : 127.0.0.1 ) \n");
	printf("      -k : key\n");
	printf("      -v : value\n");
	printf("      -h : print usage\n");
	printf("   ex1) shell mode\n");
	printf("       #> %s\n", cmd);
	printf("   ex2) key 100 value 10\n");
	printf("       #> %s -k 100 -v 10\n", cmd);
}

static void remodte_send( const char *addr, uint16_t port, int32_t key, int32_t value )
{
	int32_t data[2];
	int32_t clntSock = socket( AF_INET, SOCK_DGRAM, 0);

	data[0] = key;
	data[1] = value;

	struct sockaddr_in destAddr;
	memset( &destAddr, 0, sizeof(destAddr) );
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(port);
	destAddr.sin_addr.s_addr = inet_addr(addr);

	socklen_t addrLen = sizeof(destAddr);

	ssize_t sendSize = sendto(clntSock , data, sizeof(data), 0, (struct sockaddr*)&destAddr, addrLen);

	if( sendSize < 0 )
	{
		printf("send failed!!!\n");
	}

	close(clntSock);

}

int32_t main( int argc, char *argv[] )
{
	int32_t opt;
	int32_t key = -1;
	int32_t value = -1;
	uint16_t port = 5151;
	const char *pAddr = "127.0.0.1";	//	localhost

	while( -1 != (opt=getopt(argc, argv, "hk:v:a:p:")))
	{
		switch( opt )
		{
		case 'a':
			pAddr = (const char*)optarg;
			break;
		case 'p':
			port = atoi( optarg );
			break;
		case 'k':
			key = atoi( optarg );
			break;
		case 'v':
			value = atoi( optarg );
			break;
		case 'h':
			usage( argv[0] );
			return 0;
		}
	}

	if( -1==key || -1==value || NULL==pAddr )
	{
		usage( argv[0] );
	}
	else
	{
		remodte_send( pAddr, port, key, value );
	}
	printf("~~~~~Done\n");
	return 0;
}
