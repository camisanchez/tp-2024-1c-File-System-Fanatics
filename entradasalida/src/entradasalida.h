#include <shared.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <commons/collections/list.h>
#include <assert.h>
#include <pthread.h>
#include <interfazGenerica.h>
#include <interfazSTDIN.h>
#include <interfazSTDOUT.h>
#include <interfazDialFS.h>
#include <loggersIO.h>
#include <conexiones.h>
#include <signal.h>




extern t_config *configCargaInterfaz;




uint32_t recibir_direccion_fisica();